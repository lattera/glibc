#include "nldbl-compat.h"

int
attribute_hidden
__finitel (double x)
{
  return finite (x);
}
extern __typeof (__finitel) finitel attribute_hidden;
weak_alias (__finitel, finitel)
