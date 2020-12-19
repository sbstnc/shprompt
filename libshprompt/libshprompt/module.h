#pragma once

#include <libshprompt/context.h>
#include <libshprompt/segment.h>
#include <stdlib.h>

typedef struct module module;

module *module_new(void);

void module_add_segment(module *m, segment *seg);

size_t module_get_segment_count(module *m);

segment *module_get_segment(module *m, size_t i);

void module_dispose(module *m);

typedef struct module_registry module_registry;

module_registry *module_registry_new(void);

void module_registry_add_module(module_registry *r, module *m);

size_t module_registry_get_module_count(module_registry *r);

module *module_registry_get_module(module_registry *r, size_t i);

void module_registry_dispose(module_registry *r);
