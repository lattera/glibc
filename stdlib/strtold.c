/* The actual implementation for all floating point sizes is in strtod.c.
   These macros tell it to produce the `long double' version, `strtold'.  */

#define	FLOAT		long double
#define	FLT		LDBL
#define	STRTOF		strtold
#define	MPN2FLOAT	__mpn_construct_long_double
#define	FLOAT_HUGE_VAL	HUGE_VALL
#define SET_MANTISSA(flt, mant) \
  do { union ieee854_long_double u;					      \
       u.d = (flt);							      \
       u.ieee.mantissa0 = ((mant) >> 32) & 0x7fffffff;			      \
       u.ieee.mantissa1 = (mant) & 0xffffffff;				      \
       (flt) = u.d;							      \
  } while (0)

#include "strtod.c"
