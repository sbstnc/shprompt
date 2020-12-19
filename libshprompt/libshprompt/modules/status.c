#include <libchalk/chalk.h>
#include <libshprompt/config.h>
#include <libshprompt/modules/status.h>

module *status_module(struct context *ctx)
{
  module *m = module_new();
  const char *value = (ctx->status_code) ? BAD_EXIT_SYMBOL : "";
  segment *s = segment_new();
  segment_set_value(s, value);
  segment_set_fmt(s, CHALK_RED("%s"));
  module_add_segment(m, s);
  return m;
}