/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Changes for long double by Ulrich Drepper <drepper@cygnus.com>
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: $")

ENTRY(__ilogbl)
	fldt	4(%esp)
	fxtract
	pushl	%eax
	fstp	%st

	fistpl	(%esp)
	popl	%eax

	ret
END (__ilogbl)
weak_alias (__ilogbl, ilogbl)
