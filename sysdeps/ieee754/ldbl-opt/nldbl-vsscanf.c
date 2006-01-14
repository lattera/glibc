#include "nldbl-compat.h"

int
attribute_hidden
__vsscanf (const char *string, const char *fmt, va_list ap)
{
  return __nldbl_vsscanf (string, fmt, ap);
}
extern __typeof (__vsscanf) vsscanf attribute_hidden;
weak_alias (__vsscanf, vsscanf)
