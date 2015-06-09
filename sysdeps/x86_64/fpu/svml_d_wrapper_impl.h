/* Wrapper implementations of vector math functions.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.
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

/* SSE2 ISA version as wrapper to scalar.  */
.macro WRAPPER_IMPL_SSE2 callee
        subq      $40, %rsp
        cfi_adjust_cfa_offset(40)
        movaps    %xmm0, (%rsp)
        call      \callee@PLT
        movsd     %xmm0, 16(%rsp)
        movsd     8(%rsp), %xmm0
        call      \callee@PLT
        movsd     16(%rsp), %xmm1
        movsd     %xmm0, 24(%rsp)
        unpcklpd  %xmm0, %xmm1
        movaps    %xmm1, %xmm0
        addq      $40, %rsp
        cfi_adjust_cfa_offset(-40)
        ret
.endm

/* AVX/AVX2 ISA version as wrapper to SSE ISA version.  */
.macro WRAPPER_IMPL_AVX callee
        pushq		%rbp
        cfi_adjust_cfa_offset (8)
        cfi_rel_offset (%rbp, 0)
        movq		%rsp, %rbp
        cfi_def_cfa_register (%rbp)
        andq		$-32, %rsp
        subq		$32, %rsp
        vextractf128	$1, %ymm0, (%rsp)
        vzeroupper
        call		HIDDEN_JUMPTARGET(\callee)
        vmovapd		%xmm0, 16(%rsp)
        vmovaps		(%rsp), %xmm0
        call		HIDDEN_JUMPTARGET(\callee)
        vmovapd		%xmm0, %xmm1
        vmovapd		16(%rsp), %xmm0
        vinsertf128	$1, %xmm1, %ymm0, %ymm0
        movq		%rbp, %rsp
        cfi_def_cfa_register (%rsp)
        popq		%rbp
        cfi_adjust_cfa_offset (-8)
        cfi_restore (%rbp)
        ret
.endm

/* AVX512 ISA version as wrapper to AVX2 ISA version.  */
.macro WRAPPER_IMPL_AVX512 callee
        pushq	%rbp
        cfi_adjust_cfa_offset (8)
        cfi_rel_offset (%rbp, 0)
        movq	%rsp, %rbp
        cfi_def_cfa_register (%rbp)
        andq	$-64, %rsp
        subq	$64, %rsp
/* Below is encoding for vmovaps %zmm0, (%rsp).  */
        .byte	0x62
        .byte	0xf1
        .byte	0x7c
        .byte	0x48
        .byte	0x29
        .byte	0x04
        .byte	0x24
/* Below is encoding for vmovapd (%rsp), %ymm0.  */
        .byte	0xc5
        .byte	0xfd
        .byte	0x28
        .byte	0x04
        .byte	0x24
        call	HIDDEN_JUMPTARGET(\callee)
/* Below is encoding for vmovapd 32(%rsp), %ymm0.  */
        .byte	0xc5
        .byte	0xfd
        .byte	0x28
        .byte	0x44
        .byte	0x24
        .byte	0x20
        call	HIDDEN_JUMPTARGET(\callee)
        movq	%rbp, %rsp
        cfi_def_cfa_register (%rsp)
        popq	%rbp
        cfi_adjust_cfa_offset (-8)
        cfi_restore (%rbp)
        ret
.endm
