/* The actual implementation for all floating point sizes is in strtod.c.
   These macros tell it to produce the `float' version, `strtof'.  */

#define	FLOAT		float
#define	FLT		FLT
#define	STRTOF		__strtof
#define	MPN2FLOAT	__mpn_construct_float
#define	FLOAT_HUGE_VAL	HUGE_VALf

#include "strtod.c"

weak_alias (__strtof, strtof)
