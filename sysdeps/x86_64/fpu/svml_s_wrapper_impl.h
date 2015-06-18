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
        movss     %xmm0, 16(%rsp)
        movss     4(%rsp), %xmm0
        call      \callee@PLT
        movss     %xmm0, 20(%rsp)
        movss     8(%rsp), %xmm0
        call      \callee@PLT
        movss     %xmm0, 24(%rsp)
        movss     12(%rsp), %xmm0
        call      \callee@PLT
        movss     16(%rsp), %xmm3
        movss     20(%rsp), %xmm2
        movss     24(%rsp), %xmm1
        movss     %xmm0, 28(%rsp)
        unpcklps  %xmm1, %xmm3
        unpcklps  %xmm0, %xmm2
        unpcklps  %xmm2, %xmm3
        movaps    %xmm3, %xmm0
        addq      $40, %rsp
        cfi_adjust_cfa_offset(-40)
        ret
.endm

/* 2 argument SSE2 ISA version as wrapper to scalar.  */
.macro WRAPPER_IMPL_SSE2_ff callee
        subq      $56, %rsp
        cfi_adjust_cfa_offset(56)
        movaps    %xmm0, (%rsp)
        movaps    %xmm1, 16(%rsp)
        call      \callee@PLT
        movss     %xmm0, 32(%rsp)
        movss     4(%rsp), %xmm0
        movss     20(%rsp), %xmm1
        call      \callee@PLT
        movss     %xmm0, 36(%rsp)
        movss     8(%rsp), %xmm0
        movss     24(%rsp), %xmm1
        call      \callee@PLT
        movss     %xmm0, 40(%rsp)
        movss     12(%rsp), %xmm0
        movss     28(%rsp), %xmm1
        call      \callee@PLT
        movss     32(%rsp), %xmm3
        movss     36(%rsp), %xmm2
        movss     40(%rsp), %xmm1
        movss     %xmm0, 44(%rsp)
        unpcklps  %xmm1, %xmm3
        unpcklps  %xmm0, %xmm2
        unpcklps  %xmm2, %xmm3
        movaps    %xmm3, %xmm0
        addq      $56, %rsp
        cfi_adjust_cfa_offset(-56)
        ret
.endm

/* AVX/AVX2 ISA version as wrapper to SSE ISA version.  */
.macro WRAPPER_IMPL_AVX callee
        pushq     	%rbp
        cfi_adjust_cfa_offset (8)
        cfi_rel_offset (%rbp, 0)
        movq      	%rsp, %rbp
        cfi_def_cfa_register (%rbp)
        andq      	$-32, %rsp
        subq      	$32, %rsp
        vextractf128 	$1, %ymm0, (%rsp)
        vzeroupper
        call      	HIDDEN_JUMPTARGET(\callee)
        vmovaps   	%xmm0, 16(%rsp)
        vmovaps   	(%rsp), %xmm0
        call      	HIDDEN_JUMPTARGET(\callee)
        vmovaps   	%xmm0, %xmm1
        vmovaps   	16(%rsp), %xmm0
        vinsertf128 	$1, %xmm1, %ymm0, %ymm0
        movq      	%rbp, %rsp
        cfi_def_cfa_register (%rsp)
        popq      	%rbp
        cfi_adjust_cfa_offset (-8)
        cfi_restore (%rbp)
        ret
.endm

/* 2 argument AVX/AVX2 ISA version as wrapper to SSE ISA version.  */
.macro WRAPPER_IMPL_AVX_ff callee
        pushq     %rbp
        cfi_adjust_cfa_offset (8)
        cfi_rel_offset (%rbp, 0)
        movq      %rsp, %rbp
        cfi_def_cfa_register (%rbp)
        andq      $-32, %rsp
        subq      $64, %rsp
        vextractf128 $1, %ymm0, 16(%rsp)
        vextractf128 $1, %ymm1, (%rsp)
        vzeroupper
        call      HIDDEN_JUMPTARGET(\callee)
        vmovaps   %xmm0, 32(%rsp)
        vmovaps   16(%rsp), %xmm0
        vmovaps   (%rsp), %xmm1
        call      HIDDEN_JUMPTARGET(\callee)
        vmovaps   %xmm0, %xmm1
        vmovaps   32(%rsp), %xmm0
        vinsertf128 $1, %xmm1, %ymm0, %ymm0
        movq      %rbp, %rsp
        cfi_def_cfa_register (%rsp)
        popq      %rbp
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
/* Below is encoding for vmovaps (%rsp), %ymm0.  */
        .byte	0xc5
        .byte	0xfc
        .byte	0x28
        .byte	0x04
        .byte	0x24
        call	HIDDEN_JUMPTARGET(\callee)
/* Below is encoding for vmovaps 32(%rsp), %ymm0.  */
        .byte	0xc5
        .byte	0xfc
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

/* 2 argument AVX512 ISA version as wrapper to AVX2 ISA version.  */
.macro WRAPPER_IMPL_AVX512_ff callee
        pushq     %rbp
        cfi_adjust_cfa_offset (8)
        cfi_rel_offset (%rbp, 0)
        movq      %rsp, %rbp
        cfi_def_cfa_register (%rbp)
        andq      $-64, %rsp
        subq      $128, %rsp
/* Below is encoding for vmovaps %zmm0, (%rsp).  */
        .byte	0x62
        .byte	0xf1
        .byte	0x7c
        .byte	0x48
        .byte	0x29
        .byte	0x04
        .byte	0x24
/* Below is encoding for vmovaps %zmm1, 64(%rsp).  */
        .byte	0x62
        .byte	0xf1
        .byte	0x7c
        .byte	0x48
        .byte	0x29
        .byte	0x4c
        .byte	0x24
/* Below is encoding for vmovaps (%rsp), %ymm0.  */
        .byte	0xc5
        .byte	0xfc
        .byte	0x28
        .byte	0x04
        .byte	0x24
/* Below is encoding for vmovaps 64(%rsp), %ymm1.  */
        .byte	0xc5
        .byte	0xfc
        .byte	0x28
        .byte	0x4c
        .byte	0x24
        .byte	0x40
        call      HIDDEN_JUMPTARGET(\callee)
/* Below is encoding for vmovaps 32(%rsp), %ymm0.  */
        .byte	0xc5
        .byte	0xfc
        .byte	0x28
        .byte	0x44
        .byte	0x24
        .byte	0x20
/* Below is encoding for vmovaps 96(%rsp), %ymm1.  */
        .byte	0xc5
        .byte	0xfc
        .byte	0x28
        .byte	0x4c
        .byte	0x24
        .byte	0x60
        call      HIDDEN_JUMPTARGET(\callee)
        movq      %rbp, %rsp
        cfi_def_cfa_register (%rsp)
        popq      %rbp
        cfi_adjust_cfa_offset (-8)
        cfi_restore (%rbp)
        ret
.endm
