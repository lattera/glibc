/* Function sincosf vectorized with AVX2.
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

#include <sysdep.h>
#include "svml_s_trig_data.h"

	.text
ENTRY(_ZGVdN8vvv_sincosf_avx2)
/*
   ALGORITHM DESCRIPTION:

     1) Range reduction to [-Pi/4; +Pi/4] interval
        a) Grab sign from source argument and save it.
        b) Remove sign using AND operation
        c) Getting octant Y by 2/Pi multiplication
        d) Add "Right Shifter" value
        e) Treat obtained value as integer S for destination sign setting.
           SS = ((S-S&1)&2)<<30; For sin part
           SC = ((S+S&1)&2)<<30; For cos part
        f) Change destination sign if source sign is negative
           using XOR operation.
        g) Subtract "Right Shifter" (0x4B000000) value
        h) Subtract Y*(PI/2) from X argument, where PI/2 divided to 4 parts:
           X = X - Y*PI1 - Y*PI2 - Y*PI3 - Y*PI4;
     2) Polynomial (minimax for sin within  [-Pi/4; +Pi/4] interval)
        a) Calculate X^2 = X * X
        b) Calculate 2 polynomials for sin and cos:
           RS = X * ( A0 + X^2 * (A1 + x^2 * (A2 + x^2 * (A3))));
           RC = B0 + X^2 * (B1 + x^2 * (B2 + x^2 * (B3 + x^2 * (B4))));
        c) Swap RS & RC if if first bit of obtained value after
           Right Shifting is set to 1. Using And, Andnot & Or operations.
     3) Destination sign setting
        a) Set shifted destination sign using XOR operation:
           R1 = XOR( RS, SS );
           R2 = XOR( RC, SC ).  */

        pushq     %rbp
        cfi_adjust_cfa_offset (8)
        cfi_rel_offset (%rbp, 0)
        movq      %rsp, %rbp
        cfi_def_cfa_register (%rbp)
        andq      $-64, %rsp
        subq      $448, %rsp
        movq      __svml_s_trig_data@GOTPCREL(%rip), %rax
        vmovdqa   %ymm0, %ymm5
        vmovups   %ymm13, 352(%rsp)
        vmovups __sAbsMask(%rax), %ymm2
        vmovups __sInvPI(%rax), %ymm1
        vmovups __sPI1_FMA(%rax), %ymm13
        vmovups   %ymm15, 288(%rsp)

/* Absolute argument computation */
        vandps    %ymm2, %ymm5, %ymm4

/* c) Getting octant Y by 2/Pi multiplication
   d) Add "Right Shifter" value */
        vfmadd213ps __sRShifter(%rax), %ymm4, %ymm1

/* e) Treat obtained value as integer S for destination sign setting */
        vpslld    $31, %ymm1, %ymm0

/* g) Subtract "Right Shifter" (0x4B000000) value */
        vsubps __sRShifter(%rax), %ymm1, %ymm1

/* h) Subtract Y*(PI/2) from X argument, where PI/2 divided to 3 parts:
      X = X - Y*PI1 - Y*PI2 - Y*PI3 */
        vmovdqa   %ymm4, %ymm7
        vfnmadd231ps %ymm1, %ymm13, %ymm7
        vfnmadd231ps __sPI2_FMA(%rax), %ymm1, %ymm7
        vandps __sSignMask(%rax), %ymm7, %ymm15
        vxorps __sOneHalf(%rax), %ymm15, %ymm6

/* Add correction term 0.5 for cos() part */
        vaddps    %ymm6, %ymm1, %ymm6
        vmovdqa   %ymm4, %ymm3
        vfnmadd231ps %ymm6, %ymm13, %ymm3
        vmovups __sPI3_FMA(%rax), %ymm13
        vcmpnle_uqps __sRangeReductionVal(%rax), %ymm4, %ymm4
        vfnmadd231ps __sPI2_FMA(%rax), %ymm6, %ymm3
        vfnmadd213ps %ymm7, %ymm13, %ymm1
        vfnmadd213ps %ymm3, %ymm13, %ymm6

/* Result sign calculations */
        vxorps __sSignMask(%rax), %ymm15, %ymm3
        vxorps    %ymm0, %ymm3, %ymm7
        vxorps    %ymm7, %ymm6, %ymm3
        vxorps    %ymm0, %ymm1, %ymm15
        vandnps   %ymm5, %ymm2, %ymm6
        vmovups __sA7_FMA(%rax), %ymm2
        vmulps    %ymm15, %ymm15, %ymm13
        vmovups __sA9_FMA(%rax), %ymm7
        vmulps    %ymm3, %ymm3, %ymm1

/* 2) Polynomial (minimax for sin within  [-Pi/4; +Pi/4] interval)
      a) Calculate X^2 = X * X
      b) Calculate 2 polynomials for sin and cos:
         RS = X * ( A0 + X^2 * (A1 + x^2 * (A2 + x^2 * (A3))));
         RC = B0 + X^2 * (B1 + x^2 * (B2 + x^2 * (B3 + x^2 * (B4)))) */
        vmovdqa   %ymm2, %ymm0
        vfmadd231ps __sA9_FMA(%rax), %ymm13, %ymm0
        vfmadd213ps %ymm2, %ymm1, %ymm7
        vfmadd213ps __sA5_FMA(%rax), %ymm13, %ymm0
        vfmadd213ps __sA5_FMA(%rax), %ymm1, %ymm7
        vfmadd213ps __sA3(%rax), %ymm13, %ymm0
        vfmadd213ps __sA3(%rax), %ymm1, %ymm7
        vmulps    %ymm13, %ymm0, %ymm13
        vmulps    %ymm1, %ymm7, %ymm1
        vfmadd213ps %ymm15, %ymm15, %ymm13
        vfmadd213ps %ymm3, %ymm3, %ymm1
        vmovmskps %ymm4, %ecx
        vxorps    %ymm6, %ymm13, %ymm0
        testl     %ecx, %ecx
        jne       .LBL_1_3

.LBL_1_2:
        cfi_remember_state
        vmovups   352(%rsp), %ymm13
        vmovups   288(%rsp), %ymm15
        vmovups   %ymm0, (%rdi)
        vmovups   %ymm1, (%rsi)
        movq      %rbp, %rsp
        cfi_def_cfa_register (%rsp)
        popq      %rbp
        cfi_adjust_cfa_offset (-8)
        cfi_restore (%rbp)
        ret

.LBL_1_3:
        cfi_restore_state
        vmovups   %ymm5, 256(%rsp)
        vmovups   %ymm0, 320(%rsp)
        vmovups   %ymm1, 384(%rsp)
        je        .LBL_1_2

        xorb      %dl, %dl
        xorl      %eax, %eax
        vmovups   %ymm8, 160(%rsp)
        vmovups   %ymm9, 128(%rsp)
        vmovups   %ymm10, 96(%rsp)
        vmovups   %ymm11, 64(%rsp)
        vmovups   %ymm12, 32(%rsp)
        vmovups   %ymm14, (%rsp)
        movq      %rsi, 192(%rsp)
        movq      %r12, 232(%rsp)
        cfi_offset_rel_rsp (12, 232)
        movb      %dl, %r12b
        movq      %r13, 224(%rsp)
        cfi_offset_rel_rsp (13, 224)
        movl      %eax, %r13d
        movq      %r14, 216(%rsp)
        cfi_offset_rel_rsp (14, 216)
        movl      %ecx, %r14d
        movq      %r15, 208(%rsp)
        cfi_offset_rel_rsp (14, 208)
        movq      %rbx, 200(%rsp)
        movq      %rdi, %rbx
        cfi_remember_state

.LBL_1_6:
        btl       %r13d, %r14d
        jc        .LBL_1_13

.LBL_1_7:
        lea       1(%r13), %esi
        btl       %esi, %r14d
        jc        .LBL_1_10

.LBL_1_8:
        incb      %r12b
        addl      $2, %r13d
        cmpb      $16, %r12b
        jb        .LBL_1_6

        vmovups   160(%rsp), %ymm8
        movq      %rbx, %rdi
        vmovups   128(%rsp), %ymm9
        vmovups   96(%rsp), %ymm10
        vmovups   64(%rsp), %ymm11
        vmovups   32(%rsp), %ymm12
        vmovups   (%rsp), %ymm14
        vmovups   320(%rsp), %ymm0
        vmovups   384(%rsp), %ymm1
        movq      192(%rsp), %rsi
        movq      232(%rsp), %r12
        cfi_restore (%r12)
        movq      224(%rsp), %r13
        cfi_restore (%r13)
        movq      216(%rsp), %r14
        cfi_restore (%r14)
        movq      208(%rsp), %r15
        cfi_restore (%r15)
        movq      200(%rsp), %rbx
        jmp       .LBL_1_2

.LBL_1_10:
        cfi_restore_state
        movzbl    %r12b, %r15d
        vmovss    260(%rsp,%r15,8), %xmm0
        vzeroupper

        call      sinf@PLT

        vmovss    %xmm0, 324(%rsp,%r15,8)
        vmovss    260(%rsp,%r15,8), %xmm0

        call      cosf@PLT

        vmovss    %xmm0, 388(%rsp,%r15,8)
        jmp       .LBL_1_8

.LBL_1_13:
        movzbl    %r12b, %r15d
        vmovss    256(%rsp,%r15,8), %xmm0
        vzeroupper

        call      sinf@PLT

        vmovss    %xmm0, 320(%rsp,%r15,8)
        vmovss    256(%rsp,%r15,8), %xmm0

        call      cosf@PLT

        vmovss    %xmm0, 384(%rsp,%r15,8)
        jmp       .LBL_1_7

END(_ZGVdN8vvv_sincosf_avx2)
