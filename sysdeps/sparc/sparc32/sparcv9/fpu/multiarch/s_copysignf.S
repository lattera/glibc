#include <sysdep.h>

	.text
ENTRY(__copysignf)
	.type	__copysignf, @gnu_indirect_function
# ifdef SHARED
	SETUP_PIC_REG_LEAF(o3, o5)
# endif
# ifdef HAVE_AS_VIS3_SUPPORT
	set	HWCAP_SPARC_VIS3, %o1
	andcc	%o0, %o1, %g0
	be	9f
	 nop
#  ifdef SHARED
	sethi	%gdop_hix22(__copysignf_vis3), %o1
	xor	%o1, %gdop_lox10(__copysignf_vis3), %o1
#  else
	set	__copysignf_vis3, %o1
#  endif
	ba	10f
	 nop
9:
# endif
# ifdef SHARED
	sethi	%gdop_hix22(__copysignf_generic), %o1
	xor	%o1, %gdop_lox10(__copysignf_generic), %o1
# else
	set	__copysignf_generic, %o1
# endif
# ifdef HAVE_AS_VIS3_SUPPORT
10:
# endif
# ifdef SHARED
	add	%o3, %o1, %o1
# endif
	retl
	 mov	%o1, %o0
END(__copysignf)
weak_alias (__copysignf, copysignf)

# undef weak_alias
# define weak_alias(a, b)

#define __copysignf __copysignf_generic

#include "../../../fpu/s_copysignf.S"
