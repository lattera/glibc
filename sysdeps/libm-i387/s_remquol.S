/*
 * Written by Ulrich Drepper <drepper@cygnus.com>.
 * Based on e_remainder by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

ENTRY(__remquol)
	fldt	16(%esp)
	fldt	4(%esp)
1:	fprem1
	fstsw	%ax
	sahf
	jp	1b
	fstp	%st(1)
	/* Compute the congruent of the quotient.  */
	movl	%eax, %ecx
	shrl	$8, %eax
	shrl	$12, %ecx
	andl	$4, %ecx
	andl	$3, %eax
	orl	%eax, %ecx
	movl	$0xef2960, %eax
	shrl	%cl, %eax
	andl	$3, %eax
	movl	28(%esp), %ecx
	movl	12(%esp), %edx
	xorl	24(%esp), %edx
	testl	$0x8000, %edx
	jz	1f
	negl	%eax
1:	movl	%eax, (%ecx)
	ret
END (__remquol)
weak_alias (__remquol, remquol)
