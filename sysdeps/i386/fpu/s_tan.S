/*
 * Written by J.T. Conklin <jtc@netbsd.org>.
 * Fixed errno handling by Ulrich Drepper <drepper@redhat.com>.
 * Public domain.
 */

#define __need_Emath
#include <bits/errno.h>
#include <machine/asm.h>

RCSID("$NetBSD: s_tan.S,v 1.5 1995/05/09 00:30:00 jtc Exp $")

ENTRY(__tan)
	fldl	4(%esp)
	fxam
	fstsw	%ax
	movb	$0x45, %dh
	andb	%ah, %dh
	cmpb	$0x05, %dh
	je	3f
4:	fptan
	fnstsw	%ax
	testl	$0x400,%eax
	jnz	1f
	fstp	%st(0)
	ret
1:	fldpi
	fadd	%st(0)
	fxch	%st(1)
2:	fprem1
	fstsw	%ax
	testl	$0x400,%eax
	jnz	2b
	fstp	%st(1)
	fptan
	fstp	%st(0)
	ret
3:
#ifdef PIC
	pushl	%ebx
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (ebx, 0)
	LOAD_PIC_REG (bx)
	call	__errno_location@PLT
	movl	$EDOM, (%eax)
	popl	%ebx
	cfi_adjust_cfa_offset (-4)
	cfi_restore (ebx)
#else
	call	__errno_location@PLT
	movl	$EDOM, (%eax)
#endif
	jmp	4b
END (__tan)
weak_alias (__tan, tan)
