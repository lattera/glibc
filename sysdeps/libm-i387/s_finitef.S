/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: s_finitef.S,v 1.3 1995/05/09 00:00:02 jtc Exp $")

ENTRY(__finitef)
	movl	4(%esp),%eax
	andl	$0x7f800000, %eax
	cmpl	$0x7f800000, %eax
	setne	%al
	andl	$0x000000ff, %eax
	ret
PSEUDO_END (__finitef)
weak_alias (__finitef, finitef)
