#include "nldbl-compat.h"

int
attribute_hidden
fwscanf (FILE *stream, const wchar_t *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vfwscanf (stream, fmt, arg);
  va_end (arg);

  return done;
}
