/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Changes for long double by Ulrich Drepper <drepper@cygnus.com>
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: $")

ENTRY(__ceill)
	pushl	%ebp
	movl	%esp,%ebp
	subl	$8,%esp

	fstcw	-4(%ebp)		/* store fpu control word */

	/* We use here %edx although only the low 1 bits are defined.
	   But none of the operations should care and they are faster
	   than the 16 bit operations.  */
	movl	-4(%ebp),%edx
	orl	$0x0800,%edx		/* round towards +oo */
	andl	$0xfbff,%edx
	movl	%edx,-8(%ebp)
	fldcw	-8(%ebp)		/* load modfied control word */

	fldt	8(%ebp);		/* round */
	frndint

	fldcw	-4(%ebp)		/* restore original control word */

	leave
	ret
PSEUDO_END (__ceill)
weak_alias (__ceill, ceill)
