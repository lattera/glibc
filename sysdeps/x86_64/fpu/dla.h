#include <features.h>

#ifdef __FMA4__
# define DLA_FMS(x,y,z) \
  __builtin_fma (x, y, -(z))
#endif

#include "sysdeps/ieee754/dbl-64/dla.h"
