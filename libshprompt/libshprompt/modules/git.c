#include <libchalk/chalk.h>
#include <libshprompt/config.h>
#include <libshprompt/module.h>
#include <libshprompt/modules/git.h>
#include <string.h>

static void ahead_behind(git_repository *, size_t *, size_t *);

module *git_module(context *ctx)
{
  module *m = module_new();
  git_libgit2_opts(GIT_OPT_ENABLE_STRICT_HASH_VERIFICATION, 0);
  git_libgit2_opts(GIT_OPT_DISABLE_INDEX_CHECKSUM_VERIFICATION, 1);
  git_libgit2_opts(GIT_OPT_DISABLE_INDEX_FILEPATH_VALIDATION, 0);
  git_libgit2_opts(GIT_OPT_DISABLE_READNG_PACKED_TAGS, 1);
  git_libgit2_init();
  git_buf *buf = calloc(1, sizeof(git_buf));
  git_repository *repo = NULL;
  size_t ahead = 0;
  size_t behind = 0;

  int err = git_repository_discover(buf, ctx->path, 1, PROMPT_PATH_SEP);
  if (err && err != GIT_ENOTFOUND)
    goto cleanup;

  if (err != GIT_ENOTFOUND) {
    err = git_repository_open_ext(&repo, buf->ptr, 0, NULL);
    if (err)
      goto cleanup;
  }

  git_reference *head_ref = head(repo);
  const char *head_shorthand = shorthand(head_ref);
  const char *head_oid = oid(head_ref);
  git_repository_status repo_status = status(repo);
  ahead_behind(repo, &ahead, &behind);

  sds v;
  if (strlen(head_shorthand)) {
    v = sdscatfmt(sdsempty(), BRANCH_SYMBOL "%s", head_shorthand);
    segment *s_head = segment_new();
    segment_set_value_ext(s_head, v, sdsfree);
    segment_set_fmt(s_head, CHALK_GREEN("%s"));
    module_add_segment(m, s_head);
  }

  if (repo_status.untracked > 0) {
    segment *s_untracked = segment_new();
    segment_set_value(s_untracked, UNTRACKED_SYMBOL);
    segment_set_fmt(s_untracked, CHALK_RED("%s"));
    module_add_segment(m, s_untracked);
  }

  if (repo_status.staged > 0) {
    segment *s_staged = segment_new();
    segment_set_value(s_staged, STAGED_SYMBOL);
    segment_set_fmt(s_staged, CHALK_CYAN("%s"));
    module_add_segment(m, s_staged);
  }

  if (repo_status.changed > 0) {
    segment *s_changed = segment_new();
    segment_set_value(s_changed, CHANGED_SYMBOL);
    segment_set_fmt(s_changed, CHALK_YELLOW("%s"));
    module_add_segment(m, s_changed);
  }

  if (repo_status.renamed > 0) {
    segment *s_renamed = segment_new();
    segment_set_value(s_renamed, RENAMED_SYMBOL);
    segment_set_fmt(s_renamed, CHALK_GREEN("%s"));
    module_add_segment(m, s_renamed);
  }

  if (repo_status.deleted > 0) {
    segment *s_deleted = segment_new();
    segment_set_value(s_deleted, DELETED_SYMBOL);
    segment_set_fmt(s_deleted, CHALK_RED("%s"));
    module_add_segment(m, s_deleted);
  }

  if (ahead > 0) {
    segment *s_ahead = segment_new();
    segment_set_value(s_ahead, AHEAD_SYMBOL);
    segment_set_fmt(s_ahead, CHALK_CYAN("%s"));
    module_add_segment(m, s_ahead);
  }

  if (behind > 0) {
    segment *s_behind = segment_new();
    segment_set_value(s_behind, BEHIND_SYMBOL);
    segment_set_fmt(s_behind, CHALK_RED("%s"));
    module_add_segment(m, s_behind);
  }

  if (repo_status.conflicts > 0) {
    segment *s_conflicts = segment_new();
    segment_set_value(s_conflicts, CONFLICTS_SYMBOL);
    segment_set_fmt(s_conflicts, CHALK_YELLOW("%s"));
    module_add_segment(m, s_conflicts);
  }

  if (strlen(head_oid)) {
    segment *s_oid = segment_new();
    segment_set_value(s_oid, head_oid);
    segment_set_fmt(s_oid, CHALK_WHITE("%s"));
    module_add_segment(m, s_oid);
  }

cleanup:
  git_buf_dispose(buf);
  git_repository_free(repo);
  git_libgit2_shutdown();
  return m;
}

const char *shorthand(const git_reference *ref)
{
  return (ref == NULL) ? "" : git_reference_shorthand(ref);
}

const char *oid(const git_reference *ref)
{
  if (!ref)
    return "";

  char *oid_hex = git_oid_tostr_s(git_reference_target(ref));
  if (strlen(oid_hex) > 7) {
    oid_hex[7] = '\0';
  }
  return oid_hex;
}

git_reference *head(git_repository *repo)
{
  if (!repo)
    return NULL;

  git_reference *head_ref = NULL;
  int err = git_repository_head(&head_ref, repo);
  if (err) {
    git_reference_free(head_ref);
    return NULL;
  }
  return head_ref;
}

git_reference *upstream(const git_reference *ref)
{
  if (!git_reference_is_branch(ref))
    return NULL;

  git_reference *upstream_ref = NULL;
  int err = git_branch_upstream(&upstream_ref, ref);
  if (err) {
    git_reference_free(upstream_ref);
    return NULL;
  }
  return upstream_ref;
}

static void ahead_behind(git_repository *repo, size_t *ahead, size_t *behind)
{
  git_reference *local = head(repo);
  if (local) {
    const git_reference *upstream_ref = upstream(local);
    if (upstream_ref) {
      const git_oid *local_oid = git_reference_target(local);
      const git_oid *upstream_oid = git_reference_target(upstream_ref);
      git_graph_ahead_behind(ahead, behind, repo, local_oid, upstream_oid);
    }
  }
  git_reference_free(local);
}

git_repository_status status(git_repository *repo)
{
  git_repository_status stats = {.untracked = 0,
                                 .conflicts = 0,
                                 .changed = 0,
                                 .staged = 0,
                                 .deleted = 0,
                                 .renamed = 0};

  if (repo) {
    git_status_options opts = GIT_STATUS_OPTIONS_INIT;
    opts.flags = GIT_STATUS_OPT_DEFAULTS | GIT_STATUS_OPT_RENAMES_HEAD_TO_INDEX
                 | GIT_STATUS_OPT_RENAMES_INDEX_TO_WORKDIR
                 | GIT_STATUS_OPT_RENAMES_FROM_REWRITES;

    git_status_foreach_ext(repo, &opts, git_repository_status_cb, &stats);
  }

  return stats;
}

int git_repository_status_cb(const char *path, unsigned int flags,
                             void *payload)
{
  git_repository_status *status = (git_repository_status *)payload;

  if (flags & (GIT_STATUS_IGNORED | GIT_STATUS_CURRENT))
    return 0;

  if (flags & (GIT_STATUS_INDEX_NEW | GIT_STATUS_INDEX_MODIFIED))
    status->staged++;

  if (flags & GIT_STATUS_WT_MODIFIED)
    status->changed++;

  if (flags
      & (GIT_STATUS_INDEX_RENAMED | GIT_STATUS_WT_RENAMED
         | GIT_STATUS_INDEX_TYPECHANGE | GIT_STATUS_WT_TYPECHANGE))
    status->renamed++;

  if (flags & (GIT_STATUS_INDEX_DELETED | GIT_STATUS_WT_DELETED))
    status->deleted++;

  if (flags & GIT_STATUS_CONFLICTED)
    status->conflicts++;

  if (flags & GIT_STATUS_WT_NEW)
    status->untracked++;

  return 0;
}
