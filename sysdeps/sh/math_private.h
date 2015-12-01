#ifndef SH_MATH_PRIVATE_H
#define SH_MATH_PRIVATE_H 1

/* Enable __finitel, __isinfl, and __isnanl for binary compatibility
   when built without long double support. */
#define LDBL_CLASSIFY_COMPAT 1

#include_next <math_private.h>

#endif
