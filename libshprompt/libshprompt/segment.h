#pragma once

#include <stdlib.h>

typedef struct segment segment;

segment *segment_new(void);

void segment_set_value(segment *s, const char *value);

void segment_set_value_ext(segment *s, const char *value,
                           void (*dispose_value)(char *));

void segment_set_fmt(segment *s, const char *fmt);

void segment_set_fmt_ext(segment *s, const char *fmt,
                         void (*dispose_fmt)(char *));

const char *segment_get_value(segment *s);

const char *segment_get_fmt(segment *s);

void segment_dispose(segment *s);
