/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Changes for long double by Ulrich Drepper <drepper@cygnus.com>
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: $")

ENTRY(__logbl)
	fldt	4(%esp)
	fxtract
	ret
PSEUDO_END (__logbl)
weak_alias (__logbl, logbl)
