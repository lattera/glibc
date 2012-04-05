/* memset/bzero -- set memory area to CH/0
   Optimized version for x86-64.
   Copyright (C) 2002-2012 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

#define __STOS_LOWER_BOUNDARY	$8192
#define __STOS_UPPER_BOUNDARY	$65536

	.text
#if !defined NOT_IN_libc && !defined USE_MULTIARCH
ENTRY(__bzero)
	mov	%rsi,%rdx	/* Adjust parameter.  */
	xorl	%esi,%esi	/* Fill with 0s.  */
	jmp	L(memset_entry)
END(__bzero)
weak_alias (__bzero, bzero)
#endif

#if defined PIC && !defined NOT_IN_libc
ENTRY_CHK (__memset_chk)
	cmpq	%rdx, %rcx
	jb	HIDDEN_JUMPTARGET (__chk_fail)
END_CHK (__memset_chk)
#endif
ENTRY (memset)
L(memset_entry):
	cmp    $0x1,%rdx
	mov    %rdi,%rax	/* memset returns the dest address.  */
	jne    L(ck2)
	mov    %sil,(%rdi)
	retq
L(ck2):
	mov    $0x101010101010101,%r9
	mov    %rdx,%r8
	movzbq %sil,%rdx
	imul   %r9,%rdx
L(now_dw_aligned):
	cmp    $0x90,%r8
	ja     L(ck_mem_ops_method)
L(now_dw_aligned_small):
	add    %r8,%rdi
#ifndef PIC
	lea    L(setPxQx)(%rip),%r11
	jmpq   *(%r11,%r8,8)
#else
	lea    L(Got0)(%rip),%r11
	lea    L(setPxQx)(%rip),%rcx
	movswq (%rcx,%r8,2),%rcx
	lea    (%rcx,%r11,1),%r11
	jmpq   *%r11
#endif

L(Got0):
	retq

	.pushsection .rodata
	.balign     16
#ifndef PIC
L(setPxQx):
	.quad       L(Got0), L(P1Q0), L(P2Q0), L(P3Q0)
	.quad       L(P4Q0), L(P5Q0), L(P6Q0), L(P7Q0)
	.quad       L(P0Q1), L(P1Q1), L(P2Q1), L(P3Q1)
	.quad       L(P4Q1), L(P5Q1), L(P6Q1), L(P7Q1)
	.quad       L(P0Q2), L(P1Q2), L(P2Q2), L(P3Q2)
	.quad       L(P4Q2), L(P5Q2), L(P6Q2), L(P7Q2)
	.quad       L(P0Q3), L(P1Q3), L(P2Q3), L(P3Q3)
	.quad       L(P4Q3), L(P5Q3), L(P6Q3), L(P7Q3)
	.quad       L(P0Q4), L(P1Q4), L(P2Q4), L(P3Q4)
	.quad       L(P4Q4), L(P5Q4), L(P6Q4), L(P7Q4)
	.quad       L(P0Q5), L(P1Q5), L(P2Q5), L(P3Q5)
	.quad       L(P4Q5), L(P5Q5), L(P6Q5), L(P7Q5)
	.quad       L(P0Q6), L(P1Q6), L(P2Q6), L(P3Q6)
	.quad       L(P4Q6), L(P5Q6), L(P6Q6), L(P7Q6)
	.quad       L(P0Q7), L(P1Q7), L(P2Q7), L(P3Q7)
	.quad       L(P4Q7), L(P5Q7), L(P6Q7), L(P7Q7)
	.quad       L(P0Q8), L(P1Q8), L(P2Q8), L(P3Q8)
	.quad       L(P4Q8), L(P5Q8), L(P6Q8), L(P7Q8)
	.quad       L(P0Q9), L(P1Q9), L(P2Q9), L(P3Q9)
	.quad       L(P4Q9), L(P5Q9), L(P6Q9), L(P7Q9)
	.quad       L(P0QA), L(P1QA), L(P2QA), L(P3QA)
	.quad       L(P4QA), L(P5QA), L(P6QA), L(P7QA)
	.quad       L(P0QB), L(P1QB), L(P2QB), L(P3QB)
	.quad       L(P4QB), L(P5QB), L(P6QB), L(P7QB)
	.quad       L(P0QC), L(P1QC), L(P2QC), L(P3QC)
	.quad       L(P4QC), L(P5QC), L(P6QC), L(P7QC)
	.quad       L(P0QD), L(P1QD), L(P2QD), L(P3QD)
	.quad       L(P4QD), L(P5QD), L(P6QD), L(P7QD)
	.quad       L(P0QE), L(P1QE), L(P2QE), L(P3QE)
	.quad       L(P4QE), L(P5QE), L(P6QE), L(P7QE)
	.quad       L(P0QF), L(P1QF), L(P2QF), L(P3QF)
	.quad       L(P4QF), L(P5QF), L(P6QF), L(P7QF)
	.quad       L(P0QG), L(P1QG), L(P2QG), L(P3QG)
	.quad       L(P4QG), L(P5QG), L(P6QG), L(P7QG)
	.quad       L(P0QH), L(P1QH), L(P2QH), L(P3QH)
	.quad       L(P4QH), L(P5QH), L(P6QH), L(P7QH)
	.quad       L(P0QI)
# ifdef USE_EXTRA_TABLE
	.quad       L(P1QI), L(P2QI), L(P3QI), L(P4QI)
	.quad       L(P5QI), L(P6QI), L(P7QI)
# endif
#else
L(setPxQx):
	.short     L(Got0)-L(Got0)
	.short     L(P1Q0)-L(Got0)
	.short     L(P2Q0)-L(Got0)
	.short     L(P3Q0)-L(Got0)
	.short     L(P4Q0)-L(Got0)
	.short     L(P5Q0)-L(Got0)
	.short     L(P6Q0)-L(Got0)
	.short     L(P7Q0)-L(Got0)

	.short     L(P0Q1)-L(Got0)
	.short     L(P1Q1)-L(Got0)
	.short     L(P2Q1)-L(Got0)
	.short     L(P3Q1)-L(Got0)
	.short     L(P4Q1)-L(Got0)
	.short     L(P5Q1)-L(Got0)
	.short     L(P6Q1)-L(Got0)
	.short     L(P7Q1)-L(Got0)

	.short     L(P0Q2)-L(Got0)
	.short     L(P1Q2)-L(Got0)
	.short     L(P2Q2)-L(Got0)
	.short     L(P3Q2)-L(Got0)
	.short     L(P4Q2)-L(Got0)
	.short     L(P5Q2)-L(Got0)
	.short     L(P6Q2)-L(Got0)
	.short     L(P7Q2)-L(Got0)

	.short     L(P0Q3)-L(Got0)
	.short     L(P1Q3)-L(Got0)
	.short     L(P2Q3)-L(Got0)
	.short     L(P3Q3)-L(Got0)
	.short     L(P4Q3)-L(Got0)
	.short     L(P5Q3)-L(Got0)
	.short     L(P6Q3)-L(Got0)
	.short     L(P7Q3)-L(Got0)

	.short     L(P0Q4)-L(Got0)
	.short     L(P1Q4)-L(Got0)
	.short     L(P2Q4)-L(Got0)
	.short     L(P3Q4)-L(Got0)
	.short     L(P4Q4)-L(Got0)
	.short     L(P5Q4)-L(Got0)
	.short     L(P6Q4)-L(Got0)
	.short     L(P7Q4)-L(Got0)

	.short     L(P0Q5)-L(Got0)
	.short     L(P1Q5)-L(Got0)
	.short     L(P2Q5)-L(Got0)
	.short     L(P3Q5)-L(Got0)
	.short     L(P4Q5)-L(Got0)
	.short     L(P5Q5)-L(Got0)
	.short     L(P6Q5)-L(Got0)
	.short     L(P7Q5)-L(Got0)

	.short     L(P0Q6)-L(Got0)
	.short     L(P1Q6)-L(Got0)
	.short     L(P2Q6)-L(Got0)
	.short     L(P3Q6)-L(Got0)
	.short     L(P4Q6)-L(Got0)
	.short     L(P5Q6)-L(Got0)
	.short     L(P6Q6)-L(Got0)
	.short     L(P7Q6)-L(Got0)

	.short     L(P0Q7)-L(Got0)
	.short     L(P1Q7)-L(Got0)
	.short     L(P2Q7)-L(Got0)
	.short     L(P3Q7)-L(Got0)
	.short     L(P4Q7)-L(Got0)
	.short     L(P5Q7)-L(Got0)
	.short     L(P6Q7)-L(Got0)
	.short     L(P7Q7)-L(Got0)

	.short     L(P0Q8)-L(Got0)
	.short     L(P1Q8)-L(Got0)
	.short     L(P2Q8)-L(Got0)
	.short     L(P3Q8)-L(Got0)
	.short     L(P4Q8)-L(Got0)
	.short     L(P5Q8)-L(Got0)
	.short     L(P6Q8)-L(Got0)
	.short     L(P7Q8)-L(Got0)

	.short     L(P0Q9)-L(Got0)
	.short     L(P1Q9)-L(Got0)
	.short     L(P2Q9)-L(Got0)
	.short     L(P3Q9)-L(Got0)
	.short     L(P4Q9)-L(Got0)
	.short     L(P5Q9)-L(Got0)
	.short     L(P6Q9)-L(Got0)
	.short     L(P7Q9)-L(Got0)

	.short     L(P0QA)-L(Got0)
	.short     L(P1QA)-L(Got0)
	.short     L(P2QA)-L(Got0)
	.short     L(P3QA)-L(Got0)
	.short     L(P4QA)-L(Got0)
	.short     L(P5QA)-L(Got0)
	.short     L(P6QA)-L(Got0)
	.short     L(P7QA)-L(Got0)

	.short     L(P0QB)-L(Got0)
	.short     L(P1QB)-L(Got0)
	.short     L(P2QB)-L(Got0)
	.short     L(P3QB)-L(Got0)
	.short     L(P4QB)-L(Got0)
	.short     L(P5QB)-L(Got0)
	.short     L(P6QB)-L(Got0)
	.short     L(P7QB)-L(Got0)

	.short     L(P0QC)-L(Got0)
	.short     L(P1QC)-L(Got0)
	.short     L(P2QC)-L(Got0)
	.short     L(P3QC)-L(Got0)
	.short     L(P4QC)-L(Got0)
	.short     L(P5QC)-L(Got0)
	.short     L(P6QC)-L(Got0)
	.short     L(P7QC)-L(Got0)

	.short     L(P0QD)-L(Got0)
	.short     L(P1QD)-L(Got0)
	.short     L(P2QD)-L(Got0)
	.short     L(P3QD)-L(Got0)
	.short     L(P4QD)-L(Got0)
	.short     L(P5QD)-L(Got0)
	.short     L(P6QD)-L(Got0)
	.short     L(P7QD)-L(Got0)

	.short     L(P0QE)-L(Got0)
	.short     L(P1QE)-L(Got0)
	.short     L(P2QE)-L(Got0)
	.short     L(P3QE)-L(Got0)
	.short     L(P4QE)-L(Got0)
	.short     L(P5QE)-L(Got0)
	.short     L(P6QE)-L(Got0)
	.short     L(P7QE)-L(Got0)

	.short     L(P0QF)-L(Got0)
	.short     L(P1QF)-L(Got0)
	.short     L(P2QF)-L(Got0)
	.short     L(P3QF)-L(Got0)
	.short     L(P4QF)-L(Got0)
	.short     L(P5QF)-L(Got0)
	.short     L(P6QF)-L(Got0)
	.short     L(P7QF)-L(Got0)

	.short     L(P0QG)-L(Got0)
	.short     L(P1QG)-L(Got0)
	.short     L(P2QG)-L(Got0)
	.short     L(P3QG)-L(Got0)
	.short     L(P4QG)-L(Got0)
	.short     L(P5QG)-L(Got0)
	.short     L(P6QG)-L(Got0)
	.short     L(P7QG)-L(Got0)

	.short     L(P0QH)-L(Got0)
	.short     L(P1QH)-L(Got0)
	.short     L(P2QH)-L(Got0)
	.short     L(P3QH)-L(Got0)
	.short     L(P4QH)-L(Got0)
	.short     L(P5QH)-L(Got0)
	.short     L(P6QH)-L(Got0)
	.short     L(P7QH)-L(Got0)

	.short     L(P0QI)-L(Got0)
# ifdef USE_EXTRA_TABLE
	.short     L(P1QI)-L(Got0)
	.short     L(P2QI)-L(Got0)
	.short     L(P3QI)-L(Got0)
	.short     L(P4QI)-L(Got0)
	.short     L(P5QI)-L(Got0)
	.short     L(P6QI)-L(Got0)
	.short     L(P7QI)-L(Got0)
# endif
#endif
	.popsection

	.balign     16
#ifdef USE_EXTRA_TABLE
L(P1QI): mov    %rdx,-0x91(%rdi)
#endif
L(P1QH): mov    %rdx,-0x89(%rdi)
L(P1QG): mov    %rdx,-0x81(%rdi)
#		   .balign     16
L(P1QF): mov    %rdx,-0x79(%rdi)
L(P1QE): mov    %rdx,-0x71(%rdi)
L(P1QD): mov    %rdx,-0x69(%rdi)
L(P1QC): mov    %rdx,-0x61(%rdi)
L(P1QB): mov    %rdx,-0x59(%rdi)
L(P1QA): mov    %rdx,-0x51(%rdi)
L(P1Q9): mov    %rdx,-0x49(%rdi)
L(P1Q8): mov    %rdx,-0x41(%rdi)
L(P1Q7): mov    %rdx,-0x39(%rdi)
L(P1Q6): mov    %rdx,-0x31(%rdi)
L(P1Q5): mov    %rdx,-0x29(%rdi)
L(P1Q4): mov    %rdx,-0x21(%rdi)
L(P1Q3): mov    %rdx,-0x19(%rdi)
L(P1Q2): mov    %rdx,-0x11(%rdi)
L(P1Q1): mov    %rdx,-0x9(%rdi)
L(P1Q0): mov    %dl,-0x1(%rdi)
		retq

	.balign     16
L(P0QI): mov    %rdx,-0x90(%rdi)
L(P0QH): mov    %rdx,-0x88(%rdi)
#		   .balign     16
L(P0QG): mov    %rdx,-0x80(%rdi)
L(P0QF): mov    %rdx,-0x78(%rdi)
L(P0QE): mov    %rdx,-0x70(%rdi)
L(P0QD): mov    %rdx,-0x68(%rdi)
L(P0QC): mov    %rdx,-0x60(%rdi)
L(P0QB): mov    %rdx,-0x58(%rdi)
L(P0QA): mov    %rdx,-0x50(%rdi)
L(P0Q9): mov    %rdx,-0x48(%rdi)
L(P0Q8): mov    %rdx,-0x40(%rdi)
L(P0Q7): mov    %rdx,-0x38(%rdi)
L(P0Q6): mov    %rdx,-0x30(%rdi)
L(P0Q5): mov    %rdx,-0x28(%rdi)
L(P0Q4): mov    %rdx,-0x20(%rdi)
L(P0Q3): mov    %rdx,-0x18(%rdi)
L(P0Q2): mov    %rdx,-0x10(%rdi)
L(P0Q1): mov    %rdx,-0x8(%rdi)
L(P0Q0): retq


	.balign     16
#ifdef USE_EXTRA_TABLE
L(P2QI): mov    %rdx,-0x92(%rdi)
#endif
L(P2QH): mov    %rdx,-0x8a(%rdi)
L(P2QG): mov    %rdx,-0x82(%rdi)
#		   .balign     16
L(P2QF): mov    %rdx,-0x7a(%rdi)
L(P2QE): mov    %rdx,-0x72(%rdi)
L(P2QD): mov    %rdx,-0x6a(%rdi)
L(P2QC): mov    %rdx,-0x62(%rdi)
L(P2QB): mov    %rdx,-0x5a(%rdi)
L(P2QA): mov    %rdx,-0x52(%rdi)
L(P2Q9): mov    %rdx,-0x4a(%rdi)
L(P2Q8): mov    %rdx,-0x42(%rdi)
L(P2Q7): mov    %rdx,-0x3a(%rdi)
L(P2Q6): mov    %rdx,-0x32(%rdi)
L(P2Q5): mov    %rdx,-0x2a(%rdi)
L(P2Q4): mov    %rdx,-0x22(%rdi)
L(P2Q3): mov    %rdx,-0x1a(%rdi)
L(P2Q2): mov    %rdx,-0x12(%rdi)
L(P2Q1): mov    %rdx,-0xa(%rdi)
L(P2Q0): mov    %dx,-0x2(%rdi)
		retq

	.balign     16
#ifdef USE_EXTRA_TABLE
L(P3QI): mov    %rdx,-0x93(%rdi)
#endif
L(P3QH): mov    %rdx,-0x8b(%rdi)
L(P3QG): mov    %rdx,-0x83(%rdi)
#		   .balign     16
L(P3QF): mov    %rdx,-0x7b(%rdi)
L(P3QE): mov    %rdx,-0x73(%rdi)
L(P3QD): mov    %rdx,-0x6b(%rdi)
L(P3QC): mov    %rdx,-0x63(%rdi)
L(P3QB): mov    %rdx,-0x5b(%rdi)
L(P3QA): mov    %rdx,-0x53(%rdi)
L(P3Q9): mov    %rdx,-0x4b(%rdi)
L(P3Q8): mov    %rdx,-0x43(%rdi)
L(P3Q7): mov    %rdx,-0x3b(%rdi)
L(P3Q6): mov    %rdx,-0x33(%rdi)
L(P3Q5): mov    %rdx,-0x2b(%rdi)
L(P3Q4): mov    %rdx,-0x23(%rdi)
L(P3Q3): mov    %rdx,-0x1b(%rdi)
L(P3Q2): mov    %rdx,-0x13(%rdi)
L(P3Q1): mov    %rdx,-0xb(%rdi)
L(P3Q0): mov    %dx,-0x3(%rdi)
		mov    %dl,-0x1(%rdi)
		retq

	.balign     16
#ifdef USE_EXTRA_TABLE
L(P4QI): mov    %rdx,-0x94(%rdi)
#endif
L(P4QH): mov    %rdx,-0x8c(%rdi)
L(P4QG): mov    %rdx,-0x84(%rdi)
#		   .balign     16
L(P4QF): mov    %rdx,-0x7c(%rdi)
L(P4QE): mov    %rdx,-0x74(%rdi)
L(P4QD): mov    %rdx,-0x6c(%rdi)
L(P4QC): mov    %rdx,-0x64(%rdi)
L(P4QB): mov    %rdx,-0x5c(%rdi)
L(P4QA): mov    %rdx,-0x54(%rdi)
L(P4Q9): mov    %rdx,-0x4c(%rdi)
L(P4Q8): mov    %rdx,-0x44(%rdi)
L(P4Q7): mov    %rdx,-0x3c(%rdi)
L(P4Q6): mov    %rdx,-0x34(%rdi)
L(P4Q5): mov    %rdx,-0x2c(%rdi)
L(P4Q4): mov    %rdx,-0x24(%rdi)
L(P4Q3): mov    %rdx,-0x1c(%rdi)
L(P4Q2): mov    %rdx,-0x14(%rdi)
L(P4Q1): mov    %rdx,-0xc(%rdi)
L(P4Q0): mov    %edx,-0x4(%rdi)
		retq

	.balign     16
#ifdef USE_EXTRA_TABLE
L(P5QI): mov    %rdx,-0x95(%rdi)
#endif
L(P5QH): mov    %rdx,-0x8d(%rdi)
L(P5QG): mov    %rdx,-0x85(%rdi)
#		   .balign     16
L(P5QF): mov    %rdx,-0x7d(%rdi)
L(P5QE): mov    %rdx,-0x75(%rdi)
L(P5QD): mov    %rdx,-0x6d(%rdi)
L(P5QC): mov    %rdx,-0x65(%rdi)
L(P5QB): mov    %rdx,-0x5d(%rdi)
L(P5QA): mov    %rdx,-0x55(%rdi)
L(P5Q9): mov    %rdx,-0x4d(%rdi)
L(P5Q8): mov    %rdx,-0x45(%rdi)
L(P5Q7): mov    %rdx,-0x3d(%rdi)
L(P5Q6): mov    %rdx,-0x35(%rdi)
L(P5Q5): mov    %rdx,-0x2d(%rdi)
L(P5Q4): mov    %rdx,-0x25(%rdi)
L(P5Q3): mov    %rdx,-0x1d(%rdi)
L(P5Q2): mov    %rdx,-0x15(%rdi)
L(P5Q1): mov    %rdx,-0xd(%rdi)
L(P5Q0): mov    %edx,-0x5(%rdi)
		mov    %dl,-0x1(%rdi)
		retq

	.balign     16
#ifdef USE_EXTRA_TABLE
L(P6QI): mov    %rdx,-0x96(%rdi)
#endif
L(P6QH): mov    %rdx,-0x8e(%rdi)
L(P6QG): mov    %rdx,-0x86(%rdi)
#		   .balign     16
L(P6QF): mov    %rdx,-0x7e(%rdi)
L(P6QE): mov    %rdx,-0x76(%rdi)
L(P6QD): mov    %rdx,-0x6e(%rdi)
L(P6QC): mov    %rdx,-0x66(%rdi)
L(P6QB): mov    %rdx,-0x5e(%rdi)
L(P6QA): mov    %rdx,-0x56(%rdi)
L(P6Q9): mov    %rdx,-0x4e(%rdi)
L(P6Q8): mov    %rdx,-0x46(%rdi)
L(P6Q7): mov    %rdx,-0x3e(%rdi)
L(P6Q6): mov    %rdx,-0x36(%rdi)
L(P6Q5): mov    %rdx,-0x2e(%rdi)
L(P6Q4): mov    %rdx,-0x26(%rdi)
L(P6Q3): mov    %rdx,-0x1e(%rdi)
L(P6Q2): mov    %rdx,-0x16(%rdi)
L(P6Q1): mov    %rdx,-0xe(%rdi)
L(P6Q0): mov    %edx,-0x6(%rdi)
		mov    %dx,-0x2(%rdi)
		retq

	.balign     16
#ifdef USE_EXTRA_TABLE
L(P7QI): mov    %rdx,-0x97(%rdi)
#endif
L(P7QH): mov    %rdx,-0x8f(%rdi)
L(P7QG): mov    %rdx,-0x87(%rdi)
#		   .balign     16
L(P7QF): mov    %rdx,-0x7f(%rdi)
L(P7QE): mov    %rdx,-0x77(%rdi)
L(P7QD): mov    %rdx,-0x6f(%rdi)
L(P7QC): mov    %rdx,-0x67(%rdi)
L(P7QB): mov    %rdx,-0x5f(%rdi)
L(P7QA): mov    %rdx,-0x57(%rdi)
L(P7Q9): mov    %rdx,-0x4f(%rdi)
L(P7Q8): mov    %rdx,-0x47(%rdi)
L(P7Q7): mov    %rdx,-0x3f(%rdi)
L(P7Q6): mov    %rdx,-0x37(%rdi)
L(P7Q5): mov    %rdx,-0x2f(%rdi)
L(P7Q4): mov    %rdx,-0x27(%rdi)
L(P7Q3): mov    %rdx,-0x1f(%rdi)
L(P7Q2): mov    %rdx,-0x17(%rdi)
L(P7Q1): mov    %rdx,-0xf(%rdi)
L(P7Q0): mov    %edx,-0x7(%rdi)
		mov    %dx,-0x3(%rdi)
		mov    %dl,-0x1(%rdi)
		retq

	.balign     16
L(ck_mem_ops_method):

# align to 16 byte boundary first
	#test $0xf,%rdi
	#jz L(aligned_now)
	mov    $0x10,%r10
	mov    %rdi,%r9
	and    $0xf,%r9
	sub    %r9,%r10
	and    $0xf,%r10
	add    %r10,%rdi
	sub    %r10,%r8
#ifndef PIC
	lea    L(AliPxQx)(%rip),%r11
	jmpq   *(%r11,%r10,8)
#else
	lea    L(aligned_now)(%rip), %r11
	lea    L(AliPxQx)(%rip),%rcx
	movswq (%rcx,%r10,2),%rcx
	lea    (%rcx,%r11,1),%r11
	jmpq   *%r11
#endif

	.pushsection .rodata
	.balign     16
#ifndef PIC
L(AliPxQx):
	.quad       L(aligned_now), L(A1Q0), L(A2Q0), L(A3Q0)
	.quad	    L(A4Q0), L(A5Q0), L(A6Q0), L(A7Q0)
	.quad       L(A0Q1), L(A1Q1), L(A2Q1), L(A3Q1)
	.quad       L(A4Q1), L(A5Q1), L(A6Q1), L(A7Q1)
#else
L(AliPxQx):
	.short     L(aligned_now)-L(aligned_now)
	.short     L(A1Q0)-L(aligned_now)
	.short     L(A2Q0)-L(aligned_now)
	.short     L(A3Q0)-L(aligned_now)
	.short     L(A4Q0)-L(aligned_now)
	.short     L(A5Q0)-L(aligned_now)
	.short     L(A6Q0)-L(aligned_now)
	.short     L(A7Q0)-L(aligned_now)

	.short     L(A0Q1)-L(aligned_now)
	.short     L(A1Q1)-L(aligned_now)
	.short     L(A2Q1)-L(aligned_now)
	.short     L(A3Q1)-L(aligned_now)
	.short     L(A4Q1)-L(aligned_now)
	.short     L(A5Q1)-L(aligned_now)
	.short     L(A6Q1)-L(aligned_now)
	.short     L(A7Q1)-L(aligned_now)
#endif
	.popsection

	.balign     16
L(A5Q1):    mov    %dl,-0xd(%rdi)
L(A4Q1):    mov    %edx,-0xc(%rdi)
L(A0Q1):    mov    %rdx,-0x8(%rdi)
L(A0Q0):    jmp     L(aligned_now)

	.balign     16
L(A1Q1):   mov    %dl,-0x9(%rdi)
	mov    %rdx,-0x8(%rdi)
	jmp    L(aligned_now)

	.balign     16
L(A1Q0):   mov    %dl,-0x1(%rdi)
	jmp    L(aligned_now)

	.balign     16
L(A3Q1):    mov    %dl,-0xb(%rdi)
L(A2Q1):    mov    %dx,-0xa(%rdi)
	mov    %rdx,-0x8(%rdi)
	jmp    L(aligned_now)

	.balign     16
L(A3Q0):    mov    %dl,-0x3(%rdi)
L(A2Q0):    mov    %dx,-0x2(%rdi)
	jmp    L(aligned_now)

	.balign     16
L(A5Q0):    mov    %dl,-0x5(%rdi)
L(A4Q0):    mov    %edx,-0x4(%rdi)
	jmp    L(aligned_now)

	.balign     16
L(A7Q1):    mov    %dl,-0xf(%rdi)
L(A6Q1):    mov    %dx,-0xe(%rdi)
	mov    %edx,-0xc(%rdi)
	mov    %rdx,-0x8(%rdi)
	jmp    L(aligned_now)

	.balign     16
L(A7Q0):    mov    %dl,-0x7(%rdi)
L(A6Q0):    mov    %dx,-0x6(%rdi)
	mov    %edx,-0x4(%rdi)

#ifndef USE_MULTIARCH
	jmp    L(aligned_now)

L(SSE_pre):
#else
L(aligned_now):
#endif
#if !defined USE_MULTIARCH || defined USE_SSE2
	 # fill RegXMM0 with the pattern
	 movd   %rdx,%xmm0
	 punpcklqdq %xmm0,%xmm0

	 cmp    $0xb0,%r8 # 176
	 jae    L(byte32sse2_pre)

	 add    %r8,%rdi
# ifndef PIC
	 lea    L(SSExDx)(%rip),%r9
	 jmpq   *(%r9,%r8,8)
# else
	 lea    L(SSE0Q0)(%rip),%r9
	 lea    L(SSExDx)(%rip),%rcx
	 movswq (%rcx,%r8,2),%rcx
	 lea    (%rcx,%r9,1),%r9
	 jmpq   *%r9
# endif

L(SSE0QB):  movdqa %xmm0,-0xb0(%rdi)
L(SSE0QA):  movdqa %xmm0,-0xa0(%rdi)
L(SSE0Q9):  movdqa %xmm0,-0x90(%rdi)
L(SSE0Q8):  movdqa %xmm0,-0x80(%rdi)
L(SSE0Q7):  movdqa %xmm0,-0x70(%rdi)
L(SSE0Q6):  movdqa %xmm0,-0x60(%rdi)
L(SSE0Q5):  movdqa %xmm0,-0x50(%rdi)
L(SSE0Q4):  movdqa %xmm0,-0x40(%rdi)
L(SSE0Q3):  movdqa %xmm0,-0x30(%rdi)
L(SSE0Q2):  movdqa %xmm0,-0x20(%rdi)
L(SSE0Q1):  movdqa %xmm0,-0x10(%rdi)
L(SSE0Q0):  retq

L(SSE1QB):  movdqa %xmm0,-0xb1(%rdi)
L(SSE1QA):  movdqa %xmm0,-0xa1(%rdi)
L(SSE1Q9):  movdqa %xmm0,-0x91(%rdi)
L(SSE1Q8):  movdqa %xmm0,-0x81(%rdi)
L(SSE1Q7):  movdqa %xmm0,-0x71(%rdi)
L(SSE1Q6):  movdqa %xmm0,-0x61(%rdi)
L(SSE1Q5):  movdqa %xmm0,-0x51(%rdi)
L(SSE1Q4):  movdqa %xmm0,-0x41(%rdi)
L(SSE1Q3):  movdqa %xmm0,-0x31(%rdi)
L(SSE1Q2):  movdqa %xmm0,-0x21(%rdi)
L(SSE1Q1):  movdqa %xmm0,-0x11(%rdi)
L(SSE1Q0):  mov    %dl,-0x1(%rdi)
	retq

L(SSE2QB):  movdqa %xmm0,-0xb2(%rdi)
L(SSE2QA):  movdqa %xmm0,-0xa2(%rdi)
L(SSE2Q9):  movdqa %xmm0,-0x92(%rdi)
L(SSE2Q8):  movdqa %xmm0,-0x82(%rdi)
L(SSE2Q7):  movdqa %xmm0,-0x72(%rdi)
L(SSE2Q6):  movdqa %xmm0,-0x62(%rdi)
L(SSE2Q5):  movdqa %xmm0,-0x52(%rdi)
L(SSE2Q4):  movdqa %xmm0,-0x42(%rdi)
L(SSE2Q3):  movdqa %xmm0,-0x32(%rdi)
L(SSE2Q2):  movdqa %xmm0,-0x22(%rdi)
L(SSE2Q1):  movdqa %xmm0,-0x12(%rdi)
L(SSE2Q0):  mov    %dx,-0x2(%rdi)
	retq

L(SSE3QB):  movdqa %xmm0,-0xb3(%rdi)
L(SSE3QA):  movdqa %xmm0,-0xa3(%rdi)
L(SSE3Q9):  movdqa %xmm0,-0x93(%rdi)
L(SSE3Q8):  movdqa %xmm0,-0x83(%rdi)
L(SSE3Q7):  movdqa %xmm0,-0x73(%rdi)
L(SSE3Q6):  movdqa %xmm0,-0x63(%rdi)
L(SSE3Q5):  movdqa %xmm0,-0x53(%rdi)
L(SSE3Q4):  movdqa %xmm0,-0x43(%rdi)
L(SSE3Q3):  movdqa %xmm0,-0x33(%rdi)
L(SSE3Q2):  movdqa %xmm0,-0x23(%rdi)
L(SSE3Q1):  movdqa %xmm0,-0x13(%rdi)
L(SSE3Q0):  mov    %dx,-0x3(%rdi)
	mov    %dl,-0x1(%rdi)
	retq

L(SSE4QB):  movdqa %xmm0,-0xb4(%rdi)
L(SSE4QA):  movdqa %xmm0,-0xa4(%rdi)
L(SSE4Q9):  movdqa %xmm0,-0x94(%rdi)
L(SSE4Q8):  movdqa %xmm0,-0x84(%rdi)
L(SSE4Q7):  movdqa %xmm0,-0x74(%rdi)
L(SSE4Q6):  movdqa %xmm0,-0x64(%rdi)
L(SSE4Q5):  movdqa %xmm0,-0x54(%rdi)
L(SSE4Q4):  movdqa %xmm0,-0x44(%rdi)
L(SSE4Q3):  movdqa %xmm0,-0x34(%rdi)
L(SSE4Q2):  movdqa %xmm0,-0x24(%rdi)
L(SSE4Q1):  movdqa %xmm0,-0x14(%rdi)
L(SSE4Q0):  mov    %edx,-0x4(%rdi)
	retq

L(SSE5QB):  movdqa %xmm0,-0xb5(%rdi)
L(SSE5QA):  movdqa %xmm0,-0xa5(%rdi)
L(SSE5Q9):  movdqa %xmm0,-0x95(%rdi)
L(SSE5Q8):  movdqa %xmm0,-0x85(%rdi)
L(SSE5Q7):  movdqa %xmm0,-0x75(%rdi)
L(SSE5Q6):  movdqa %xmm0,-0x65(%rdi)
L(SSE5Q5):  movdqa %xmm0,-0x55(%rdi)
L(SSE5Q4):  movdqa %xmm0,-0x45(%rdi)
L(SSE5Q3):  movdqa %xmm0,-0x35(%rdi)
L(SSE5Q2):  movdqa %xmm0,-0x25(%rdi)
L(SSE5Q1):  movdqa %xmm0,-0x15(%rdi)
L(SSE5Q0):  mov    %edx,-0x5(%rdi)
	mov    %dl,-0x1(%rdi)
	retq


L(SSE6QB):  movdqa %xmm0,-0xb6(%rdi)
L(SSE6QA):  movdqa %xmm0,-0xa6(%rdi)
L(SSE6Q9):  movdqa %xmm0,-0x96(%rdi)
L(SSE6Q8):  movdqa %xmm0,-0x86(%rdi)
L(SSE6Q7):  movdqa %xmm0,-0x76(%rdi)
L(SSE6Q6):  movdqa %xmm0,-0x66(%rdi)
L(SSE6Q5):  movdqa %xmm0,-0x56(%rdi)
L(SSE6Q4):  movdqa %xmm0,-0x46(%rdi)
L(SSE6Q3):  movdqa %xmm0,-0x36(%rdi)
L(SSE6Q2):  movdqa %xmm0,-0x26(%rdi)
L(SSE6Q1):  movdqa %xmm0,-0x16(%rdi)
L(SSE6Q0):  mov    %edx,-0x6(%rdi)
	mov    %dx,-0x2(%rdi)
	retq

L(SSE7QB):  movdqa %xmm0,-0xb7(%rdi)
L(SSE7QA):  movdqa %xmm0,-0xa7(%rdi)
L(SSE7Q9):  movdqa %xmm0,-0x97(%rdi)
L(SSE7Q8):  movdqa %xmm0,-0x87(%rdi)
L(SSE7Q7):  movdqa %xmm0,-0x77(%rdi)
L(SSE7Q6):  movdqa %xmm0,-0x67(%rdi)
L(SSE7Q5):  movdqa %xmm0,-0x57(%rdi)
L(SSE7Q4):  movdqa %xmm0,-0x47(%rdi)
L(SSE7Q3):  movdqa %xmm0,-0x37(%rdi)
L(SSE7Q2):  movdqa %xmm0,-0x27(%rdi)
L(SSE7Q1):  movdqa %xmm0,-0x17(%rdi)
L(SSE7Q0):  mov    %edx,-0x7(%rdi)
	mov    %dx,-0x3(%rdi)
	mov    %dl,-0x1(%rdi)
	retq

L(SSE8QB):  movdqa %xmm0,-0xb8(%rdi)
L(SSE8QA):  movdqa %xmm0,-0xa8(%rdi)
L(SSE8Q9):  movdqa %xmm0,-0x98(%rdi)
L(SSE8Q8):  movdqa %xmm0,-0x88(%rdi)
L(SSE8Q7):  movdqa %xmm0,-0x78(%rdi)
L(SSE8Q6):  movdqa %xmm0,-0x68(%rdi)
L(SSE8Q5):  movdqa %xmm0,-0x58(%rdi)
L(SSE8Q4):  movdqa %xmm0,-0x48(%rdi)
L(SSE8Q3):  movdqa %xmm0,-0x38(%rdi)
L(SSE8Q2):  movdqa %xmm0,-0x28(%rdi)
L(SSE8Q1):  movdqa %xmm0,-0x18(%rdi)
L(SSE8Q0):  mov    %rdx,-0x8(%rdi)
	retq

L(SSE9QB):  movdqa %xmm0,-0xb9(%rdi)
L(SSE9QA):  movdqa %xmm0,-0xa9(%rdi)
L(SSE9Q9):  movdqa %xmm0,-0x99(%rdi)
L(SSE9Q8):  movdqa %xmm0,-0x89(%rdi)
L(SSE9Q7):  movdqa %xmm0,-0x79(%rdi)
L(SSE9Q6):  movdqa %xmm0,-0x69(%rdi)
L(SSE9Q5):  movdqa %xmm0,-0x59(%rdi)
L(SSE9Q4):  movdqa %xmm0,-0x49(%rdi)
L(SSE9Q3):  movdqa %xmm0,-0x39(%rdi)
L(SSE9Q2):  movdqa %xmm0,-0x29(%rdi)
L(SSE9Q1):  movdqa %xmm0,-0x19(%rdi)
L(SSE9Q0):  mov    %rdx,-0x9(%rdi)
	mov    %dl,-0x1(%rdi)
	retq

L(SSE10QB): movdqa %xmm0,-0xba(%rdi)
L(SSE10QA): movdqa %xmm0,-0xaa(%rdi)
L(SSE10Q9): movdqa %xmm0,-0x9a(%rdi)
L(SSE10Q8): movdqa %xmm0,-0x8a(%rdi)
L(SSE10Q7): movdqa %xmm0,-0x7a(%rdi)
L(SSE10Q6): movdqa %xmm0,-0x6a(%rdi)
L(SSE10Q5): movdqa %xmm0,-0x5a(%rdi)
L(SSE10Q4): movdqa %xmm0,-0x4a(%rdi)
L(SSE10Q3): movdqa %xmm0,-0x3a(%rdi)
L(SSE10Q2): movdqa %xmm0,-0x2a(%rdi)
L(SSE10Q1): movdqa %xmm0,-0x1a(%rdi)
L(SSE10Q0): mov    %rdx,-0xa(%rdi)
	mov    %dx,-0x2(%rdi)
	retq

L(SSE11QB): movdqa %xmm0,-0xbb(%rdi)
L(SSE11QA): movdqa %xmm0,-0xab(%rdi)
L(SSE11Q9): movdqa %xmm0,-0x9b(%rdi)
L(SSE11Q8): movdqa %xmm0,-0x8b(%rdi)
L(SSE11Q7): movdqa %xmm0,-0x7b(%rdi)
L(SSE11Q6): movdqa %xmm0,-0x6b(%rdi)
L(SSE11Q5): movdqa %xmm0,-0x5b(%rdi)
L(SSE11Q4): movdqa %xmm0,-0x4b(%rdi)
L(SSE11Q3): movdqa %xmm0,-0x3b(%rdi)
L(SSE11Q2): movdqa %xmm0,-0x2b(%rdi)
L(SSE11Q1): movdqa %xmm0,-0x1b(%rdi)
L(SSE11Q0): mov    %rdx,-0xb(%rdi)
	mov    %dx,-0x3(%rdi)
	mov    %dl,-0x1(%rdi)
	retq

L(SSE12QB): movdqa %xmm0,-0xbc(%rdi)
L(SSE12QA): movdqa %xmm0,-0xac(%rdi)
L(SSE12Q9): movdqa %xmm0,-0x9c(%rdi)
L(SSE12Q8): movdqa %xmm0,-0x8c(%rdi)
L(SSE12Q7): movdqa %xmm0,-0x7c(%rdi)
L(SSE12Q6): movdqa %xmm0,-0x6c(%rdi)
L(SSE12Q5): movdqa %xmm0,-0x5c(%rdi)
L(SSE12Q4): movdqa %xmm0,-0x4c(%rdi)
L(SSE12Q3): movdqa %xmm0,-0x3c(%rdi)
L(SSE12Q2): movdqa %xmm0,-0x2c(%rdi)
L(SSE12Q1): movdqa %xmm0,-0x1c(%rdi)
L(SSE12Q0): mov    %rdx,-0xc(%rdi)
	mov    %edx,-0x4(%rdi)
	retq

L(SSE13QB): movdqa %xmm0,-0xbd(%rdi)
L(SSE13QA): movdqa %xmm0,-0xad(%rdi)
L(SSE13Q9): movdqa %xmm0,-0x9d(%rdi)
L(SSE13Q8): movdqa %xmm0,-0x8d(%rdi)
L(SSE13Q7): movdqa %xmm0,-0x7d(%rdi)
L(SSE13Q6): movdqa %xmm0,-0x6d(%rdi)
L(SSE13Q5): movdqa %xmm0,-0x5d(%rdi)
L(SSE13Q4): movdqa %xmm0,-0x4d(%rdi)
L(SSE13Q3): movdqa %xmm0,-0x3d(%rdi)
L(SSE13Q2): movdqa %xmm0,-0x2d(%rdi)
L(SSE13Q1): movdqa %xmm0,-0x1d(%rdi)
L(SSE13Q0): mov    %rdx,-0xd(%rdi)
	mov    %edx,-0x5(%rdi)
	mov    %dl,-0x1(%rdi)
	retq

L(SSE14QB): movdqa %xmm0,-0xbe(%rdi)
L(SSE14QA): movdqa %xmm0,-0xae(%rdi)
L(SSE14Q9): movdqa %xmm0,-0x9e(%rdi)
L(SSE14Q8): movdqa %xmm0,-0x8e(%rdi)
L(SSE14Q7): movdqa %xmm0,-0x7e(%rdi)
L(SSE14Q6): movdqa %xmm0,-0x6e(%rdi)
L(SSE14Q5): movdqa %xmm0,-0x5e(%rdi)
L(SSE14Q4): movdqa %xmm0,-0x4e(%rdi)
L(SSE14Q3): movdqa %xmm0,-0x3e(%rdi)
L(SSE14Q2): movdqa %xmm0,-0x2e(%rdi)
L(SSE14Q1): movdqa %xmm0,-0x1e(%rdi)
L(SSE14Q0): mov    %rdx,-0xe(%rdi)
	mov    %edx,-0x6(%rdi)
	mov    %dx,-0x2(%rdi)
	retq

L(SSE15QB): movdqa %xmm0,-0xbf(%rdi)
L(SSE15QA): movdqa %xmm0,-0xaf(%rdi)
L(SSE15Q9): movdqa %xmm0,-0x9f(%rdi)
L(SSE15Q8): movdqa %xmm0,-0x8f(%rdi)
L(SSE15Q7): movdqa %xmm0,-0x7f(%rdi)
L(SSE15Q6): movdqa %xmm0,-0x6f(%rdi)
L(SSE15Q5): movdqa %xmm0,-0x5f(%rdi)
L(SSE15Q4): movdqa %xmm0,-0x4f(%rdi)
L(SSE15Q3): movdqa %xmm0,-0x3f(%rdi)
L(SSE15Q2): movdqa %xmm0,-0x2f(%rdi)
L(SSE15Q1): movdqa %xmm0,-0x1f(%rdi)
L(SSE15Q0): mov    %rdx,-0xf(%rdi)
	mov    %edx,-0x7(%rdi)
	mov    %dx,-0x3(%rdi)
	mov    %dl,-0x1(%rdi)
	retq

	.balign     16
L(byte32sse2_pre):

	mov    __x86_64_shared_cache_size(%rip),%r9d  # The largest cache size
	cmp    %r9,%r8
	ja     L(sse2_nt_move_pre)
	#jmp    L(byte32sse2)
	.balign     16
L(byte32sse2):
	lea    -0x80(%r8),%r8 # 128
	cmp    $0x80,%r8   # 128
	movdqa %xmm0,(%rdi)
	movdqa %xmm0,0x10(%rdi)
	movdqa %xmm0,0x20(%rdi)
	movdqa %xmm0,0x30(%rdi)
	movdqa %xmm0,0x40(%rdi)
	movdqa %xmm0,0x50(%rdi)
	movdqa %xmm0,0x60(%rdi)
	movdqa %xmm0,0x70(%rdi)

	lea    0x80(%rdi),%rdi
	jae    L(byte32sse2)
	add    %r8,%rdi
# ifndef PIC
	lea    L(SSExDx)(%rip),%r11
	jmpq   *(%r11,%r8,8)
# else
	lea    L(SSE0Q0)(%rip),%r11
	lea    L(SSExDx)(%rip),%rcx
	movswq (%rcx,%r8,2),%rcx
	lea    (%rcx,%r11,1),%r11
	jmpq   *%r11
# endif

	.balign     16
L(sse2_nt_move_pre):
	cmp    $0x0,%r9
	je     L(byte32sse2)
	jmp    L(sse2_nt_move)

	.balign     16
L(sse2_nt_move):
	lea    -0x80(%r8),%r8
	cmp    $0x80,%r8

	movntdq %xmm0,(%rdi)
	movntdq %xmm0,0x10(%rdi)
	movntdq %xmm0,0x20(%rdi)
	movntdq %xmm0,0x30(%rdi)
	movntdq %xmm0,0x40(%rdi)
	movntdq %xmm0,0x50(%rdi)
	movntdq %xmm0,0x60(%rdi)
	movntdq %xmm0,0x70(%rdi)

	lea    0x80(%rdi),%rdi
	jae    L(sse2_nt_move)
	sfence
	add    %r8,%rdi
# ifndef PIC
	lea    L(SSExDx)(%rip),%r11
	jmpq   *(%r11,%r8,8)
# else
	lea    L(SSE0Q0)(%rip),%r11
	lea    L(SSExDx)(%rip),%rcx
	movswq (%rcx,%r8,2),%rcx
	lea   (%rcx,%r11,1),%r11
	jmpq   *%r11
# endif

	.pushsection .rodata
	.balign     16
# ifndef PIC
L(SSExDx):
	.quad       L(SSE0Q0), L(SSE1Q0), L(SSE2Q0), L(SSE3Q0)
	.quad       L(SSE4Q0), L(SSE5Q0), L(SSE6Q0), L(SSE7Q0)
	.quad       L(SSE8Q0), L(SSE9Q0), L(SSE10Q0), L(SSE11Q0)
	.quad       L(SSE12Q0), L(SSE13Q0), L(SSE14Q0), L(SSE15Q0)
	.quad       L(SSE0Q1), L(SSE1Q1), L(SSE2Q1), L(SSE3Q1)
	.quad       L(SSE4Q1), L(SSE5Q1), L(SSE6Q1), L(SSE7Q1)
	.quad       L(SSE8Q1), L(SSE9Q1), L(SSE10Q1), L(SSE11Q1)
	.quad       L(SSE12Q1), L(SSE13Q1), L(SSE14Q1), L(SSE15Q1)
	.quad       L(SSE0Q2), L(SSE1Q2), L(SSE2Q2), L(SSE3Q2)
	.quad       L(SSE4Q2), L(SSE5Q2), L(SSE6Q2), L(SSE7Q2)
	.quad       L(SSE8Q2), L(SSE9Q2), L(SSE10Q2), L(SSE11Q2)
	.quad       L(SSE12Q2), L(SSE13Q2), L(SSE14Q2), L(SSE15Q2)
	.quad       L(SSE0Q3), L(SSE1Q3), L(SSE2Q3), L(SSE3Q3)
	.quad       L(SSE4Q3), L(SSE5Q3), L(SSE6Q3), L(SSE7Q3)
	.quad       L(SSE8Q3), L(SSE9Q3), L(SSE10Q3), L(SSE11Q3)
	.quad       L(SSE12Q3), L(SSE13Q3), L(SSE14Q3), L(SSE15Q3)
	.quad       L(SSE0Q4), L(SSE1Q4), L(SSE2Q4), L(SSE3Q4)
	.quad       L(SSE4Q4), L(SSE5Q4), L(SSE6Q4), L(SSE7Q4)
	.quad       L(SSE8Q4), L(SSE9Q4), L(SSE10Q4), L(SSE11Q4)
	.quad       L(SSE12Q4), L(SSE13Q4), L(SSE14Q4), L(SSE15Q4)
	.quad       L(SSE0Q5), L(SSE1Q5), L(SSE2Q5), L(SSE3Q5)
	.quad       L(SSE4Q5), L(SSE5Q5), L(SSE6Q5), L(SSE7Q5)
	.quad       L(SSE8Q5), L(SSE9Q5), L(SSE10Q5), L(SSE11Q5)
	.quad       L(SSE12Q5), L(SSE13Q5), L(SSE14Q5), L(SSE15Q5)
	.quad       L(SSE0Q6), L(SSE1Q6), L(SSE2Q6), L(SSE3Q6)
	.quad       L(SSE4Q6), L(SSE5Q6), L(SSE6Q6), L(SSE7Q6)
	.quad       L(SSE8Q6), L(SSE9Q6), L(SSE10Q6), L(SSE11Q6)
	.quad       L(SSE12Q6), L(SSE13Q6), L(SSE14Q6), L(SSE15Q6)
	.quad       L(SSE0Q7), L(SSE1Q7), L(SSE2Q7), L(SSE3Q7)
	.quad       L(SSE4Q7), L(SSE5Q7), L(SSE6Q7), L(SSE7Q7)
	.quad       L(SSE8Q7), L(SSE9Q7), L(SSE10Q7), L(SSE11Q7)
	.quad       L(SSE12Q7), L(SSE13Q7), L(SSE14Q7), L(SSE15Q7)
	.quad       L(SSE0Q8), L(SSE1Q8), L(SSE2Q8), L(SSE3Q8)
	.quad       L(SSE4Q8), L(SSE5Q8), L(SSE6Q8), L(SSE7Q8)
	.quad       L(SSE8Q8), L(SSE9Q8), L(SSE10Q8), L(SSE11Q8)
	.quad       L(SSE12Q8), L(SSE13Q8), L(SSE14Q8), L(SSE15Q8)
	.quad       L(SSE0Q9), L(SSE1Q9), L(SSE2Q9), L(SSE3Q9)
	.quad       L(SSE4Q9), L(SSE5Q9), L(SSE6Q9), L(SSE7Q9)
	.quad       L(SSE8Q9), L(SSE9Q9), L(SSE10Q9), L(SSE11Q9)
	.quad       L(SSE12Q9), L(SSE13Q9), L(SSE14Q9), L(SSE15Q9)
	.quad       L(SSE0QA), L(SSE1QA), L(SSE2QA), L(SSE3QA)
	.quad       L(SSE4QA), L(SSE5QA), L(SSE6QA), L(SSE7QA)
	.quad       L(SSE8QA), L(SSE9QA), L(SSE10QA), L(SSE11QA)
	.quad       L(SSE12QA), L(SSE13QA), L(SSE14QA), L(SSE15QA)
	.quad       L(SSE0QB), L(SSE1QB), L(SSE2QB), L(SSE3QB)
	.quad       L(SSE4QB), L(SSE5QB), L(SSE6QB), L(SSE7QB)
	.quad       L(SSE8QB), L(SSE9QB), L(SSE10QB), L(SSE11QB)
	.quad       L(SSE12QB), L(SSE13QB), L(SSE14QB), L(SSE15QB)
# else
L(SSExDx):
	.short     L(SSE0Q0) -L(SSE0Q0)
	.short     L(SSE1Q0) -L(SSE0Q0)
	.short     L(SSE2Q0) -L(SSE0Q0)
	.short     L(SSE3Q0) -L(SSE0Q0)
	.short     L(SSE4Q0) -L(SSE0Q0)
	.short     L(SSE5Q0) -L(SSE0Q0)
	.short     L(SSE6Q0) -L(SSE0Q0)
	.short     L(SSE7Q0) -L(SSE0Q0)

	.short     L(SSE8Q0) -L(SSE0Q0)
	.short     L(SSE9Q0) -L(SSE0Q0)
	.short     L(SSE10Q0)-L(SSE0Q0)
	.short     L(SSE11Q0)-L(SSE0Q0)
	.short     L(SSE12Q0)-L(SSE0Q0)
	.short     L(SSE13Q0)-L(SSE0Q0)
	.short     L(SSE14Q0)-L(SSE0Q0)
	.short     L(SSE15Q0)-L(SSE0Q0)

	.short     L(SSE0Q1) -L(SSE0Q0)
	.short     L(SSE1Q1) -L(SSE0Q0)
	.short     L(SSE2Q1) -L(SSE0Q0)
	.short     L(SSE3Q1) -L(SSE0Q0)
	.short     L(SSE4Q1) -L(SSE0Q0)
	.short     L(SSE5Q1) -L(SSE0Q0)
	.short     L(SSE6Q1) -L(SSE0Q0)
	.short     L(SSE7Q1) -L(SSE0Q0)

	.short     L(SSE8Q1) -L(SSE0Q0)
	.short     L(SSE9Q1) -L(SSE0Q0)
	.short     L(SSE10Q1)-L(SSE0Q0)
	.short     L(SSE11Q1)-L(SSE0Q0)
	.short     L(SSE12Q1)-L(SSE0Q0)
	.short     L(SSE13Q1)-L(SSE0Q0)
	.short     L(SSE14Q1)-L(SSE0Q0)
	.short     L(SSE15Q1)-L(SSE0Q0)

	.short     L(SSE0Q2) -L(SSE0Q0)
	.short     L(SSE1Q2) -L(SSE0Q0)
	.short     L(SSE2Q2) -L(SSE0Q0)
	.short     L(SSE3Q2) -L(SSE0Q0)
	.short     L(SSE4Q2) -L(SSE0Q0)
	.short     L(SSE5Q2) -L(SSE0Q0)
	.short     L(SSE6Q2) -L(SSE0Q0)
	.short     L(SSE7Q2) -L(SSE0Q0)

	.short     L(SSE8Q2) -L(SSE0Q0)
	.short     L(SSE9Q2) -L(SSE0Q0)
	.short     L(SSE10Q2)-L(SSE0Q0)
	.short     L(SSE11Q2)-L(SSE0Q0)
	.short     L(SSE12Q2)-L(SSE0Q0)
	.short     L(SSE13Q2)-L(SSE0Q0)
	.short     L(SSE14Q2)-L(SSE0Q0)
	.short     L(SSE15Q2)-L(SSE0Q0)

	.short     L(SSE0Q3) -L(SSE0Q0)
	.short     L(SSE1Q3) -L(SSE0Q0)
	.short     L(SSE2Q3) -L(SSE0Q0)
	.short     L(SSE3Q3) -L(SSE0Q0)
	.short     L(SSE4Q3) -L(SSE0Q0)
	.short     L(SSE5Q3) -L(SSE0Q0)
	.short     L(SSE6Q3) -L(SSE0Q0)
	.short     L(SSE7Q3) -L(SSE0Q0)

	.short     L(SSE8Q3) -L(SSE0Q0)
	.short     L(SSE9Q3) -L(SSE0Q0)
	.short     L(SSE10Q3)-L(SSE0Q0)
	.short     L(SSE11Q3)-L(SSE0Q0)
	.short     L(SSE12Q3)-L(SSE0Q0)
	.short     L(SSE13Q3)-L(SSE0Q0)
	.short     L(SSE14Q3)-L(SSE0Q0)
	.short     L(SSE15Q3)-L(SSE0Q0)

	.short     L(SSE0Q4) -L(SSE0Q0)
	.short     L(SSE1Q4) -L(SSE0Q0)
	.short     L(SSE2Q4) -L(SSE0Q0)
	.short     L(SSE3Q4) -L(SSE0Q0)
	.short     L(SSE4Q4) -L(SSE0Q0)
	.short     L(SSE5Q4) -L(SSE0Q0)
	.short     L(SSE6Q4) -L(SSE0Q0)
	.short     L(SSE7Q4) -L(SSE0Q0)

	.short     L(SSE8Q4) -L(SSE0Q0)
	.short     L(SSE9Q4) -L(SSE0Q0)
	.short     L(SSE10Q4)-L(SSE0Q0)
	.short     L(SSE11Q4)-L(SSE0Q0)
	.short     L(SSE12Q4)-L(SSE0Q0)
	.short     L(SSE13Q4)-L(SSE0Q0)
	.short     L(SSE14Q4)-L(SSE0Q0)
	.short     L(SSE15Q4)-L(SSE0Q0)

	.short     L(SSE0Q5) -L(SSE0Q0)
	.short     L(SSE1Q5) -L(SSE0Q0)
	.short     L(SSE2Q5) -L(SSE0Q0)
	.short     L(SSE3Q5) -L(SSE0Q0)
	.short     L(SSE4Q5) -L(SSE0Q0)
	.short     L(SSE5Q5) -L(SSE0Q0)
	.short     L(SSE6Q5) -L(SSE0Q0)
	.short     L(SSE7Q5) -L(SSE0Q0)

	.short     L(SSE8Q5) -L(SSE0Q0)
	.short     L(SSE9Q5) -L(SSE0Q0)
	.short     L(SSE10Q5)-L(SSE0Q0)
	.short     L(SSE11Q5)-L(SSE0Q0)
	.short     L(SSE12Q5)-L(SSE0Q0)
	.short     L(SSE13Q5)-L(SSE0Q0)
	.short     L(SSE14Q5)-L(SSE0Q0)
	.short     L(SSE15Q5)-L(SSE0Q0)

	.short     L(SSE0Q6) -L(SSE0Q0)
	.short     L(SSE1Q6) -L(SSE0Q0)
	.short     L(SSE2Q6) -L(SSE0Q0)
	.short     L(SSE3Q6) -L(SSE0Q0)
	.short     L(SSE4Q6) -L(SSE0Q0)
	.short     L(SSE5Q6) -L(SSE0Q0)
	.short     L(SSE6Q6) -L(SSE0Q0)
	.short     L(SSE7Q6) -L(SSE0Q0)

	.short     L(SSE8Q6) -L(SSE0Q0)
	.short     L(SSE9Q6) -L(SSE0Q0)
	.short     L(SSE10Q6)-L(SSE0Q0)
	.short     L(SSE11Q6)-L(SSE0Q0)
	.short     L(SSE12Q6)-L(SSE0Q0)
	.short     L(SSE13Q6)-L(SSE0Q0)
	.short     L(SSE14Q6)-L(SSE0Q0)
	.short     L(SSE15Q6)-L(SSE0Q0)

	.short     L(SSE0Q7) -L(SSE0Q0)
	.short     L(SSE1Q7) -L(SSE0Q0)
	.short     L(SSE2Q7) -L(SSE0Q0)
	.short     L(SSE3Q7) -L(SSE0Q0)
	.short     L(SSE4Q7) -L(SSE0Q0)
	.short     L(SSE5Q7) -L(SSE0Q0)
	.short     L(SSE6Q7) -L(SSE0Q0)
	.short     L(SSE7Q7) -L(SSE0Q0)

	.short     L(SSE8Q7) -L(SSE0Q0)
	.short     L(SSE9Q7) -L(SSE0Q0)
	.short     L(SSE10Q7)-L(SSE0Q0)
	.short     L(SSE11Q7)-L(SSE0Q0)
	.short     L(SSE12Q7)-L(SSE0Q0)
	.short     L(SSE13Q7)-L(SSE0Q0)
	.short     L(SSE14Q7)-L(SSE0Q0)
	.short     L(SSE15Q7)-L(SSE0Q0)

	.short     L(SSE0Q8) -L(SSE0Q0)
	.short     L(SSE1Q8) -L(SSE0Q0)
	.short     L(SSE2Q8) -L(SSE0Q0)
	.short     L(SSE3Q8) -L(SSE0Q0)
	.short     L(SSE4Q8) -L(SSE0Q0)
	.short     L(SSE5Q8) -L(SSE0Q0)
	.short     L(SSE6Q8) -L(SSE0Q0)
	.short     L(SSE7Q8) -L(SSE0Q0)

	.short     L(SSE8Q8) -L(SSE0Q0)
	.short     L(SSE9Q8) -L(SSE0Q0)
	.short     L(SSE10Q8)-L(SSE0Q0)
	.short     L(SSE11Q8)-L(SSE0Q0)
	.short     L(SSE12Q8)-L(SSE0Q0)
	.short     L(SSE13Q8)-L(SSE0Q0)
	.short     L(SSE14Q8)-L(SSE0Q0)
	.short     L(SSE15Q8)-L(SSE0Q0)

	.short     L(SSE0Q9) -L(SSE0Q0)
	.short     L(SSE1Q9) -L(SSE0Q0)
	.short     L(SSE2Q9) -L(SSE0Q0)
	.short     L(SSE3Q9) -L(SSE0Q0)
	.short     L(SSE4Q9) -L(SSE0Q0)
	.short     L(SSE5Q9) -L(SSE0Q0)
	.short     L(SSE6Q9) -L(SSE0Q0)
	.short     L(SSE7Q9) -L(SSE0Q0)

	.short     L(SSE8Q9) -L(SSE0Q0)
	.short     L(SSE9Q9) -L(SSE0Q0)
	.short     L(SSE10Q9)-L(SSE0Q0)
	.short     L(SSE11Q9)-L(SSE0Q0)
	.short     L(SSE12Q9)-L(SSE0Q0)
	.short     L(SSE13Q9)-L(SSE0Q0)
	.short     L(SSE14Q9)-L(SSE0Q0)
	.short     L(SSE15Q9)-L(SSE0Q0)

	.short     L(SSE0QA) -L(SSE0Q0)
	.short     L(SSE1QA) -L(SSE0Q0)
	.short     L(SSE2QA) -L(SSE0Q0)
	.short     L(SSE3QA) -L(SSE0Q0)
	.short     L(SSE4QA) -L(SSE0Q0)
	.short     L(SSE5QA) -L(SSE0Q0)
	.short     L(SSE6QA) -L(SSE0Q0)
	.short     L(SSE7QA) -L(SSE0Q0)

	.short     L(SSE8QA) -L(SSE0Q0)
	.short     L(SSE9QA) -L(SSE0Q0)
	.short     L(SSE10QA)-L(SSE0Q0)
	.short     L(SSE11QA)-L(SSE0Q0)
	.short     L(SSE12QA)-L(SSE0Q0)
	.short     L(SSE13QA)-L(SSE0Q0)
	.short     L(SSE14QA)-L(SSE0Q0)
	.short     L(SSE15QA)-L(SSE0Q0)

	.short     L(SSE0QB) -L(SSE0Q0)
	.short     L(SSE1QB) -L(SSE0Q0)
	.short     L(SSE2QB) -L(SSE0Q0)
	.short     L(SSE3QB) -L(SSE0Q0)
	.short     L(SSE4QB) -L(SSE0Q0)
	.short     L(SSE5QB) -L(SSE0Q0)
	.short     L(SSE6QB) -L(SSE0Q0)
	.short     L(SSE7QB) -L(SSE0Q0)

	.short     L(SSE8QB) -L(SSE0Q0)
	.short     L(SSE9QB) -L(SSE0Q0)
	.short     L(SSE10QB)-L(SSE0Q0)
	.short     L(SSE11QB)-L(SSE0Q0)
	.short     L(SSE12QB)-L(SSE0Q0)
	.short     L(SSE13QB)-L(SSE0Q0)
	.short     L(SSE14QB)-L(SSE0Q0)
	.short     L(SSE15QB)-L(SSE0Q0)
# endif
	.popsection
#endif /* !defined USE_MULTIARCH || defined USE_SSE2  */

	.balign     16
#ifndef USE_MULTIARCH
L(aligned_now):

	 cmpl   $0x1,__x86_64_preferred_memory_instruction(%rip)
	 jg     L(SSE_pre)
#endif /* USE_MULTIARCH */

L(8byte_move_try):
	cmpq	__STOS_LOWER_BOUNDARY,%r8
	jae	L(8byte_stos_try)

	.balign     16
L(8byte_move):
	movq	%r8,%rcx
	shrq	$7,%rcx
	jz	L(8byte_move_skip)

	.p2align 4

L(8byte_move_loop):
	decq	%rcx

	movq	%rdx,    (%rdi)
	movq	%rdx,  8 (%rdi)
	movq	%rdx, 16 (%rdi)
	movq	%rdx, 24 (%rdi)
	movq	%rdx, 32 (%rdi)
	movq	%rdx, 40 (%rdi)
	movq	%rdx, 48 (%rdi)
	movq	%rdx, 56 (%rdi)
	movq	%rdx, 64 (%rdi)
	movq	%rdx, 72 (%rdi)
	movq	%rdx, 80 (%rdi)
	movq	%rdx, 88 (%rdi)
	movq	%rdx, 96 (%rdi)
	movq	%rdx, 104 (%rdi)
	movq	%rdx, 112 (%rdi)
	movq	%rdx, 120 (%rdi)

	leaq	128 (%rdi),%rdi

	jnz     L(8byte_move_loop)

L(8byte_move_skip):
	andl	$127,%r8d
	lea	(%rdi,%r8,1),%rdi

#ifndef PIC
	lea	L(setPxQx)(%rip),%r11
	jmpq	*(%r11,%r8,8) # old scheme remained for nonPIC
#else
	lea	L(Got0)(%rip),%r11
	lea	L(setPxQx)(%rip),%rcx
	movswq	(%rcx,%r8,2),%rcx
	lea	(%rcx,%r11,1),%r11
	jmpq	*%r11
#endif

	.balign     16
L(8byte_stos_try):
	mov    __x86_64_shared_cache_size(%rip),%r9d // ck largest cache size
	cmpq	%r8,%r9		// calculate the lesser of remaining
	cmovaq	%r8,%r9		// bytes and largest cache size
	jbe	L(8byte_stos)

L(8byte_move_reuse_try):
	cmp	__STOS_UPPER_BOUNDARY,%r8
	jae	L(8byte_move)

	.balign     16
L(8byte_stos):
	movq	%r9,%rcx
	andq	$-8,%r9

	shrq	$3,%rcx
	jz	L(8byte_stos_skip)

	xchgq	%rax,%rdx

	rep
	stosq

	xchgq	%rax,%rdx

L(8byte_stos_skip):
	subq	%r9,%r8
	ja	L(8byte_nt_move)

	andl	$7,%r8d
	lea	(%rdi,%r8,1),%rdi
#ifndef PIC
	lea	L(setPxQx)(%rip),%r11
	jmpq	*(%r11,%r8,8) # old scheme remained for nonPIC
#else
	lea	L(Got0)(%rip),%r11
	lea     L(setPxQx)(%rip),%rcx
	movswq	(%rcx,%r8,2),%rcx
	lea	(%rcx,%r11,1),%r11
	jmpq	*%r11
#endif

	.balign     16
L(8byte_nt_move):
	movq	%r8,%rcx
	shrq	$7,%rcx
	jz      L(8byte_nt_move_skip)

	.balign     16
L(8byte_nt_move_loop):
	decq	%rcx

	movntiq	%rdx,     (%rdi)
	movntiq	%rdx,   8 (%rdi)
	movntiq	%rdx,  16 (%rdi)
	movntiq	%rdx,  24 (%rdi)
	movntiq	%rdx,  32 (%rdi)
	movntiq	%rdx,  40 (%rdi)
	movntiq	%rdx,  48 (%rdi)
	movntiq	%rdx,  56 (%rdi)
	movntiq	%rdx,  64 (%rdi)
	movntiq	%rdx,  72 (%rdi)
	movntiq	%rdx,  80 (%rdi)
	movntiq	%rdx,  88 (%rdi)
	movntiq	%rdx,  96 (%rdi)
	movntiq	%rdx, 104 (%rdi)
	movntiq	%rdx, 112 (%rdi)
	movntiq	%rdx, 120 (%rdi)

	leaq	128 (%rdi),%rdi

	jnz     L(8byte_nt_move_loop)

	sfence

L(8byte_nt_move_skip):
	andl	$127,%r8d

	lea	(%rdi,%r8,1),%rdi
#ifndef PIC
	lea	L(setPxQx)(%rip),%r11
	jmpq	*(%r11,%r8,8) # old scheme remained for nonPIC
#else
	lea	L(Got0)(%rip),%r11
	lea     L(setPxQx)(%rip),%rcx
	movswq	(%rcx,%r8,2),%rcx
	lea	(%rcx,%r11,1),%r11
	jmpq	*%r11
#endif

END (memset)
libc_hidden_builtin_def (memset)

#if defined PIC && !defined NOT_IN_libc && !defined USE_MULTIARCH
strong_alias (__memset_chk, __memset_zero_constant_len_parameter)
	.section .gnu.warning.__memset_zero_constant_len_parameter
	.string "memset used with constant zero length parameter; this could be due to transposed parameters"
#endif
