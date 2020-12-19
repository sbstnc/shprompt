#include <errno.h>
#include <flag.h>
#include <libshprompt/config.h>
#include <libshprompt/context.h>
#include <libshprompt/module.h>
#include <libshprompt/modules/character.h>
#include <libshprompt/modules/directory.h>
#include <libshprompt/modules/git.h>
#include <libshprompt/modules/status.h>
#include <stdio.h>

static void get_abs_path(const char *str, const char **dest);

int main(int argc, const char **argv)
{
  context ctx = {.color = true, .status_code = 0, .path = "."};
  flag_bool(&ctx.color, "color", "Use ANSI codes for colored output");
  flag_int(&ctx.status_code, "status", "Status of last shell command");
  flag_string((const char **)&ctx.path, "path",
              "Path to render the prompt for");
  flag_parse(argc, argv, "0.0.1");
  get_abs_path(ctx.path, &ctx.path);

  module_registry *reg = module_registry_new();
  module_registry_add_module(reg, directory_module(&ctx));
  module_registry_add_module(reg, git_module(&ctx));
  module_registry_add_module(reg, status_module(&ctx));
  module_registry_add_module(reg, character_module(&ctx));

  for (int i = 0; i < module_registry_get_module_count(reg); ++i) {
    module *mod = module_registry_get_module(reg, i);
    for (size_t j = 0; j < module_get_segment_count(mod); ++j) {
      segment *s = module_get_segment(mod, j);
      const char *fmt = segment_get_fmt(s);
      printf((ctx.color && fmt) ? fmt : "%s", segment_get_value(s));
      printf(STATUS_SEPARATOR);
    }
  }

  module_registry_dispose(reg);

  return EXIT_SUCCESS;
}

static void get_abs_path(const char *str, const char **dest)
{
  char abs_path[PROMPT_PATH_MAX];
  char *abs_path_p = realpath(str, abs_path);
  if (abs_path_p)
    *dest = abs_path_p;
  else
    exit(errno);
}
