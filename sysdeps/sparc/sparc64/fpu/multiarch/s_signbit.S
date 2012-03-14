#include <sysdep.h>

	.text
ENTRY(__signbit)
	.type	__signbit, @gnu_indirect_function
# ifdef SHARED
	SETUP_PIC_REG_LEAF(o3, o5)
# endif
# ifdef HAVE_AS_VIS3_SUPPORT
	set	HWCAP_SPARC_VIS3, %o1
	andcc	%o0, %o1, %g0
	be	9f
	 nop
#  ifdef SHARED
	sethi	%gdop_hix22(__signbit_vis3), %o1
	xor	%o1, %gdop_lox10(__signbit_vis3), %o1
#  else
	set	__signbit_vis3, %o1
#  endif
	ba	10f
	 nop
9:
# endif
# ifdef SHARED
	sethi	%gdop_hix22(__signbit_generic), %o1
	xor	%o1, %gdop_lox10(__signbit_generic), %o1
# else
	set	__signbit_generic, %o1
# endif
# ifdef HAVE_AS_VIS3_SUPPORT
10:
# endif
# ifdef SHARED
	add	%o3, %o1, %o1
# endif
	retl
	 mov	%o1, %o0
END(__signbit)
weak_alias (__signbit, signbit)

/* On 64-bit the double version will also always work for
   long-double-precision since in both cases the word with the
   sign bit in it is passed always in register %f0.  */
strong_alias (__signbit, __signbitl)
hidden_def (__signbitl)
weak_alias (__signbitl, signbitl)

# undef weak_alias
# define weak_alias(a, b)
# undef strong_alias
# define strong_alias(a, b)
# undef hidden_def
# define hidden_def(a)

#define __signbit __signbit_generic

#include "../s_signbit.S"
