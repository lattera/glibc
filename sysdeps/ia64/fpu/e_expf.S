.file "expf.s"

// Copyright (c) 2000, 2001, Intel Corporation
// All rights reserved.
//
// Contributed 2/2/2000 by John Harrison, Ted Kubaska, Bob Norin, Shane Story,
// and Ping Tak Peter Tang of the Computational Software Lab, Intel Corporation.
//
// WARRANTY DISCLAIMER
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of this code, and requests that all
// problem reports or change requests be submitted to it directly at
// http://developer.intel.com/opensource.

// History
//==============================================================
// 4/04/00  Unwind update
// 4/04/00  Unwind support added
// 8/15/00  Bundle added after call to __libm_error_support to properly
//          set [the previously overwritten] GR_Parameter_RESULT.
// 8/21/00  Improvements to save 2 cycles on main path, and shorten x=0 case
// 12/07/00 Widen main path, shorten x=inf, nan paths
//

#include "libm_support.h"

// Assembly macros
//==============================================================
// integer registers used

 exp_GR_0x0f                = r33
 exp_GR_0xf0                = r34

 EXP_AD_P_1                 = r36
 EXP_AD_P_2                 = r37
 EXP_AD_T1                  = r38
 EXP_AD_T2                  = r39
 exp_GR_Mint                = r40

 exp_GR_Mint_p_128          = r41
 exp_GR_Ind1                = r42
 EXP_AD_M1                  = r43
 exp_GR_Ind2                = r44
 EXP_AD_M2                  = r45

 exp_GR_min_oflow           = r46
 exp_GR_max_zero            = r47
 exp_GR_max_norm            = r48
 exp_GR_max_uflow           = r49
 exp_GR_min_norm            = r50

 exp_GR_17ones              = r51
 exp_GR_gt_ln               = r52
 exp_GR_T2_size             = r53

 exp_GR_17ones_m1           = r56
 exp_GR_one                 = r57



GR_SAVE_B0                    = r53
GR_SAVE_PFS                   = r55
GR_SAVE_GP                    = r54 

GR_Parameter_X                = r59
GR_Parameter_Y                = r60
GR_Parameter_RESULT           = r61
GR_Parameter_TAG              = r62

FR_X             = f10
FR_Y             = f1
FR_RESULT        = f8


// floating point registers used

 EXP_MIN_SGL_OFLOW_ARG      = f11
 EXP_MAX_SGL_ZERO_ARG       = f12
 EXP_MAX_SGL_NORM_ARG       = f13
 EXP_MAX_SGL_UFLOW_ARG      = f14
 EXP_MIN_SGL_NORM_ARG       = f15

 exp_coeff_P5               = f32
 exp_coeff_P6               = f33
 exp_coeff_P3               = f34
 exp_coeff_P4               = f35

 exp_coeff_P1               = f36
 exp_coeff_P2               = f37
 exp_Mx                     = f38
 exp_Mfloat                 = f39
 exp_R                      = f40

 exp_P1                     = f41
 exp_P2                     = f42
 exp_P3                     = f43
 exp_Rsq                    = f44
 exp_R4                     = f45

 exp_P4                     = f46
 exp_P5                     = f47
 exp_P6                     = f48
 exp_P7                     = f49
 exp_T1                     = f50

 exp_T2                     = f51
 exp_T                      = f52
 exp_A                      = f53
 exp_norm_f8                = f54
 exp_wre_urm_f8             = f55

 exp_ftz_urm_f8             = f56
 exp_gt_pln                 = f57


#ifdef _LIBC
.rodata
#else
.data
#endif

.align 16

exp_coeff_1_table:
ASM_TYPE_DIRECTIVE(exp_coeff_1_table,@object)
data8 0x3F56F35FDE4F8563 // p5
data8 0x3F2A378BEFECCFDD // p6
data8 0x3FE00000258C581D // p1
data8 0x3FC555557AE7B3D4 // p2
ASM_SIZE_DIRECTIVE(exp_coeff_1_table)


exp_coeff_2_table:
ASM_TYPE_DIRECTIVE(exp_coeff_2_table,@object)
data8 0x3FA5551BB6592FAE // p3
data8 0x3F8110E8EBFFD485 // p4
ASM_SIZE_DIRECTIVE(exp_coeff_2_table)


exp_T2_table:
ASM_TYPE_DIRECTIVE(exp_T2_table,@object)
data8 0xa175cf9cd7d85844 , 0x00003f46 // exp(-128)
data8 0xdb7279415a1f9eed , 0x00003f47 // exp(-127)
data8 0x95213b242bd8ca5f , 0x00003f49 // exp(-126)
data8 0xcab03c968c989f83 , 0x00003f4a // exp(-125)
data8 0x89bdb674702961ad , 0x00003f4c // exp(-124)
data8 0xbb35a2eec278be35 , 0x00003f4d // exp(-123)
data8 0xfe71b17f373e7e7a , 0x00003f4e // exp(-122)
data8 0xace9a6ec52a39b63 , 0x00003f50 // exp(-121)
data8 0xeb03423fe393cf1c , 0x00003f51 // exp(-120)
data8 0x9fb52c5bcaef1693 , 0x00003f53 // exp(-119)
data8 0xd910b6377ed60bf1 , 0x00003f54 // exp(-118)
data8 0x9382dad8a9fdbfe4 , 0x00003f56 // exp(-117)
data8 0xc87d0a84dea869a3 , 0x00003f57 // exp(-116)
data8 0x883efb4c6d1087b0 , 0x00003f59 // exp(-115)
data8 0xb92d7373dce9a502 , 0x00003f5a // exp(-114)
data8 0xfbaeb020577fb0cb , 0x00003f5b // exp(-113)
ASM_SIZE_DIRECTIVE(exp_T2_table)


exp_T1_table:
ASM_TYPE_DIRECTIVE(exp_T1_table,@object)
data8 0x8000000000000000 , 0x00003fff // exp(16 * 0)
data8 0x87975e8540010249 , 0x00004016 // exp(16 * 1) 
data8 0x8fa1fe625b3163ec , 0x0000402d // exp(16 * 2) 
data8 0x9826b576512a59d7 , 0x00004044 // exp(16 * 3) 
data8 0xa12cc167acbe6902 , 0x0000405b // exp(16 * 4) 
data8 0xaabbcdcc279f59e4 , 0x00004072 // exp(16 * 5) 
data8 0xb4dbfaadc045d16f , 0x00004089 // exp(16 * 6) 
data8 0xbf95e372ccdbf146 , 0x000040a0 // exp(16 * 7) 
data8 0xcaf2a62eea10bbfb , 0x000040b7 // exp(16 * 8) 
data8 0xd6fbeb62fddbd340 , 0x000040ce // exp(16 * 9) 
data8 0xe3bbee32e4a440ea , 0x000040e5 // exp(16 * 10)
data8 0xf13d8517c34199a8 , 0x000040fc // exp(16 * 11)
data8 0xff8c2b166241eedd , 0x00004113 // exp(16 * 12)
data8 0x875a04c0b38d6129 , 0x0000412b // exp(16 * 13)
data8 0x8f610127db6774d7 , 0x00004142 // exp(16 * 14)
data8 0x97e1dd87e5c20bb6 , 0x00004159 // exp(16 * 15)
ASM_SIZE_DIRECTIVE(exp_T1_table)

// Argument Reduction
//  exp_Mx = (int)f8            ==> The value of f8 rounded to int is placed into the
//                                  significand of exp_Mx as a two's
//                                  complement number.

// Later we want to have exp_Mx in a general register. Do this with a getf.sig
// and call the general register exp_GR_Mint

//  exp_Mfloat = (float)(int)f8 ==> the two's complement number in
//                                  significand of exp_Mx is turned
//                                  into a floating point number.
//  R = 1 - exp_Mfloat          ==> reduced argument

// Core Approximation
// Calculate a series in R
//  R * p6 + p5
//  R * p4 + p3
//  R * p2 + p1
//  R^2
//  R^4
//  R^2(R * p6 + p5) + (R * p4 + p3)
//  R^2(R * p2 + p1)
//  R^4(R^2(R * p6 + p5) + (R * p4 + p3)) + (R^2(R * p2 + p1))
//  R + 1
//  exp(R) = (1 + R) + R^4(R^2(R * p6 + p5) + (R * p4 + p3)) + (R^2(R * p2 + p1))
//  exp(R) = 1 + R + R^2 * p1 + R^3 * p2 + R^4 * p3 + R^5 * p4 + R^6 * p5 + R^7 * p6

// Reconstruction
// signficand of exp_Mx is two's complement,
// -103 < x < 89
// The smallest single denormal is 2^-149 = ssdn
//    For e^x = ssdn
//        x   = log(ssdn) = -103.279
//    But with rounding result goes to ssdn until -103.972079
// The largest single normal is  1.<23 1's> 2^126 ~ 2^127 = lsn
//    For e^x = lsn
//        x   = log(lsn) = 88.7228
//
// expf overflows                       when x > 42b17218 = 88.7228
// expf returns largest single denormal when x = c2aeac50
// expf goes to zero when                    x < c2cff1b5 

// Consider range of 8-bit two's complement, -128 ---> 127
// Add 128; range becomes                       0 ---> 255

// The number (=i) in 0 ---> 255 is used as offset into two tables.

// i = abcd efgh = abcd * 16 + efgh = i1 * 16 + i2

// i1 = (exp_GR_Mint + 128)  & 0xf0 (show 0xf0 as -0x10 to avoid assembler error)
//                                  (The immediate in the AND is an 8-bit two's complement)
// i1 = i1 + start of T1 table (EXP_AD_T1)
//    Note that the entries in T1 are double-extended numbers on 16-byte boundaries
//    and that i1 is already shifted left by 16 after the AND.

// i2 must be shifted left by 4 before adding to the start of the table.
// i2 = ((exp_GR_Mint + 128)  & 0x0f) << 4
// i2 = i2 + start of T2 table (EXP_AD_T2)

// T      = T1 * T2
// A      = T * (1 + R)
// answer = T *  (R^2 * p1 + R^3 * p2 + R^4 * p3 + R^5 * p4 + R^6 * p5 + R^7 * p6) +
//          T *  (1 + R)
//        = T * exp(R)


.global expf#

.section .text
.proc  expf#
.align 32
expf:
#ifdef _LIBC
.global __ieee754_expf#
__ieee754_expf:
#endif

{ .mfi
     alloc      r32            = ar.pfs,1,26,4,0
     fcvt.fx.s1   exp_Mx       =    f8
     mov       exp_GR_17ones   =    0x1FFFF
}
{ .mlx
     addl      EXP_AD_P_1      =    @ltoff(exp_coeff_1_table),gp
     movl      exp_GR_min_oflow = 0x42b17218    
}
;;

// Fnorm done to take any enabled faults
{ .mfi
     ld8       EXP_AD_P_1      =  [EXP_AD_P_1]
     fclass.m  p6,p0      = f8, 0x07	//@zero
     nop.i 999
}
{ .mfi
     add       exp_GR_max_norm = -1, exp_GR_min_oflow  // 0x42b17217
     fnorm     exp_norm_f8     =    f8
     nop.i 999
}
;;

{ .mfi
     setf.s    EXP_MIN_SGL_OFLOW_ARG = exp_GR_min_oflow  // 0x42b17218
     fclass.m  p7,p0      = f8, 0x22	// Test for x=-inf
     mov       exp_GR_0xf0 = 0x0f0
}
{ .mlx
     setf.s    EXP_MAX_SGL_NORM_ARG = exp_GR_max_norm
     movl      exp_GR_max_zero = 0xc2cff1b5    
}
;;


{ .mlx
     mov       exp_GR_0x0f = 0x00f
     movl      exp_GR_max_uflow = 0xc2aeac50    
}
{ .mfb
     nop.m 999
(p6) fma.s     f8 = f1,f1,f0
(p6) br.ret.spnt   b0        // quick exit for x=0
}
;;

{ .mfi
     setf.s    EXP_MAX_SGL_ZERO_ARG = exp_GR_max_zero
     fclass.m  p8,p0      = f8, 0x21	// Test for x=+inf
     adds      exp_GR_min_norm = 1, exp_GR_max_uflow  // 0xc2aeac51
}
{ .mfb
     ldfpd     exp_coeff_P5,exp_coeff_P6     =    [EXP_AD_P_1],16
(p7) fma.s     f8 = f0,f0,f0
(p7) br.ret.spnt   b0        // quick exit for x=-inf
}
;;

{ .mmf
     ldfpd     exp_coeff_P1,exp_coeff_P2     =    [EXP_AD_P_1],16
     setf.s    EXP_MAX_SGL_UFLOW_ARG = exp_GR_max_uflow
     fclass.m  p9,p0      = f8, 0xc3	// Test for x=nan
}
;;

{ .mmb
     ldfpd     exp_coeff_P3,exp_coeff_P4     =    [EXP_AD_P_1],16
     setf.s    EXP_MIN_SGL_NORM_ARG = exp_GR_min_norm
(p8) br.ret.spnt   b0        // quick exit for x=+inf
}
;;

// EXP_AD_P_1 now points to exp_T2_table
{ .mfi
     mov exp_GR_T2_size           = 0x100
     fcvt.xf   exp_Mfloat     =    exp_Mx
     nop.i 999
}
;;

{ .mfb
     getf.sig  exp_GR_Mint    =    exp_Mx
(p9) fmerge.s     f8 = exp_norm_f8, exp_norm_f8
(p9) br.ret.spnt   b0        // quick exit for x=nan
}
;;

{ .mmi
     nop.m 999
     mov      EXP_AD_T2       =  EXP_AD_P_1
     add      EXP_AD_T1       =  exp_GR_T2_size,EXP_AD_P_1 ;;
}


{ .mmi
     adds      exp_GR_Mint_p_128   =    0x80,exp_GR_Mint ;;
     and       exp_GR_Ind1      =    exp_GR_Mint_p_128, exp_GR_0xf0
     and       exp_GR_Ind2      =    exp_GR_Mint_p_128, exp_GR_0x0f ;;
}

// Divide arguments into the following categories:
//  Certain Underflow/zero  p11 - -inf < x <= MAX_SGL_ZERO_ARG 
//  Certain Underflow       p12 - MAX_SGL_ZERO_ARG < x <= MAX_SGL_UFLOW_ARG 
//  Possible Underflow      p13 - MAX_SGL_UFLOW_ARG < x < MIN_SGL_NORM_ARG
//  Certain Safe                - MIN_SGL_NORM_ARG <= x <= MAX_SGL_NORM_ARG
//  Possible Overflow       p14 - MAX_SGL_NORM_ARG < x < MIN_SGL_OFLOW_ARG
//  Certain Overflow        p15 - MIN_SGL_OFLOW_ARG <= x < +inf
//
// If the input is really a single arg, then there will never be "Possible
// Underflow" or "Possible Overflow" arguments.
//

{ .mfi
     add       EXP_AD_M1 =    exp_GR_Ind1,EXP_AD_T1
     fcmp.ge.s1  p15,p14 = exp_norm_f8,EXP_MIN_SGL_OFLOW_ARG
     nop.i 999
}
{ .mfi
     shladd       EXP_AD_M2                =    exp_GR_Ind2,4,EXP_AD_T2
     fms.s1    exp_R                    =    f1,f8,exp_Mfloat
     nop.i 999 ;;
}

{ .mfi
     ldfe           exp_T1    =    [EXP_AD_M1]
     fcmp.le.s1  p11,p12 = exp_norm_f8,EXP_MAX_SGL_ZERO_ARG
     nop.i 999 ;;
}

{ .mfb
      ldfe           exp_T2   =    [EXP_AD_M2]
(p14) fcmp.gt.s1  p14,p0 = exp_norm_f8,EXP_MAX_SGL_NORM_ARG
(p15) br.cond.spnt L(EXP_CERTAIN_OVERFLOW) ;;
}

{ .mfb
      nop.m 999
(p12) fcmp.le.s1  p12,p0 = exp_norm_f8,EXP_MAX_SGL_UFLOW_ARG
(p11) br.cond.spnt L(EXP_CERTAIN_UNDERFLOW_ZERO)
}
;;

{ .mfi
      nop.m 999
(p13) fcmp.lt.s1  p13,p0 = exp_norm_f8,EXP_MIN_SGL_NORM_ARG
      nop.i 999
}
;;


{ .mfi
     nop.m                 999
     fma.s1    exp_Rsq   =    exp_R,exp_R,f0
     nop.i                 999
}
{ .mfi
     nop.m                 999
     fma.s1    exp_P3    =    exp_R,exp_coeff_P2,exp_coeff_P1
     nop.i                 999 
}
;;

{ .mfi
     nop.m                 999
     fma.s1    exp_P1    =    exp_R,exp_coeff_P6,exp_coeff_P5
     nop.i                 999 
}
{ .mfi
     nop.m                 999
     fma.s1    exp_P2    =    exp_R,exp_coeff_P4,exp_coeff_P3
     nop.i                 999
}
;;


{ .mfi
     nop.m                 999
     fma.s1    exp_P7    =    f1,exp_R,f1
     nop.i                 999
}
;;


{ .mfi
     nop.m                 999
     fma.s1    exp_P5    =    exp_Rsq,exp_P3,f0
     nop.i                 999
}
{ .mfi
     nop.m                 999
     fma.s1    exp_R4    =    exp_Rsq,exp_Rsq,f0
     nop.i                 999 
}
;;

{ .mfi
     nop.m                 999
     fma.s1    exp_T     =    exp_T1,exp_T2,f0
     nop.i                 999 
}
{ .mfi
     nop.m                 999
     fma.s1    exp_P4    =    exp_Rsq,exp_P1,exp_P2
     nop.i                 999 
}
;;

{ .mfi
     nop.m                 999
     fma.s1    exp_A     =    exp_T,exp_P7,f0
     nop.i                 999
}
{ .mfi
     nop.m                 999
     fma.s1    exp_P6    =    exp_R4,exp_P4,exp_P5
     nop.i                 999
}
;;

{ .bbb
(p12) br.cond.spnt L(EXP_CERTAIN_UNDERFLOW)
(p13) br.cond.spnt L(EXP_POSSIBLE_UNDERFLOW)
(p14) br.cond.spnt L(EXP_POSSIBLE_OVERFLOW)
}
;;

{ .mfb
     nop.m            999
     fma.s     f8   =    exp_T,exp_P6,exp_A
     br.ret.sptk     b0
}
;;

L(EXP_POSSIBLE_OVERFLOW):

// We got an answer. EXP_MAX_SGL_NORM_ARG < x < EXP_MIN_SGL_OFLOW_ARG
// overflow is a possibility, not a certainty
// Set wre in s2 and perform the last operation with s2

// We define an overflow when the answer with
//    WRE set
//    user-defined rounding mode
// is lsn +1

// Is the exponent 1 more than the largest single?
// If so, go to ERROR RETURN, else (no overflow) get the answer and
// leave.

// Largest single is FE (biased single)
//                   FE - 7F + FFFF = 1007E

// Create + largest_single_plus_ulp
// Create - largest_single_plus_ulp

// Calculate answer with WRE set.

// Cases when answer is lsn+1  are as follows:

//           midpoint
//              |
//  lsn         |         lsn+1
// --+----------|----------+------------
//              |
//    +inf          +inf      -inf
//                  RN         RN
//                             RZ
// exp_gt_pln contains the floating point number lsn+1.
// The setf.exp puts 0x1007f in the exponent and 0x800... in the significand.

// If the answer is >= lsn+1, we have overflowed.
// Then p6 is TRUE. Set the overflow tag, save input in FR_X,
// do the final calculation for IEEE result, and branch to error return.

{ .mfi
       mov         exp_GR_gt_ln    = 0x1007F 
       fsetc.s2    0x7F,0x42
       nop.i 999
}
;;

{ .mfi
       setf.exp      exp_gt_pln    = exp_GR_gt_ln
       fma.s.s2    exp_wre_urm_f8  = exp_T,  exp_P6, exp_A
       nop.i 999
}
;;

{ .mfi
       nop.m 999
       fsetc.s2 0x7F,0x40
       nop.i 999
}
;;

{ .mfi
       nop.m 999
       fcmp.ge.unc.s1 p6, p0       =  exp_wre_urm_f8, exp_gt_pln
       nop.i 999
}
;;

{ .mfb
       nop.m 999
       nop.f 999
(p6)   br.cond.spnt L(EXP_CERTAIN_OVERFLOW)  // Branch if really overflow
}
;;

{ .mfb
       nop.m 999
       fma.s        f8             = exp_T,  exp_P6, exp_A
       br.ret.sptk     b0                 // Exit if really no overflow
}
;;

L(EXP_CERTAIN_OVERFLOW):
{ .mmi
      sub   exp_GR_17ones_m1 = exp_GR_17ones, r0, 1 ;;
      setf.exp     f9 = exp_GR_17ones_m1
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
      fmerge.s FR_X = f8,f8
      nop.i 999
}
{ .mfb
      mov        GR_Parameter_TAG = 16
      fma.s       FR_RESULT = f9, f9, f0    // Set I,O and +INF result
      br.cond.sptk  __libm_error_region ;;                             
}

L(EXP_POSSIBLE_UNDERFLOW): 

// We got an answer. EXP_MAX_SGL_UFLOW_ARG < x < EXP_MIN_SGL_NORM_ARG
// underflow is a possibility, not a certainty

// We define an underflow when the answer with
//    ftz set
// is zero (tiny numbers become zero)

// Notice (from below) that if we have an unlimited exponent range,
// then there is an extra machine number E between the largest denormal and
// the smallest normal.

// So if with unbounded exponent we round to E or below, then we are
// tiny and underflow has occurred.

// But notice that you can be in a situation where we are tiny, namely
// rounded to E, but when the exponent is bounded we round to smallest
// normal. So the answer can be the smallest normal with underflow.

//                           E
// -----+--------------------+--------------------+-----
//      |                    |                    |
//   1.1...10 2^-7f      1.1...11 2^-7f      1.0...00 2^-7e  
//   0.1...11 2^-7e                                     (biased, 1)
//    largest dn                               smallest normal

// If the answer is = 0, we have underflowed.
// Then p6 is TRUE. Set the underflow tag, save input in FR_X,
// do the final calculation for IEEE result, and branch to error return.

{ .mfi
       nop.m 999
       fsetc.s2 0x7F,0x41
       nop.i 999
}
;;

{ .mfi
       nop.m 999
       fma.s.s2     exp_ftz_urm_f8  = exp_T,  exp_P6, exp_A
       nop.i 999
}
;;


{ .mfi
       nop.m 999
       fsetc.s2 0x7F,0x40
       nop.i 999
}
;;

{ .mfi
       nop.m 999
       fcmp.eq.unc.s1 p6, p0     =  exp_ftz_urm_f8, f0
       nop.i 999
}
;;

{ .mfb
       nop.m 999
       nop.f 999
(p6)   br.cond.spnt L(EXP_CERTAIN_UNDERFLOW)  // Branch if really underflow 
}
;;

{ .mfb
       nop.m 999
       fma.s        f8             = exp_T,  exp_P6, exp_A
       br.ret.sptk     b0                  // Exit if really no underflow
}
;;

L(EXP_CERTAIN_UNDERFLOW):
{ .mfi
      nop.m 999
      fmerge.s FR_X = f8,f8
      nop.i 999
}
{ .mfb
      mov        GR_Parameter_TAG = 17
      fma.s       FR_RESULT  = exp_T, exp_P6, exp_A // Set I,U and tiny result
      br.cond.sptk  __libm_error_region ;;                             
}

L(EXP_CERTAIN_UNDERFLOW_ZERO):
{ .mmi
      mov   exp_GR_one = 1 ;;
      setf.exp     f9 = exp_GR_one
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
      fmerge.s FR_X = f8,f8
      nop.i 999
}
{ .mfb
      mov        GR_Parameter_TAG = 17
      fma.s       FR_RESULT = f9, f9, f0    // Set I,U and tiny (+0.0) result
      br.cond.sptk  __libm_error_region ;;                             
}

.endp expf
ASM_SIZE_DIRECTIVE(expf)


.proc __libm_error_region
__libm_error_region:
.prologue
{ .mfi
        add   GR_Parameter_Y=-32,sp             // Parameter 2 value
	nop.f 999
.save   ar.pfs,GR_SAVE_PFS
        mov  GR_SAVE_PFS=ar.pfs                 // Save ar.pfs
}
{ .mfi
.fframe 64
        add sp=-64,sp                           // Create new stack
        nop.f 0
        mov GR_SAVE_GP=gp                       // Save gp
};;
{ .mmi
        stfs [GR_Parameter_Y] = FR_Y,16         // Store Parameter 2 on stack
        add GR_Parameter_X = 16,sp              // Parameter 1 address
.save   b0, GR_SAVE_B0
        mov GR_SAVE_B0=b0                       // Save b0
};;
.body
{ .mfi
        stfs [GR_Parameter_X] = FR_X            // Store Parameter 1 on stack
        nop.f 0
        add   GR_Parameter_RESULT = 0,GR_Parameter_Y // Parameter 3 address
}
{ .mib
        stfs [GR_Parameter_Y] = FR_RESULT       // Store Parameter 3 on stack
        add   GR_Parameter_Y = -16,GR_Parameter_Y
        br.call.sptk b0=__libm_error_support#   // Call error handling function
};;

{ .mmi
        nop.m 0
        nop.m 0
        add   GR_Parameter_RESULT = 48,sp
};;

{ .mmi
        ldfs  f8 = [GR_Parameter_RESULT]       // Get return result off stack
.restore sp
        add   sp = 64,sp                       // Restore stack pointer
        mov   b0 = GR_SAVE_B0                  // Restore return address
};;
{ .mib
        mov   gp = GR_SAVE_GP                  // Restore gp 
        mov   ar.pfs = GR_SAVE_PFS             // Restore ar.pfs
        br.ret.sptk     b0                     // Return
};; 

.endp __libm_error_region
ASM_SIZE_DIRECTIVE(__libm_error_region)


.type   __libm_error_support#,@function
.global __libm_error_support#
