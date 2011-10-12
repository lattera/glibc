/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 * Adapted for `long double' by Ulrich Drepper <drepper@cygnus.com>.
 */

#include <machine/asm.h>

ENTRY(__ieee754_remainderl)
	fldt	16(%esp)
	fldt	4(%esp)
1:	fprem1
	fstsw	%ax
	sahf
	jp	1b
	fstp	%st(1)
	ret
END (__ieee754_remainderl)
strong_alias (__ieee754_remainderl, __remainderl_finite)
