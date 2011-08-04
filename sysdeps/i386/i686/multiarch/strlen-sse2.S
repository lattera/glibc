/* strlen with SSE2
   Copyright (C) 2010, 2011 Free Software Foundation, Inc.
   Contributed by Intel Corporation.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#if (defined USE_AS_STRCAT || defined SHARED) && !defined NOT_IN_libc
# ifndef USE_AS_STRCAT

#  include <sysdep.h>
#  include "asm-syntax.h"

#  define CFI_PUSH(REG)	\
	cfi_adjust_cfa_offset (4);	\
	cfi_rel_offset (REG, 0)

#  define CFI_POP(REG)	\
	cfi_adjust_cfa_offset (-4);	\
	cfi_restore (REG)

#  define PUSH(REG)	pushl REG; CFI_PUSH (REG)
#  define POP(REG)	popl REG; CFI_POP (REG)
#  define PARMS		4
#  define STR		PARMS
#  define ENTRANCE
#  define RETURN		ret

	.text
ENTRY (__strlen_sse2)
	ENTRANCE
	mov	STR(%esp), %edx
# endif
	xor	%eax, %eax
	cmpb	$0, (%edx)
	jz	L(exit_tail0)
	cmpb	$0, 1(%edx)
	jz	L(exit_tail1)
	cmpb	$0, 2(%edx)
	jz	L(exit_tail2)
	cmpb	$0, 3(%edx)
	jz	L(exit_tail3)
	cmpb	$0, 4(%edx)
	jz	L(exit_tail4)
	cmpb	$0, 5(%edx)
	jz	L(exit_tail5)
	cmpb	$0, 6(%edx)
	jz	L(exit_tail6)
	cmpb	$0, 7(%edx)
	jz	L(exit_tail7)
	cmpb	$0, 8(%edx)
	jz	L(exit_tail8)
	cmpb	$0, 9(%edx)
	jz	L(exit_tail9)
	cmpb	$0, 10(%edx)
	jz	L(exit_tail10)
	cmpb	$0, 11(%edx)
	jz	L(exit_tail11)
	cmpb	$0, 12(%edx)
	jz	L(exit_tail12)
	cmpb	$0, 13(%edx)
	jz	L(exit_tail13)
	cmpb	$0, 14(%edx)
	jz	L(exit_tail14)
	cmpb	$0, 15(%edx)
	jz	L(exit_tail15)
	pxor	%xmm0, %xmm0
	mov	%edx, %eax
	lea	16(%edx), %ecx
	and	$-16, %eax
	add	$16, %eax

	pcmpeqb	(%eax), %xmm0
	pmovmskb %xmm0, %edx
	pxor	%xmm1, %xmm1
	test	%edx, %edx
	lea	16(%eax), %eax
	jnz	L(exit)

	pcmpeqb	(%eax), %xmm1
	pmovmskb %xmm1, %edx
	pxor	%xmm2, %xmm2
	test	%edx, %edx
	lea	16(%eax), %eax
	jnz	L(exit)


	pcmpeqb	(%eax), %xmm2
	pmovmskb %xmm2, %edx
	pxor	%xmm3, %xmm3
	test	%edx, %edx
	lea	16(%eax), %eax
	jnz	L(exit)

	pcmpeqb	(%eax), %xmm3
	pmovmskb %xmm3, %edx
	test	%edx, %edx
	lea	16(%eax), %eax
	jnz	L(exit)

	pcmpeqb	(%eax), %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	lea	16(%eax), %eax
	jnz	L(exit)

	pcmpeqb	(%eax), %xmm1
	pmovmskb %xmm1, %edx
	test	%edx, %edx
	lea	16(%eax), %eax
	jnz	L(exit)

	pcmpeqb	(%eax), %xmm2
	pmovmskb %xmm2, %edx
	test	%edx, %edx
	lea	16(%eax), %eax
	jnz	L(exit)

	pcmpeqb	(%eax), %xmm3
	pmovmskb %xmm3, %edx
	test	%edx, %edx
	lea	16(%eax), %eax
	jnz	L(exit)

	pcmpeqb	(%eax), %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	lea	16(%eax), %eax
	jnz	L(exit)

	pcmpeqb	(%eax), %xmm1
	pmovmskb %xmm1, %edx
	test	%edx, %edx
	lea	16(%eax), %eax
	jnz	L(exit)

	pcmpeqb	(%eax), %xmm2
	pmovmskb %xmm2, %edx
	test	%edx, %edx
	lea	16(%eax), %eax
	jnz	L(exit)

	pcmpeqb	(%eax), %xmm3
	pmovmskb %xmm3, %edx
	test	%edx, %edx
	lea	16(%eax), %eax
	jnz	L(exit)

	pcmpeqb	(%eax), %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	lea	16(%eax), %eax
	jnz	L(exit)

	pcmpeqb	(%eax), %xmm1
	pmovmskb %xmm1, %edx
	test	%edx, %edx
	lea	16(%eax), %eax
	jnz	L(exit)

	pcmpeqb	(%eax), %xmm2
	pmovmskb %xmm2, %edx
	test	%edx, %edx
	lea	16(%eax), %eax
	jnz	L(exit)

	pcmpeqb	(%eax), %xmm3
	pmovmskb %xmm3, %edx
	test	%edx, %edx
	lea	16(%eax), %eax
	jnz	L(exit)

	and	$-0x40, %eax
L(aligned_64):
	movaps	(%eax), %xmm0
	movaps	16(%eax), %xmm1
	movaps	32(%eax), %xmm2
	movaps	48(%eax), %xmm6
	pminub	%xmm1, %xmm0
	pminub	%xmm6, %xmm2
	pminub	%xmm0, %xmm2
	pcmpeqb	%xmm3, %xmm2
	pmovmskb %xmm2, %edx
	test	%edx, %edx
	lea	64(%eax), %eax
	jz	L(aligned_64)

	pcmpeqb	-64(%eax), %xmm3
	pmovmskb %xmm3, %edx
	test	%edx, %edx
	lea	48(%ecx), %ecx
	jnz	L(exit)

	pcmpeqb	%xmm1, %xmm3
	pmovmskb %xmm3, %edx
	test	%edx, %edx
	lea	-16(%ecx), %ecx
	jnz	L(exit)

	pcmpeqb	-32(%eax), %xmm3
	pmovmskb %xmm3, %edx
	test	%edx, %edx
	lea	-16(%ecx), %ecx
	jnz	L(exit)

	pcmpeqb	%xmm6, %xmm3
	pmovmskb %xmm3, %edx
	lea	-16(%ecx), %ecx
L(exit):
	sub	%ecx, %eax
	test	%dl, %dl
	jz	L(exit_high)
	test	$0x01, %dl
	jnz	L(exit_tail0)

	test	$0x02, %dl
	jnz	L(exit_tail1)

	test	$0x04, %dl
	jnz	L(exit_tail2)

	test	$0x08, %dl
	jnz	L(exit_tail3)

	test	$0x10, %dl
	jnz	L(exit_tail4)

	test	$0x20, %dl
	jnz	L(exit_tail5)

	test	$0x40, %dl
	jnz	L(exit_tail6)
	add	$7, %eax
L(exit_tail0):
	RETURN

L(exit_high):
	add	$8, %eax
	test	$0x01, %dh
	jnz	L(exit_tail0)

	test	$0x02, %dh
	jnz	L(exit_tail1)

	test	$0x04, %dh
	jnz	L(exit_tail2)

	test	$0x08, %dh
	jnz	L(exit_tail3)

	test	$0x10, %dh
	jnz	L(exit_tail4)

	test	$0x20, %dh
	jnz	L(exit_tail5)

	test	$0x40, %dh
	jnz	L(exit_tail6)
	add	$7, %eax
	RETURN

	.p2align 4
L(exit_tail1):
	add	$1, %eax
	RETURN

L(exit_tail2):
	add	$2, %eax
	RETURN

L(exit_tail3):
	add	$3, %eax
	RETURN

L(exit_tail4):
	add	$4, %eax
	RETURN

L(exit_tail5):
	add	$5, %eax
	RETURN

L(exit_tail6):
	add	$6, %eax
	RETURN

L(exit_tail7):
	add	$7, %eax
	RETURN

L(exit_tail8):
	add	$8, %eax
	RETURN

L(exit_tail9):
	add	$9, %eax
	RETURN

L(exit_tail10):
	add	$10, %eax
	RETURN

L(exit_tail11):
	add	$11, %eax
	RETURN

L(exit_tail12):
	add	$12, %eax
	RETURN

L(exit_tail13):
	add	$13, %eax
	RETURN

L(exit_tail14):
	add	$14, %eax
	RETURN

L(exit_tail15):
	add	$15, %eax
# ifndef USE_AS_STRCAT
	ret
END (__strlen_sse2)
# endif
#endif
