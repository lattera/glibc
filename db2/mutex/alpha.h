/* For alpha, 0 is clear, 1 is set.  */

#ifdef __GNUC__
#define	TSL_SET(tsl) ({							\
	register tsl_t *__l = (tsl);					\
	int __r;							\
	asm volatile(							\
		"1:	ldl_l	%0,%1\n"				\
		"	blbs	%0,2f\n"				\
		"	mov	1,%0\n"					\
		"	stl_c	%0,%1\n"				\
		"	bne	%0,1b\n"				\
		"	mb\n"						\
		"2:"							\
		: "=&r"(__r), "=m"(*__l) : "m"(*__l) : "memory");	\
	__r;								\
})
#endif

#ifdef __DECC
#include <alpha/builtins.h>
#define TSL_SET(tsl) (__LOCK_LONG_RETRY((tsl), 1) != 0)
#endif

#define	TSL_UNSET(tsl)	(*(tsl) = 0)
#define	TSL_INIT(tsl)	TSL_UNSET(tsl)
