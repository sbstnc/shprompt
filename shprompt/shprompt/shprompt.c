#include <argtable/argtable3.h>
#include <errno.h>
#include <libshprompt/config.h>
#include <libshprompt/context.h>
#include <libshprompt/module.h>
#include <libshprompt/modules/character.h>
#include <libshprompt/modules/directory.h>
#include <libshprompt/modules/git.h>
#include <libshprompt/modules/status.h>
#include <stdio.h>

struct arg_lit *color, *help, *version;
struct arg_int *status;
struct arg_file *o, *file;
struct arg_end *end;

static void get_abs_path(const char *str, const char **dest);

int main(int argc, char *argv[])
{
  // clang-format off
  void *argtable[] = {
      help    = arg_lit0(NULL, "help", "display this help and exit"),
      version = arg_lit0(NULL, "version", "display version info and exit"),
      color   = arg_lit0("c", "color", "enable colored output"),
      status  = arg_int0("s", "status", "<n>", "exit code of last shell command"),
      file    = arg_file0(NULL, NULL, "FILE", NULL),
      end     = arg_end(20),
  };
  // clang-format on

  int exitcode = EXIT_SUCCESS;

  if (arg_nullcheck(argtable) != 0) {
    printf("error: insufficient memory\n");
    exitcode = EXIT_FAILURE;
    goto exit;
  }

  status->ival[0] = 0;
  file->filename[0] = ".";
  int nerrors = arg_parse(argc, argv, argtable);

  if (help->count > 0) {
    printf("Usage: %s", argv[0]);
    arg_print_syntax(stdout, argtable, "\n");
    arg_print_glossary(stdout, argtable, "  %-25s %s\n");
    exitcode = EXIT_SUCCESS;
    goto exit;
  }

  if (nerrors > 0) {
    arg_print_errors(stdout, end, argv[0]);
    printf("Try '%s --help' for more information.\n", argv[0]);
    exitcode = EXIT_FAILURE;
    goto exit;
  }

  if (version->count > 0) {
    printf("%s %s\n", argv[0], "0.0.1");
    exitcode = EXIT_SUCCESS;
    goto exit;
  }

  context ctx = {.color = color->count,
                 .status_code = status->ival[0],
                 .path = file->filename[0]};
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

exit:
  arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
  return exitcode;
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
