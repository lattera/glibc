#include "nldbl-compat.h"

int
attribute_hidden
weak_function
vwscanf (const wchar_t *fmt, va_list ap)
{
  return __nldbl_vfwscanf (stdin, fmt, ap);
}
