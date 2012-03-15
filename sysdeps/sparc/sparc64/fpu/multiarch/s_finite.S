#include <sysdep.h>

	.text
ENTRY(__finite)
	.type	__finite, @gnu_indirect_function
# ifdef SHARED
	SETUP_PIC_REG_LEAF(o3, o5)
# endif
# ifdef HAVE_AS_VIS3_SUPPORT
	set	HWCAP_SPARC_VIS3, %o1
	andcc	%o0, %o1, %g0
	be	9f
	 nop
#  ifdef SHARED
	sethi	%gdop_hix22(__finite_vis3), %o1
	xor	%o1, %gdop_lox10(__finite_vis3), %o1
#  else
	set	__finite_vis3, %o1
#  endif
	ba	10f
	 nop
9:
# endif
# ifdef SHARED
	sethi	%gdop_hix22(__finite_generic), %o1
	xor	%o1, %gdop_lox10(__finite_generic), %o1
# else
	set	__finite_generic, %o1
# endif
# ifdef HAVE_AS_VIS3_SUPPORT
10:
# endif
# ifdef SHARED
	add	%o3, %o1, %o1
# endif
	retl
	 mov	%o1, %o0
END(__finite)
hidden_def (__finite)
weak_alias (__finite, finite)

# undef weak_alias
# define weak_alias(a, b)
# undef hidden_def
# define hidden_def(a)

#define __finite __finite_generic

#include "../s_finite.S"
