/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: s_ilogbf.S,v 1.4 1995/10/22 20:32:43 pk Exp $")

ENTRY(__ilogbf)
	flds	4(%esp)
	fxtract
	pushl	%eax
	fstp	%st

	fistpl	(%esp)
	fwait
	popl	%eax

	ret
END (__ilogbf)
weak_alias (__ilogbf, ilogbf)
