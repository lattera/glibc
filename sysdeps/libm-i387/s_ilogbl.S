/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Changes for long double by Ulrich Drepper <drepper@cygnus.com>
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: $")

ENTRY(__ilogbl)
	pushl	%ebp
	movl	%esp,%ebp
	subl	$4,%esp

	fldt	8(%ebp)
	fxtract

	fistpl	-4(%ebp)
	movl	-4(%ebp),%eax

	leave
	ret
PSEUDO_END (__ilogbl)
weak_alias (__ilogbl, ilogbl)
