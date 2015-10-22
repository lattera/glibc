/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */
/* Adapted for use as nearbyint by Ulrich Drepper <drepper@cygnus.com>.  */

#include <machine/asm.h>

ENTRY(__nearbyintl)
	fldt	8(%rsp)
	fnstenv	-28(%rsp)
	movl	-28(%rsp), %eax
	orl	$0x20, %eax
	movl	%eax, -32(%rsp)
	fldcw	-32(%rsp)
	frndint
	fnstsw
	andl	$0x1, %eax
	orl	%eax, -24(%rsp)
	fldenv	-28(%rsp)
	ret
END (__nearbyintl)
weak_alias (__nearbyintl, nearbyintl)
