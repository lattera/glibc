#include "nldbl-compat.h"

int
attribute_hidden
__isinfl (double x)
{
  return __isinf (x);
}
extern __typeof (__isinfl) isinfl attribute_hidden;
weak_alias (__isinfl, isinfl)
