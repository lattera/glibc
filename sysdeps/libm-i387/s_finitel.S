/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 * Adapted for `long double' by Ulrich Drepper <drepper@cygnus.com>.
 */

#include <machine/asm.h>

RCSID("$NetBSD: $")

ENTRY(__finitel)
	movl	12(%esp),%eax
	andl	$0x7fff, %eax
	cmpl	$0x7fff, %eax
	setne	%al
	andl	$0x000000ff, %eax
	ret
PSEUDO_END (__finitel)
weak_alias (__finitel, finitel)
