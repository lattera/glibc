/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 * Adapted for `long double' by Ulrich Drepper <drepper@cygnus.com>.
 */

#include <machine/asm.h>

RCSID("$NetBSD: $")

/*
 * Since the fyl2xp1 instruction has such a limited range:
 *	-(1 - (sqrt(2) / 2)) <= x <= sqrt(2) - 1
 * it's not worth trying to use it.
 */

ENTRY(__log1pl)
	fldln2
	fldt 4(%esp)
	fld1
	faddp
	fyl2x
	ret
PSEUDO_END (__log1pl)
weak_alias (__log1pl, log1pl)
