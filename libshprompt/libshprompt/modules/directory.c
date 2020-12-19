#include <libchalk/chalk.h>
#include <libsds/sds.h>
#include <libshprompt/config.h>
#include <libshprompt/module.h>
#include <libshprompt/modules/directory.h>
#include <stb/stb_ds.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static sds read_env(const char *name);
static sds home_path(void);
static void substitute_prefix(sds str, const char *old_pre,
                              const char *new_pre);
static void shorten_path_segments(sds path, const char *path_sep);

module *directory_module(context *ctx)
{
  module *m = module_new();
  sds curr = sdsnew(ctx->path);

  char *home = home_path();
  substitute_prefix(curr, home, HOME_SYMBOL);
  shorten_path_segments(curr, PROMPT_PATH_SEP);

  sdsfree(home);
  segment *s = segment_new();
  segment_set_value_ext(s, curr, sdsfree);
  segment_set_fmt(s, CHALK_CYAN("%s"));
  module_add_segment(m, s);

  return m;
}

sds read_env(const char *name)
{
  const char *env_var = getenv(name);
  return sdsnew(env_var);
}

sds home_path(void)
{
  sds home = read_env("HOME");

  if (!sdslen(home))
    home = read_env("USERPROFILE");

  if (!sdslen(home)) {
    sds home_drive = read_env("HOMEDRIVE");
    sds home_path = read_env("HOMEPATH");

    if (sdslen(home_drive) && sdslen(home_path)) {
      home_drive = sdscat(home_drive, home_path);

      free(home_path);
      return home_drive;
    }
  }

  return home;
}

void substitute_prefix(sds str, const char *old_pre, const char *new_pre)
{
  size_t cur_len = sdslen(str);
  if (!cur_len || !old_pre || !new_pre)
    return;

  size_t old_len = strlen(old_pre);
  if (strncmp(old_pre, str, old_len) != 0)
    return;

  size_t new_len = strlen(new_pre);
  bool new_is_shorter = new_len < old_len;

  sds tmp = sdsgrowzero(str, cur_len + new_len - old_len);
  if (tmp == NULL)
    return;
  str = tmp;

  memmove(str + new_len, str + old_len, cur_len - old_len + 1);
  memcpy(str, new_pre, new_len);

  if (new_is_shorter) {
    cur_len = cur_len + new_len - old_len;
    memset(str + cur_len, 0, old_len - new_len + 1);
    sdssetlen(str, cur_len);
  }
}

void shorten_path_segments(sds path, const char *path_sep)
{
  if (path == NULL || path_sep == NULL)
    return;

  size_t path_len = sdslen(path);
  size_t sep_len = strlen(path_sep);

  if (!path_len || !sep_len)
    return;

  int count = 0;
  sds *tokens = sdssplitlen(path, sdslen(path), path_sep, (int)strlen(path_sep),
                            &count);
  for (int i = 0; i < count - 1; ++i)
    tokens[i][1] = '\0';
  sds shortened = sdsjoin(tokens, count, (char *)path_sep);
  sdsfreesplitres(tokens, count);

  sdscpy(path, shortened);
}