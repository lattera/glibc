/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */
/* Adapted for use as nearbyint by Ulrich Drepper <drepper@cygnus.com>.  */

#include <machine/asm.h>

ENTRY(__nearbyintl)
	fldt	4(%esp)
	subl	$32, %esp
	cfi_adjust_cfa_offset (32)
	fnstenv	4(%esp)
	frndint
	fnstsw
	andl	$0x1, %eax
	orl	%eax, 8(%esp)
	fldenv	4(%esp)
	addl	$32, %esp
	cfi_adjust_cfa_offset (-32)
	ret
END (__nearbyintl)
weak_alias (__nearbyintl, nearbyintl)
