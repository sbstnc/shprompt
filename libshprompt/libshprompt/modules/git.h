#pragma once

#include <git2.h>
#include <libsds/sds.h>
#include <libshprompt/context.h>

module *git_module(context *ctx);

const char *shorthand(const git_reference *ref);

const char *oid(const git_reference *ref);

git_reference *head(git_repository *repo);

git_reference *upstream(const git_reference *ref);

typedef struct git_repository_status {
  size_t untracked;
  size_t conflicts;
  size_t changed;
  size_t staged;
  size_t deleted;
  size_t renamed;
} git_repository_status;

git_repository_status status(git_repository *repo);

int git_repository_status_cb(const char *path, unsigned int flags,
                             void *payload);
