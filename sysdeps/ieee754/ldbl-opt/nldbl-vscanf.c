#include "nldbl-compat.h"

int
attribute_hidden
weak_function
vscanf (const char *fmt, va_list ap)
{
  return __nldbl__IO_vfscanf (stdin, fmt, ap, NULL);
}
