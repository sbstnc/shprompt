#include <libshprompt/segment.h>

struct segment {
  const char *value;
  const char *fmt;
  void (*dispose_value)(char *value);
  void (*dispose_fmt)(char *fmt);
};

segment *segment_new(void)
{
  segment *s = calloc(1, sizeof(struct segment));
  return s;
};

void segment_set_value(segment *s, const char *value)
{
  segment_set_value_ext(s, value, NULL);
}

void segment_set_value_ext(segment *s, const char *value,
                           void (*dispose_value)(char *))
{
  if (s == NULL)
    return;
  s->value = value;
  s->dispose_value = dispose_value;
}

void segment_set_fmt(segment *s, const char *fmt)
{
  segment_set_fmt_ext(s, fmt, NULL);
}

void segment_set_fmt_ext(segment *s, const char *fmt,
                         void (*dispose_fmt)(char *))
{
  if (s == NULL)
    return;
  s->fmt = fmt;
  s->dispose_fmt = dispose_fmt;
}

const char *segment_get_value(segment *s) { return s->value; }

const char *segment_get_fmt(segment *s) { return s->fmt; }

void segment_dispose(segment *s)
{
  if (s != NULL) {
    if (s->dispose_value != NULL)
      s->dispose_value((char *)s->value);

    if (s->dispose_fmt != NULL)
      s->dispose_fmt((char *)s->fmt);

    free(s);
  }
}
