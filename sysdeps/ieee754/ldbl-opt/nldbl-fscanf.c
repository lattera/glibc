#include "nldbl-compat.h"

int
attribute_hidden
fscanf (FILE *stream, const char *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl__IO_vfscanf (stream, fmt, arg, NULL);
  va_end (arg);

  return done;
}
