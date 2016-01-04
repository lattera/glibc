/* Function sincosf vectorized with AVX-512. KNL and SKX versions.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
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
#include "svml_s_wrapper_impl.h"

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

	.text
ENTRY (_ZGVeN16vvv_sincosf_knl)
#ifndef HAVE_AVX512_ASM_SUPPORT
WRAPPER_IMPL_AVX512_fFF _ZGVdN8vvv_sincosf
#else
        pushq     %rbp
        cfi_adjust_cfa_offset (8)
        cfi_rel_offset (%rbp, 0)
        movq      %rsp, %rbp
        cfi_def_cfa_register (%rbp)
        andq      $-64, %rsp
        subq      $1344, %rsp
        movq      __svml_s_trig_data@GOTPCREL(%rip), %rax
        vmovaps   %zmm0, %zmm2
        movl      $-1, %edx
        vmovups __sAbsMask(%rax), %zmm0
        vmovups __sInvPI(%rax), %zmm3

/* Absolute argument computation */
        vpandd    %zmm0, %zmm2, %zmm1
        vmovups __sPI1_FMA(%rax), %zmm5
        vmovups __sSignMask(%rax), %zmm9
        vpandnd   %zmm2, %zmm0, %zmm0

/* h) Subtract Y*(PI/2) from X argument, where PI/2 divided to 3 parts:
      X = X - Y*PI1 - Y*PI2 - Y*PI3 */
        vmovaps   %zmm1, %zmm6
        vmovaps   %zmm1, %zmm8

/* c) Getting octant Y by 2/Pi multiplication
   d) Add "Right Shifter" value */
        vfmadd213ps __sRShifter(%rax), %zmm1, %zmm3
        vmovups __sPI3_FMA(%rax), %zmm7

/* g) Subtract "Right Shifter" (0x4B000000) value */
        vsubps __sRShifter(%rax), %zmm3, %zmm12

/* e) Treat obtained value as integer S for destination sign setting */
        vpslld    $31, %zmm3, %zmm13
        vmovups __sA7_FMA(%rax), %zmm14
        vfnmadd231ps %zmm12, %zmm5, %zmm6

/* 2) Polynomial (minimax for sin within  [-Pi/4; +Pi/4] interval)
      a) Calculate X^2 = X * X
      b) Calculate 2 polynomials for sin and cos:
         RS = X * ( A0 + X^2 * (A1 + x^2 * (A2 + x^2 * (A3))));
         RC = B0 + X^2 * (B1 + x^2 * (B2 + x^2 * (B3 + x^2 * (B4)))) */
        vmovaps   %zmm14, %zmm15
        vmovups __sA9_FMA(%rax), %zmm3
        vcmpps    $22, __sRangeReductionVal(%rax), %zmm1, %k1
        vpbroadcastd %edx, %zmm1{%k1}{z}
        vfnmadd231ps __sPI2_FMA(%rax), %zmm12, %zmm6
        vptestmd  %zmm1, %zmm1, %k0
        vpandd    %zmm6, %zmm9, %zmm11
        kmovw     %k0, %ecx
        vpxord __sOneHalf(%rax), %zmm11, %zmm4

/* Result sign calculations */
        vpternlogd $150, %zmm13, %zmm9, %zmm11

/* Add correction term 0.5 for cos() part */
        vaddps    %zmm4, %zmm12, %zmm10
        vfnmadd213ps %zmm6, %zmm7, %zmm12
        vfnmadd231ps %zmm10, %zmm5, %zmm8
        vpxord    %zmm13, %zmm12, %zmm13
        vmulps    %zmm13, %zmm13, %zmm12
        vfnmadd231ps __sPI2_FMA(%rax), %zmm10, %zmm8
        vfmadd231ps __sA9_FMA(%rax), %zmm12, %zmm15
        vfnmadd213ps %zmm8, %zmm7, %zmm10
        vfmadd213ps __sA5_FMA(%rax), %zmm12, %zmm15
        vpxord    %zmm11, %zmm10, %zmm5
        vmulps    %zmm5, %zmm5, %zmm4
        vfmadd213ps __sA3(%rax), %zmm12, %zmm15
        vfmadd213ps %zmm14, %zmm4, %zmm3
        vmulps    %zmm12, %zmm15, %zmm14
        vfmadd213ps __sA5_FMA(%rax), %zmm4, %zmm3
        vfmadd213ps %zmm13, %zmm13, %zmm14
        vfmadd213ps __sA3(%rax), %zmm4, %zmm3
        vpxord    %zmm0, %zmm14, %zmm0
        vmulps    %zmm4, %zmm3, %zmm3
        vfmadd213ps %zmm5, %zmm5, %zmm3
        testl     %ecx, %ecx
        jne       .LBL_1_3

.LBL_1_2:
        cfi_remember_state
        vmovups   %zmm0, (%rdi)
        vmovups   %zmm3, (%rsi)
        movq      %rbp, %rsp
        cfi_def_cfa_register (%rsp)
        popq      %rbp
        cfi_adjust_cfa_offset (-8)
        cfi_restore (%rbp)
        ret

.LBL_1_3:
        cfi_restore_state
        vmovups   %zmm2, 1152(%rsp)
        vmovups   %zmm0, 1216(%rsp)
        vmovups   %zmm3, 1280(%rsp)
        je        .LBL_1_2

        xorb      %dl, %dl
        kmovw     %k4, 1048(%rsp)
        xorl      %eax, %eax
        kmovw     %k5, 1040(%rsp)
        kmovw     %k6, 1032(%rsp)
        kmovw     %k7, 1024(%rsp)
        vmovups   %zmm16, 960(%rsp)
        vmovups   %zmm17, 896(%rsp)
        vmovups   %zmm18, 832(%rsp)
        vmovups   %zmm19, 768(%rsp)
        vmovups   %zmm20, 704(%rsp)
        vmovups   %zmm21, 640(%rsp)
        vmovups   %zmm22, 576(%rsp)
        vmovups   %zmm23, 512(%rsp)
        vmovups   %zmm24, 448(%rsp)
        vmovups   %zmm25, 384(%rsp)
        vmovups   %zmm26, 320(%rsp)
        vmovups   %zmm27, 256(%rsp)
        vmovups   %zmm28, 192(%rsp)
        vmovups   %zmm29, 128(%rsp)
        vmovups   %zmm30, 64(%rsp)
        vmovups   %zmm31, (%rsp)
        movq      %rsi, 1056(%rsp)
        movq      %r12, 1096(%rsp)
        cfi_offset_rel_rsp (12, 1096)
        movb      %dl, %r12b
        movq      %r13, 1088(%rsp)
        cfi_offset_rel_rsp (13, 1088)
        movl      %eax, %r13d
        movq      %r14, 1080(%rsp)
        cfi_offset_rel_rsp (14, 1080)
        movl      %ecx, %r14d
        movq      %r15, 1072(%rsp)
        cfi_offset_rel_rsp (15, 1072)
        movq      %rbx, 1064(%rsp)
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
        addb      $1, %r12b
        addl      $2, %r13d
        cmpb      $16, %r12b
        jb        .LBL_1_6

        movq      %rbx, %rdi
        kmovw     1048(%rsp), %k4
        movq      1056(%rsp), %rsi
        kmovw     1040(%rsp), %k5
        movq      1096(%rsp), %r12
        cfi_restore (%r12)
        kmovw     1032(%rsp), %k6
        movq      1088(%rsp), %r13
        cfi_restore (%r13)
        kmovw     1024(%rsp), %k7
        vmovups   960(%rsp), %zmm16
        vmovups   896(%rsp), %zmm17
        vmovups   832(%rsp), %zmm18
        vmovups   768(%rsp), %zmm19
        vmovups   704(%rsp), %zmm20
        vmovups   640(%rsp), %zmm21
        vmovups   576(%rsp), %zmm22
        vmovups   512(%rsp), %zmm23
        vmovups   448(%rsp), %zmm24
        vmovups   384(%rsp), %zmm25
        vmovups   320(%rsp), %zmm26
        vmovups   256(%rsp), %zmm27
        vmovups   192(%rsp), %zmm28
        vmovups   128(%rsp), %zmm29
        vmovups   64(%rsp), %zmm30
        vmovups   (%rsp), %zmm31
        movq      1080(%rsp), %r14
        cfi_restore (%r14)
        movq      1072(%rsp), %r15
        cfi_restore (%r15)
        movq      1064(%rsp), %rbx
        vmovups   1216(%rsp), %zmm0
        vmovups   1280(%rsp), %zmm3
        jmp       .LBL_1_2

.LBL_1_10:
        cfi_restore_state
        movzbl    %r12b, %r15d
        vmovss    1156(%rsp,%r15,8), %xmm0

        call      sinf@PLT

        vmovss    %xmm0, 1220(%rsp,%r15,8)
        vmovss    1156(%rsp,%r15,8), %xmm0

        call      cosf@PLT

        vmovss    %xmm0, 1284(%rsp,%r15,8)
        jmp       .LBL_1_8

.LBL_1_13:
        movzbl    %r12b, %r15d
        vmovss    1152(%rsp,%r15,8), %xmm0

        call      sinf@PLT

        vmovss    %xmm0, 1216(%rsp,%r15,8)
        vmovss    1152(%rsp,%r15,8), %xmm0

        call      cosf@PLT

        vmovss    %xmm0, 1280(%rsp,%r15,8)
        jmp       .LBL_1_7
#endif
END (_ZGVeN16vvv_sincosf_knl)

ENTRY (_ZGVeN16vvv_sincosf_skx)
#ifndef HAVE_AVX512_ASM_SUPPORT
WRAPPER_IMPL_AVX512_fFF _ZGVdN8vvv_sincosf
#else
        pushq     %rbp
        cfi_adjust_cfa_offset (8)
        cfi_rel_offset (%rbp, 0)
        movq      %rsp, %rbp
        cfi_def_cfa_register (%rbp)
        andq      $-64, %rsp
        subq      $1344, %rsp
        movq      __svml_s_trig_data@GOTPCREL(%rip), %rax
        vmovaps   %zmm0, %zmm4
        vmovups __sAbsMask(%rax), %zmm3
        vmovups __sInvPI(%rax), %zmm5
        vmovups __sRShifter(%rax), %zmm6
        vmovups __sPI1_FMA(%rax), %zmm9
        vmovups __sPI2_FMA(%rax), %zmm10
        vmovups __sSignMask(%rax), %zmm14
        vmovups __sOneHalf(%rax), %zmm7
        vmovups __sPI3_FMA(%rax), %zmm12

/* Absolute argument computation */
        vandps    %zmm3, %zmm4, %zmm2

/* c) Getting octant Y by 2/Pi multiplication
   d) Add "Right Shifter" value */
        vfmadd213ps %zmm6, %zmm2, %zmm5
        vcmpps    $18, __sRangeReductionVal(%rax), %zmm2, %k1

/* e) Treat obtained value as integer S for destination sign setting */
        vpslld    $31, %zmm5, %zmm0

/* g) Subtract "Right Shifter" (0x4B000000) value */
        vsubps    %zmm6, %zmm5, %zmm5
        vmovups __sA3(%rax), %zmm6

/* h) Subtract Y*(PI/2) from X argument, where PI/2 divided to 3 parts:
      X = X - Y*PI1 - Y*PI2 - Y*PI3 */
        vmovaps   %zmm2, %zmm11
        vfnmadd231ps %zmm5, %zmm9, %zmm11
        vfnmadd231ps %zmm5, %zmm10, %zmm11
        vandps    %zmm11, %zmm14, %zmm1
        vxorps    %zmm1, %zmm7, %zmm8

/* Result sign calculations */
        vpternlogd $150, %zmm0, %zmm14, %zmm1
        vmovups   .L_2il0floatpacket.13(%rip), %zmm14

/* Add correction term 0.5 for cos() part */
        vaddps    %zmm8, %zmm5, %zmm15
        vfnmadd213ps %zmm11, %zmm12, %zmm5
        vandnps   %zmm4, %zmm3, %zmm11
        vmovups __sA7_FMA(%rax), %zmm3
        vmovaps   %zmm2, %zmm13
        vfnmadd231ps %zmm15, %zmm9, %zmm13
        vxorps    %zmm0, %zmm5, %zmm9
        vmovups __sA5_FMA(%rax), %zmm0
        vfnmadd231ps %zmm15, %zmm10, %zmm13
        vmulps    %zmm9, %zmm9, %zmm8
        vfnmadd213ps %zmm13, %zmm12, %zmm15
        vmovups __sA9_FMA(%rax), %zmm12
        vxorps    %zmm1, %zmm15, %zmm1
        vmulps    %zmm1, %zmm1, %zmm13

/* 2) Polynomial (minimax for sin within  [-Pi/4; +Pi/4] interval)
      a) Calculate X^2 = X * X
      b) Calculate 2 polynomials for sin and cos:
         RS = X * ( A0 + X^2 * (A1 + x^2 * (A2 + x^2 * (A3))));
         RC = B0 + X^2 * (B1 + x^2 * (B2 + x^2 * (B3 + x^2 * (B4)))) */
        vmovaps   %zmm12, %zmm7
        vfmadd213ps %zmm3, %zmm8, %zmm7
        vfmadd213ps %zmm3, %zmm13, %zmm12
        vfmadd213ps %zmm0, %zmm8, %zmm7
        vfmadd213ps %zmm0, %zmm13, %zmm12
        vfmadd213ps %zmm6, %zmm8, %zmm7
        vfmadd213ps %zmm6, %zmm13, %zmm12
        vmulps    %zmm8, %zmm7, %zmm10
        vmulps    %zmm13, %zmm12, %zmm3
        vfmadd213ps %zmm9, %zmm9, %zmm10
        vfmadd213ps %zmm1, %zmm1, %zmm3
        vxorps    %zmm11, %zmm10, %zmm0
        vpandnd   %zmm2, %zmm2, %zmm14{%k1}
        vptestmd  %zmm14, %zmm14, %k0
        kmovw     %k0, %ecx
        testl     %ecx, %ecx
        jne       .LBL_2_3

.LBL_2_2:
        cfi_remember_state
        vmovups   %zmm0, (%rdi)
        vmovups   %zmm3, (%rsi)
        movq      %rbp, %rsp
        cfi_def_cfa_register (%rsp)
        popq      %rbp
        cfi_adjust_cfa_offset (-8)
        cfi_restore (%rbp)
        ret

.LBL_2_3:
        cfi_restore_state
        vmovups   %zmm4, 1152(%rsp)
        vmovups   %zmm0, 1216(%rsp)
        vmovups   %zmm3, 1280(%rsp)
        je        .LBL_2_2

        xorb      %dl, %dl
        xorl      %eax, %eax
        kmovw     %k4, 1048(%rsp)
        kmovw     %k5, 1040(%rsp)
        kmovw     %k6, 1032(%rsp)
        kmovw     %k7, 1024(%rsp)
        vmovups   %zmm16, 960(%rsp)
        vmovups   %zmm17, 896(%rsp)
        vmovups   %zmm18, 832(%rsp)
        vmovups   %zmm19, 768(%rsp)
        vmovups   %zmm20, 704(%rsp)
        vmovups   %zmm21, 640(%rsp)
        vmovups   %zmm22, 576(%rsp)
        vmovups   %zmm23, 512(%rsp)
        vmovups   %zmm24, 448(%rsp)
        vmovups   %zmm25, 384(%rsp)
        vmovups   %zmm26, 320(%rsp)
        vmovups   %zmm27, 256(%rsp)
        vmovups   %zmm28, 192(%rsp)
        vmovups   %zmm29, 128(%rsp)
        vmovups   %zmm30, 64(%rsp)
        vmovups   %zmm31, (%rsp)
        movq      %rsi, 1056(%rsp)
        movq      %r12, 1096(%rsp)
        cfi_offset_rel_rsp (12, 1096)
        movb      %dl, %r12b
        movq      %r13, 1088(%rsp)
        cfi_offset_rel_rsp (13, 1088)
        movl      %eax, %r13d
        movq      %r14, 1080(%rsp)
        cfi_offset_rel_rsp (14, 1080)
        movl      %ecx, %r14d
        movq      %r15, 1072(%rsp)
        cfi_offset_rel_rsp (15, 1072)
        movq      %rbx, 1064(%rsp)
        movq      %rdi, %rbx
        cfi_remember_state

.LBL_2_6:
        btl       %r13d, %r14d
        jc        .LBL_2_13

.LBL_2_7:
        lea       1(%r13), %esi
        btl       %esi, %r14d
        jc        .LBL_2_10

.LBL_2_8:
        incb      %r12b
        addl      $2, %r13d
        cmpb      $16, %r12b
        jb        .LBL_2_6

        kmovw     1048(%rsp), %k4
        movq      %rbx, %rdi
        kmovw     1040(%rsp), %k5
        kmovw     1032(%rsp), %k6
        kmovw     1024(%rsp), %k7
        vmovups   960(%rsp), %zmm16
        vmovups   896(%rsp), %zmm17
        vmovups   832(%rsp), %zmm18
        vmovups   768(%rsp), %zmm19
        vmovups   704(%rsp), %zmm20
        vmovups   640(%rsp), %zmm21
        vmovups   576(%rsp), %zmm22
        vmovups   512(%rsp), %zmm23
        vmovups   448(%rsp), %zmm24
        vmovups   384(%rsp), %zmm25
        vmovups   320(%rsp), %zmm26
        vmovups   256(%rsp), %zmm27
        vmovups   192(%rsp), %zmm28
        vmovups   128(%rsp), %zmm29
        vmovups   64(%rsp), %zmm30
        vmovups   (%rsp), %zmm31
        vmovups   1216(%rsp), %zmm0
        vmovups   1280(%rsp), %zmm3
        movq      1056(%rsp), %rsi
        movq      1096(%rsp), %r12
        cfi_restore (%r12)
        movq      1088(%rsp), %r13
        cfi_restore (%r13)
        movq      1080(%rsp), %r14
        cfi_restore (%r14)
        movq      1072(%rsp), %r15
        cfi_restore (%r15)
        movq      1064(%rsp), %rbx
        jmp       .LBL_2_2

.LBL_2_10:
        cfi_restore_state
        movzbl    %r12b, %r15d
        vmovss    1156(%rsp,%r15,8), %xmm0
        vzeroupper
        vmovss    1156(%rsp,%r15,8), %xmm0

        call      sinf@PLT

        vmovss    %xmm0, 1220(%rsp,%r15,8)
        vmovss    1156(%rsp,%r15,8), %xmm0

        call      cosf@PLT

        vmovss    %xmm0, 1284(%rsp,%r15,8)
        jmp       .LBL_2_8

.LBL_2_13:
        movzbl    %r12b, %r15d
        vmovss    1152(%rsp,%r15,8), %xmm0
        vzeroupper
        vmovss    1152(%rsp,%r15,8), %xmm0

        call      sinf@PLT

        vmovss    %xmm0, 1216(%rsp,%r15,8)
        vmovss    1152(%rsp,%r15,8), %xmm0

        call      cosf@PLT

        vmovss    %xmm0, 1280(%rsp,%r15,8)
        jmp       .LBL_2_7
#endif
END (_ZGVeN16vvv_sincosf_skx)

	.section .rodata, "a"
.L_2il0floatpacket.13:
	.long	0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff,0xffffffff
	.type	.L_2il0floatpacket.13,@object
