#include <math.h>

#ifndef __NO_LONG_DOUBLE_MATH
/* The actual implementation for all floating point sizes is in strtod.c.
   These macros tell it to produce the `long double' version, `strtold'.  */

# define FLOAT		long double
# define FLT		LDBL
# ifdef USE_IN_EXTENDED_LOCALE_MODEL
#  define STRTOF	__strtold_l
# else
#  define STRTOF	strtold
# endif
# define MPN2FLOAT	__mpn_construct_long_double
# define FLOAT_HUGE_VAL	HUGE_VALL
# define SET_MANTISSA(flt, mant) \
  do { union ieee854_long_double u;					      \
       u.d = (flt);							      \
       if ((mant & 0x7fffffffffffffffULL) == 0)				      \
	 mant = 0x4000000000000000ULL;					      \
       u.ieee.mantissa0 = (((mant) >> 32) & 0x7fffffff) | 0x80000000;	      \
       u.ieee.mantissa1 = (mant) & 0xffffffff;				      \
       (flt) = u.d;							      \
  } while (0)

# include "strtod.c"
#else
/* There is no `long double' type, use the `double' implementations.  */
long double
__strtold_internal (const char *nptr, char **endptr, int group)
{
  return __strtod_internal (nptr, endptr, group);
}

long double
strtold (const char *nptr, char **endptr)
{
  return __strtod_internal (nptr, endptr, 0);
}
#endif
