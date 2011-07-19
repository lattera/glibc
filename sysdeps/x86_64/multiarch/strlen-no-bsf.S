/* strlen SSE2 without bsf
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

#if (defined SHARED || defined USE_AS_STRCAT) && !defined NOT_IN_libc

# ifndef USE_AS_STRCAT

#  include <sysdep.h>

#  define RETURN ret

	.section .text.sse2,"ax",@progbits
ENTRY (__strlen_no_bsf)
# endif
	xor	%eax, %eax
	cmpb	$0, (%rdi)
	jz	L(exit_tail0)
	cmpb	$0, 1(%rdi)
	jz	L(exit_tail1)
	cmpb	$0, 2(%rdi)
	jz	L(exit_tail2)
	cmpb	$0, 3(%rdi)
	jz	L(exit_tail3)
	cmpb	$0, 4(%rdi)
	jz	L(exit_tail4)
	cmpb	$0, 5(%rdi)
	jz	L(exit_tail5)
	cmpb	$0, 6(%rdi)
	jz	L(exit_tail6)
	cmpb	$0, 7(%rdi)
	jz	L(exit_tail7)
	cmpb	$0, 8(%rdi)
	jz	L(exit_tail8)
	cmpb	$0, 9(%rdi)
	jz	L(exit_tail9)
	cmpb	$0, 10(%rdi)
	jz	L(exit_tail10)
	cmpb	$0, 11(%rdi)
	jz	L(exit_tail11)
	cmpb	$0, 12(%rdi)
	jz	L(exit_tail12)
	cmpb	$0, 13(%rdi)
	jz	L(exit_tail13)
	cmpb	$0, 14(%rdi)
	jz	L(exit_tail14)
	cmpb	$0, 15(%rdi)
	jz	L(exit_tail15)
	pxor	%xmm0, %xmm0
	mov	%rdi, %rcx
	mov	%rdi, %rax
	and	$-16, %rax
	add	$16, %rax
	add	$16, %rcx

	pcmpeqb	(%rax), %xmm0
	pmovmskb %xmm0, %edx
	pxor	%xmm1, %xmm1
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm1
	pmovmskb %xmm1, %edx
	pxor	%xmm2, %xmm2
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)


	pcmpeqb	(%rax), %xmm2
	pmovmskb %xmm2, %edx
	pxor	%xmm3, %xmm3
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm3
	pmovmskb %xmm3, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm1
	pmovmskb %xmm1, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm2
	pmovmskb %xmm2, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm3
	pmovmskb %xmm3, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm1
	pmovmskb %xmm1, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm2
	pmovmskb %xmm2, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm3
	pmovmskb %xmm3, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm0
	pmovmskb %xmm0, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm1
	pmovmskb %xmm1, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm2
	pmovmskb %xmm2, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	pcmpeqb	(%rax), %xmm3
	pmovmskb %xmm3, %edx
	test	%edx, %edx
	lea	16(%rax), %rax
	jnz	L(exit)

	and	$-0x40, %rax
L(aligned_64):
	pcmpeqb	(%rax), %xmm0
	pcmpeqb	16(%rax), %xmm1
	pcmpeqb	32(%rax), %xmm2
	pcmpeqb	48(%rax), %xmm3
	pmovmskb %xmm0, %edx
	pmovmskb %xmm1, %r11d
	pmovmskb %xmm2, %r10d
	pmovmskb %xmm3, %r9d
	or	%edx, %r9d
	or	%r11d, %r9d
	or	%r10d, %r9d
	lea	64(%rax), %rax
	jz	L(aligned_64)

	test	%edx, %edx
	jnz	L(aligned_64_exit_16)
	test	%r11d, %r11d
	jnz	L(aligned_64_exit_32)
	test	%r10d, %r10d
	jnz	L(aligned_64_exit_48)
L(aligned_64_exit_64):
	pmovmskb %xmm3, %edx
	jmp	L(aligned_64_exit)
L(aligned_64_exit_48):
	lea	-16(%rax), %rax
	mov	%r10d, %edx
	jmp	L(aligned_64_exit)
L(aligned_64_exit_32):
	lea	-32(%rax), %rax
	mov	%r11d, %edx
	jmp	L(aligned_64_exit)
L(aligned_64_exit_16):
	lea	-48(%rax), %rax
L(aligned_64_exit):
L(exit):
	sub	%rcx, %rax
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
	RETURN
END (__strlen_no_bsf)
# endif
#endif
