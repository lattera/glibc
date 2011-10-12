/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

ENTRY(__ieee754_sqrt)
	fldl	4(%esp)
	fsqrt
	ret
END (__ieee754_sqrt)
strong_alias (__ieee754_sqrt, __sqrt_finite)
