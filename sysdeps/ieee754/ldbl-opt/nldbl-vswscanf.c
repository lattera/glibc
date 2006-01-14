#include "nldbl-compat.h"

int
attribute_hidden
vswscanf (const wchar_t *string, const wchar_t *fmt, va_list ap)
{
  return __nldbl_vswscanf (string, fmt, ap);
}
