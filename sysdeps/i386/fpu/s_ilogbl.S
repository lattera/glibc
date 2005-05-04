/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Changes for long double by Ulrich Drepper <drepper@cygnus.com>
 * Public domain.
 */

#include <machine/asm.h>

RCSID("$NetBSD: $")

ENTRY(__ilogbl)
	fldt	4(%esp)
/* I added the following ugly construct because ilogb(+-Inf) is
   required to return INT_MAX in ISO C99.
   -- jakub@redhat.com.  */
	fxam			/* Is NaN or +-Inf?  */
	fstsw   %ax
	movb    $0x45, %dh
	andb    %ah, %dh
	cmpb    $0x05, %dh
	je      1f		/* Is +-Inf, jump.  */

	fxtract
	pushl	%eax
	cfi_adjust_cfa_offset (4)
	fstp	%st

	fistpl	(%esp)
	fwait
	popl	%eax
	cfi_adjust_cfa_offset (-4)

	ret

1:	fstp	%st
	movl	$0x7fffffff, %eax
	ret
END (__ilogbl)
weak_alias (__ilogbl, ilogbl)
