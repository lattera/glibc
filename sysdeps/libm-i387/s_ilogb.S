/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: s_ilogb.S,v 1.5 1995/10/12 15:53:09 jtc Exp $")

ENTRY(__ilogb)
	fldl	4(%esp)
	fxtract
	pushl	%eax
	fstp	%st

	fistpl	(%esp)
	fwait
	popl	%eax

	ret
END (__ilogb)
weak_alias (__ilogb, ilogb)
