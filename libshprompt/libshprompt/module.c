#include <libshprompt/module.h>
#include <stb/stb_ds.h>

struct module {
  segment **segments;
};

module *module_new(void)
{
  module *m = (module *)calloc(1, sizeof(struct module));
  return m;
}

void module_add_segment(module *m, segment *seg) { arrput(m->segments, seg); }

size_t module_get_segment_count(module *m) { return arrlenu(m->segments); }

segment *module_get_segment(module *m, size_t i)
{
  if (m->segments == NULL)
    return NULL;
  return m->segments[i];
}

void module_dispose(module *m)
{
  if (m == NULL)
    return;

  for (int i = 0; i < arrlenu(m->segments); ++i) {
    segment *s = arrpop(m->segments);
    segment_dispose(s);
  }

  arrfree(m->segments);
  free(m);
}

struct module_registry {
  module **modules;
};

module_registry *module_registry_new(void)
{
  module_registry *reg = calloc(1, sizeof(struct module_registry));
  return reg;
}

void module_registry_add_module(module_registry *r, module *m)
{
  arrput(r->modules, m);
}

size_t module_registry_get_module_count(module_registry *r)
{
  return arrlenu(r->modules);
}

module *module_registry_get_module(module_registry *r, size_t i)
{
  if (r->modules == NULL)
    return NULL;
  return r->modules[i];
}

void module_registry_dispose(module_registry *r)
{
  for (size_t i = arrlenu(r->modules); i; --i) {
    module *m = arrpop(r->modules);
    module_dispose(m);
  }
  arrfree(r->modules);
  free(r);
}