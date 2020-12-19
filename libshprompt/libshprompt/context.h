#pragma once

#include <stdbool.h>

typedef struct context {
  bool color;
  int status_code;
  const char *path;
} context;
