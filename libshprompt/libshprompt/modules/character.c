#include <libshprompt/config.h>
#include <libshprompt/modules/character.h>

module *character_module(context *ctx)
{
  module *m = module_new();
  segment *s = segment_new();
  segment_set_value(s, "\n" PROMPT_CHARACTER);
  module_add_segment(m, s);
  return m;
}