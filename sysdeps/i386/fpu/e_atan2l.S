/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 *
 * Adapted for `long double' by Ulrich Drepper <drepper@cygnus.com>.
 */

#include <machine/asm.h>

RCSID("$NetBSD: $")

ENTRY(__ieee754_atan2l)
	fldt	 4(%esp)
	fldt	16(%esp)
	fpatan
	ret
END (__ieee754_atan2l)
