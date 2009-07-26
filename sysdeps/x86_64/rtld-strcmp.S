#include <sysdep.h>
#include "asm-syntax.h"
#include "bp-sym.h"
#include "bp-asm.h"

#ifndef LABEL
#define LABEL(l) L(l)
#endif

	.text
ENTRY (BP_SYM (STRCMP))
/* Simple version since we can't use SSE registers in ld.so.  */
L(oop):	movb	(%rdi), %al
	cmpb	(%rsi), %al
	jne	L(neq)
	incq	%rdi
	incq	%rsi
	testb	%al, %al
	jnz	L(oop)

	xorl	%eax, %eax
	ret

L(neq):	movl	$1, %eax
	movl	$-1, %ecx
	cmovbl	%ecx, %eax
	ret
END (BP_SYM (STRCMP))
