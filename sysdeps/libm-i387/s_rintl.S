/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Changes for long double by Ulrich Drepper <drepper@cygnus.com>
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: $")

ENTRY(__rintl)
	fldt	4(%esp)
	frndint
	ret
PSEUDO_END (__rintl)
weak_alias (__rintl, rintl)
