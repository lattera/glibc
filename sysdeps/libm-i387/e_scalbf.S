/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 * Adapted for float type by Ulrich Drepper <drepper@cygnus.com>.
 */

#include <machine/asm.h>

RCSID("$NetBSD: $")

ENTRY(__ieee754_scalbf)
	flds	8(%esp)
	flds	4(%esp)
	fscale
	ret
END(__ieee754_scalbf)
