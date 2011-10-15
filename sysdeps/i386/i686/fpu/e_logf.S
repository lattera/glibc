/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 * Adapted for float by Ulrich Drepper <drepper@cygnus.com>.
 *
 * Adapted for i686 instructions.
 */

#include <machine/asm.h>


	.text
ENTRY(__ieee754_logf)
	fldln2			// log(2)
	flds	4(%esp)		// x : log(2)
	fucomi	%st
	jp	3f
	fyl2x			// log(x)
	ret

3:	fstp	%st(1)
	ret
END (__ieee754_logf)

ENTRY(__logf_finite)
	fldln2			// log(2)
	flds	4(%esp)		// x : log(2)
	fyl2x			// log(x)
	ret
END(__logf_finite)
