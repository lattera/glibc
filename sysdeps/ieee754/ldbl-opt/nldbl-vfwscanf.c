#include "nldbl-compat.h"

int
attribute_hidden
vfwscanf (FILE *s, const wchar_t *fmt, va_list ap)
{
  return __nldbl_vfwscanf (s, fmt, ap);
}
