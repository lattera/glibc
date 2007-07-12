#include "nldbl-compat.h"

int
attribute_hidden
__vfscanf (FILE *s, const char *fmt, va_list ap)
{
  return __nldbl__IO_vfscanf (s, fmt, ap, NULL);
}
extern __typeof (__vfscanf) vfscanf attribute_hidden;
weak_alias (__vfscanf, vfscanf)
