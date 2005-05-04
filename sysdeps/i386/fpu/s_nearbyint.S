/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */
/* Adapted for use as nearbyint by Ulrich Drepper <drepper@cygnus.com>.  */

#include <machine/asm.h>

ENTRY(__nearbyint)
	fldl	4(%esp)
	pushl	%eax
	cfi_adjust_cfa_offset (4)
	pushl	%ecx
	cfi_adjust_cfa_offset (4)
	fnstcw	(%esp)
	movl	(%esp), %eax
	orl	$0x20, %eax
	movl	%eax, 4(%esp)
	fldcw	4(%esp)
	frndint
	fclex
	fldcw	(%esp)
	popl	%ecx
	cfi_adjust_cfa_offset (-4)
	popl	%eax
	cfi_adjust_cfa_offset (-4)
	ret
END (__nearbyint)
weak_alias (__nearbyint, nearbyint)
