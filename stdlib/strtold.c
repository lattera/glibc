/* The actual implementation for all floating point sizes is in strtod.c.
   These macros tell it to produce the `long double' version, `strtold'.  */

#define	FLOAT		long double
#define	FLT		LDBL
#define	STRTOF		strtold
#define	MPN2FLOAT	__mpn_construct_long_double
#define	FLOAT_HUGE_VAL	HUGE_VALl
#define	IMPLICIT_ONE	0	/* XXX for i387 extended format */

#include "strtod.c"
