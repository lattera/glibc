#include "nldbl-compat.h"

int
attribute_hidden
scanf (const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl__IO_vfscanf (stdin, fmt, arg, NULL);
  va_end (arg);

  return done;
}
