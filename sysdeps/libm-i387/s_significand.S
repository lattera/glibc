/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: s_significand.S,v 1.4 1995/05/09 00:21:47 jtc Exp $")

ENTRY(__significand)
	fldl	4(%esp)
	fxtract
	fstpl	%st(1)
	ret
weak_alias (__significand, significand)
