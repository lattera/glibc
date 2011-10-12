/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

ENTRY(__ieee754_sqrtf)
	flds	4(%esp)
	fsqrt
	ret
END (__ieee754_sqrtf)
strong_alias (__ieee754_sqrtf, __sqrtf_finite)
