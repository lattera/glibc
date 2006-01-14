#define qgcvt qgcvt_XXX
#include "nldbl-compat.h"
#undef qgcvt

char *
qgcvt (double val, int ndigit, char *buf)
{
  return gcvt (val, ndigit, buf);
}
