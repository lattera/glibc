#include "nldbl-compat.h"

int
attribute_hidden
wscanf (const wchar_t *fmt, ...)
{
  va_list arg;
  int done;

  va_start (arg, fmt);
  done = __nldbl_vfwscanf (stdin, fmt, arg);
  va_end (arg);

  return done;
}
