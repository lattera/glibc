.file "exp.s"

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
//
// History
//==============================================================
// 2/02/00  Initial version 
// 3/07/00  exp(inf)  = inf but now does NOT call error support
//          exp(-inf) = 0   but now does NOT call error support
// 4/04/00  Unwind support added
// 8/15/00  Bundle added after call to __libm_error_support to properly
//          set [the previously overwritten] GR_Parameter_RESULT.
// 11/30/00 Reworked to shorten main path, widen main path to include all
//          args in normal range, and add quick exit for 0, nan, inf.
// 12/05/00 Loaded constants earlier with setf to save 2 cycles.

// API
//==============================================================
// double exp(double)

// Overview of operation
//==============================================================
// Take the input x. w is "how many log2/128 in x?"
//  w = x * 128/log2
//  n = int(w)
//  x = n log2/128 + r + delta

//  n = 128M + index_1 + 2^4 index_2
//  x = M log2 + (log2/128) index_1 + (log2/8) index_2 + r + delta

//  exp(x) = 2^M  2^(index_1/128)  2^(index_2/8) exp(r) exp(delta)
//       Construct 2^M
//       Get 2^(index_1/128) from table_1;
//       Get 2^(index_2/8)   from table_2;
//       Calculate exp(r) by series
//          r = x - n (log2/128)_high
//          delta = - n (log2/128)_low
//       Calculate exp(delta) as 1 + delta


// Special values 
//==============================================================
// exp(+0)    = 1.0
// exp(-0)    = 1.0

// exp(+qnan) = +qnan 
// exp(-qnan) = -qnan 
// exp(+snan) = +qnan 
// exp(-snan) = -qnan 

// exp(-inf)  = +0 
// exp(+inf)  = +inf

// Overfow and Underfow
//=======================
// exp(-x) = smallest double normal when
//     x = -708.396 = c086232bdd7abcd2

// exp(x) = largest double normal when
//     x = 709.7827 = 40862e42fefa39ef



// Registers used
//==============================================================
// Floating Point registers used: 
// f8, input
// f9 -> f15,  f32 -> f60

// General registers used: 
// r32 -> r60 

// Predicate registers used:
// p6 -> p15

#include "libm_support.h"

// Assembly macros
//==============================================================

exp_GR_rshf                   = r33
EXP_AD_TB1                    = r34
EXP_AD_TB2                    = r35
EXP_AD_P                      = r36

exp_GR_N                      = r37
exp_GR_index_1                = r38
exp_GR_index_2_16             = r39

exp_GR_biased_M               = r40
exp_GR_index_1_16             = r41
EXP_AD_T1                     = r42
EXP_AD_T2                     = r43
exp_GR_sig_inv_ln2            = r44

exp_GR_17ones                 = r45
exp_GR_one                    = r46
exp_TB1_size                  = r47
exp_TB2_size                  = r48
exp_GR_rshf_2to56             = r49

exp_GR_gt_ln                  = r50
exp_GR_exp_2tom56             = r51

exp_GR_17ones_m1              = r52

GR_SAVE_B0                    = r53
GR_SAVE_PFS                   = r54
GR_SAVE_GP                    = r55
GR_SAVE_SP                    = r56

GR_Parameter_X                = r57
GR_Parameter_Y                = r58
GR_Parameter_RESULT           = r59
GR_Parameter_TAG              = r60


FR_X             = f10
FR_Y             = f1
FR_RESULT        = f8

EXP_RSHF_2TO56   = f6
EXP_INV_LN2_2TO63 = f7
EXP_W_2TO56_RSH  = f9
EXP_2TOM56       = f11
exp_P4           = f12 
exp_P3           = f13 
exp_P2           = f14 
exp_P1           = f15 

exp_ln2_by_128_hi  = f33 
exp_ln2_by_128_lo  = f34 

EXP_RSHF           = f35
EXP_Nfloat         = f36 
exp_W              = f37
exp_r              = f38
exp_f              = f39

exp_rsq            = f40
exp_rcube          = f41

EXP_2M             = f42
exp_S1             = f43
exp_T1             = f44

EXP_MIN_DBL_OFLOW_ARG = f45
EXP_MAX_DBL_ZERO_ARG  = f46
EXP_MAX_DBL_NORM_ARG  = f47
EXP_MAX_DBL_UFLOW_ARG = f48
EXP_MIN_DBL_NORM_ARG  = f49
exp_rP4pP3         = f50
exp_P_lo           = f51
exp_P_hi           = f52
exp_P              = f53
exp_S              = f54

EXP_NORM_f8        = f56   

exp_wre_urm_f8     = f57
exp_ftz_urm_f8     = f57

exp_gt_pln         = f58

exp_S2             = f59
exp_T2             = f60


// Data tables
//==============================================================

#ifdef _LIBC
.rodata
#else
.data
#endif

.align 16

// ************* DO NOT CHANGE ORDER OF THESE TABLES ********************

// double-extended 1/ln(2)
// 3fff b8aa 3b29 5c17 f0bb be87fed0691d3e88
// 3fff b8aa 3b29 5c17 f0bc 
// For speed the significand will be loaded directly with a movl and setf.sig
//   and the exponent will be bias+63 instead of bias+0.  Thus subsequent
//   computations need to scale appropriately.
// The constant 128/ln(2) is needed for the computation of w.  This is also 
//   obtained by scaling the computations.
//
// Two shifting constants are loaded directly with movl and setf.d. 
//   1. EXP_RSHF_2TO56 = 1.1000..00 * 2^(63-7) 
//        This constant is added to x*1/ln2 to shift the integer part of
//        x*128/ln2 into the rightmost bits of the significand.
//        The result of this fma is EXP_W_2TO56_RSH.
//   2. EXP_RSHF       = 1.1000..00 * 2^(63) 
//        This constant is subtracted from EXP_W_2TO56_RSH * 2^(-56) to give
//        the integer part of w, n, as a floating-point number.
//        The result of this fms is EXP_Nfloat.


exp_table_1:
ASM_TYPE_DIRECTIVE(exp_table_1,@object)
data8 0x40862e42fefa39f0 // smallest dbl overflow arg
data8 0xc0874c0000000000 // approx largest arg for zero result
data8 0x40862e42fefa39ef // largest dbl arg to give normal dbl result
data8 0xc086232bdd7abcd3 // largest dbl underflow arg
data8 0xc086232bdd7abcd2 // smallest dbl arg to give normal dbl result
data8 0x0                // pad
data8 0xb17217f7d1cf79ab , 0x00003ff7 // ln2/128 hi
data8 0xc9e3b39803f2f6af , 0x00003fb7 // ln2/128 lo

// Table 1 is 2^(index_1/128) where
// index_1 goes from 0 to 15

data8 0x8000000000000000 , 0x00003FFF
data8 0x80B1ED4FD999AB6C , 0x00003FFF
data8 0x8164D1F3BC030773 , 0x00003FFF
data8 0x8218AF4373FC25EC , 0x00003FFF
data8 0x82CD8698AC2BA1D7 , 0x00003FFF
data8 0x8383594EEFB6EE37 , 0x00003FFF
data8 0x843A28C3ACDE4046 , 0x00003FFF
data8 0x84F1F656379C1A29 , 0x00003FFF
data8 0x85AAC367CC487B15 , 0x00003FFF
data8 0x8664915B923FBA04 , 0x00003FFF
data8 0x871F61969E8D1010 , 0x00003FFF
data8 0x87DB357FF698D792 , 0x00003FFF
data8 0x88980E8092DA8527 , 0x00003FFF
data8 0x8955EE03618E5FDD , 0x00003FFF
data8 0x8A14D575496EFD9A , 0x00003FFF
data8 0x8AD4C6452C728924 , 0x00003FFF
ASM_SIZE_DIRECTIVE(exp_table_1)

// Table 2 is 2^(index_1/8) where
// index_2 goes from 0 to 7
exp_table_2:
ASM_TYPE_DIRECTIVE(exp_table_2,@object)
data8 0x8000000000000000 , 0x00003FFF
data8 0x8B95C1E3EA8BD6E7 , 0x00003FFF
data8 0x9837F0518DB8A96F , 0x00003FFF
data8 0xA5FED6A9B15138EA , 0x00003FFF
data8 0xB504F333F9DE6484 , 0x00003FFF
data8 0xC5672A115506DADD , 0x00003FFF
data8 0xD744FCCAD69D6AF4 , 0x00003FFF
data8 0xEAC0C6E7DD24392F , 0x00003FFF
ASM_SIZE_DIRECTIVE (exp_table_2)


exp_p_table:
ASM_TYPE_DIRECTIVE(exp_p_table,@object)
data8 0x3f8111116da21757 //P_4
data8 0x3fa55555d787761c //P_3
data8 0x3fc5555555555414 //P_2
data8 0x3fdffffffffffd6a //P_1
ASM_SIZE_DIRECTIVE(exp_p_table)


.align 32
.global exp#

.section .text
.proc  exp#
.align 32
exp: 
#ifdef _LIBC
.global __ieee754_exp#
__ieee754_exp:
#endif

{ .mlx
      alloc      r32=ar.pfs,1,24,4,0                               
      movl exp_GR_sig_inv_ln2 = 0xb8aa3b295c17f0bc  // significand of 1/ln2
}
{ .mlx
      addl       EXP_AD_TB1    = @ltoff(exp_table_1), gp
      movl exp_GR_rshf_2to56 = 0x4768000000000000 ;;  // 1.10000 2^(63+56)
}
;;

// We do this fnorm right at the beginning to take any enabled
// faults and to normalize any input unnormals so that SWA is not taken.
{ .mfi
      ld8        EXP_AD_TB1    = [EXP_AD_TB1]
      fclass.m   p8,p0 = f8,0x07  // Test for x=0
      mov        exp_GR_17ones = 0x1FFFF                          
}
{ .mfi
      mov        exp_TB1_size  = 0x100
      fnorm      EXP_NORM_f8   = f8                                          
      mov exp_GR_exp_2tom56 = 0xffff-56
}
;;

// Form two constants we need
//  1/ln2 * 2^63  to compute  w = x * 1/ln2 * 128 
//  1.1000..000 * 2^(63+63-7) to right shift int(w) into the significand

{ .mmf
      setf.sig  EXP_INV_LN2_2TO63 = exp_GR_sig_inv_ln2 // form 1/ln2 * 2^63
      setf.d  EXP_RSHF_2TO56 = exp_GR_rshf_2to56 // Form const 1.100 * 2^(63+56)
      fclass.m   p9,p0 = f8,0x22  // Test for x=-inf
}
;;

{ .mlx
      setf.exp EXP_2TOM56 = exp_GR_exp_2tom56 // form 2^-56 for scaling Nfloat
      movl exp_GR_rshf = 0x43e8000000000000   // 1.10000 2^63 for right shift
}
{ .mfb
      mov        exp_TB2_size  = 0x80
(p8)  fma.d      f8 = f1,f1,f0           // quick exit for x=0
(p8)  br.ret.spnt b0
;;
}

{ .mfi
      ldfpd      EXP_MIN_DBL_OFLOW_ARG, EXP_MAX_DBL_ZERO_ARG = [EXP_AD_TB1],16
      fclass.m   p10,p0 = f8,0x21  // Test for x=+inf
      nop.i 999
}
{ .mfb
      nop.m 999
(p9)  fma.d      f8 = f0,f0,f0           // quick exit for x=-inf
(p9)  br.ret.spnt b0
;;                    
}

{ .mmf
      ldfpd      EXP_MAX_DBL_NORM_ARG, EXP_MAX_DBL_UFLOW_ARG = [EXP_AD_TB1],16
      setf.d  EXP_RSHF = exp_GR_rshf // Form right shift const 1.100 * 2^63
      fclass.m   p11,p0 = f8,0xc3  // Test for x=nan
;;
}

{ .mfb
      ldfd      EXP_MIN_DBL_NORM_ARG = [EXP_AD_TB1],16
      nop.f 999
(p10) br.ret.spnt b0               // quick exit for x=+inf
;;
}

{ .mfi
      ldfe       exp_ln2_by_128_hi  = [EXP_AD_TB1],16
      nop.f 999
      nop.i 999
;;
}


{ .mfb
      ldfe       exp_ln2_by_128_lo  = [EXP_AD_TB1],16
(p11) fmerge.s   f8 = EXP_NORM_f8, EXP_NORM_f8
(p11) br.ret.spnt b0               // quick exit for x=nan
;;
}

// After that last load, EXP_AD_TB1 points to the beginning of table 1

// W = X * Inv_log2_by_128
// By adding 1.10...0*2^63 we shift and get round_int(W) in significand.
// We actually add 1.10...0*2^56 to X * Inv_log2 to do the same thing.

{ .mfi
      nop.m 999
      fma.s1  EXP_W_2TO56_RSH  = EXP_NORM_f8, EXP_INV_LN2_2TO63, EXP_RSHF_2TO56
      nop.i 999
;;
}


// Divide arguments into the following categories:
//  Certain Underflow/zero  p11 - -inf < x <= MAX_DBL_ZERO_ARG 
//  Certain Underflow       p12 - MAX_DBL_ZERO_ARG < x <= MAX_DBL_UFLOW_ARG 
//  Possible Underflow      p13 - MAX_DBL_UFLOW_ARG < x < MIN_DBL_NORM_ARG
//  Certain Safe                - MIN_DBL_NORM_ARG <= x <= MAX_DBL_NORM_ARG
//  Possible Overflow       p14 - MAX_DBL_NORM_ARG < x < MIN_DBL_OFLOW_ARG
//  Certain Overflow        p15 - MIN_DBL_OFLOW_ARG <= x < +inf
//
// If the input is really a double arg, then there will never be "Possible
// Underflow" or "Possible Overflow" arguments.
//

{ .mfi
      add        EXP_AD_TB2 = exp_TB1_size, EXP_AD_TB1
      fcmp.ge.s1  p15,p14 = EXP_NORM_f8,EXP_MIN_DBL_OFLOW_ARG
      nop.i 999
;;                        
}

{ .mfi
      add        EXP_AD_P = exp_TB2_size, EXP_AD_TB2
      fcmp.le.s1  p11,p12 = EXP_NORM_f8,EXP_MAX_DBL_ZERO_ARG
      nop.i 999
;;
}

{ .mfb
      ldfpd      exp_P4, exp_P3  = [EXP_AD_P] ,16
(p14) fcmp.gt.unc.s1  p14,p0 = EXP_NORM_f8,EXP_MAX_DBL_NORM_ARG
(p15) br.cond.spnt L(EXP_CERTAIN_OVERFLOW)
;;
}


// Nfloat = round_int(W) 
// The signficand of EXP_W_2TO56_RSH contains the rounded integer part of W,
// as a twos complement number in the lower bits (that is, it may be negative).
// That twos complement number (called N) is put into exp_GR_N.

// Since EXP_W_2TO56_RSH is scaled by 2^56, it must be multiplied by 2^-56
// before the shift constant 1.10000 * 2^63 is subtracted to yield EXP_Nfloat.
// Thus, EXP_Nfloat contains the floating point version of N


{ .mfi
      nop.m 999
(p12) fcmp.le.unc  p12,p0 = EXP_NORM_f8,EXP_MAX_DBL_UFLOW_ARG
      nop.i 999
}
{ .mfb
      ldfpd      exp_P2, exp_P1  = [EXP_AD_P]                                  
      fms.s1          EXP_Nfloat = EXP_W_2TO56_RSH, EXP_2TOM56, EXP_RSHF 
(p11) br.cond.spnt L(EXP_CERTAIN_UNDERFLOW_ZERO)
;;
}

{ .mfi
      getf.sig        exp_GR_N        = EXP_W_2TO56_RSH
(p13) fcmp.lt.unc  p13,p0 = EXP_NORM_f8,EXP_MIN_DBL_NORM_ARG
      nop.i 999
;;
}


// exp_GR_index_1 has index_1
// exp_GR_index_2_16 has index_2 * 16
// exp_GR_biased_M has M
// exp_GR_index_1_16 has index_1 * 16

// r2 has true M
{ .mfi
      and            exp_GR_index_1 = 0x0f, exp_GR_N
      fnma.s1    exp_r   = EXP_Nfloat, exp_ln2_by_128_hi, EXP_NORM_f8 
      shr            r2 = exp_GR_N,  0x7
}
{ .mfi
      and            exp_GR_index_2_16 = 0x70, exp_GR_N
      fnma.s1    exp_f   = EXP_Nfloat, exp_ln2_by_128_lo, f1 
      nop.i 999
;;                            
}


// EXP_AD_T1 has address of T1                           
// EXP_AD_T2 has address if T2                            

{ .mmi
      addl           exp_GR_biased_M = 0xffff, r2 
      add            EXP_AD_T2 = EXP_AD_TB2, exp_GR_index_2_16 
      shladd         EXP_AD_T1 = exp_GR_index_1, 4, EXP_AD_TB1
;;                            
}


// Create Scale = 2^M
// r = x - Nfloat * ln2_by_128_hi 
// f = 1 - Nfloat * ln2_by_128_lo 

{ .mmi
      setf.exp        EXP_2M = exp_GR_biased_M                              
      ldfe       exp_T2  = [EXP_AD_T2]                                
      nop.i 999
;;
}

// Load T1 and T2
{ .mfi
      ldfe       exp_T1  = [EXP_AD_T1]                                
      nop.f 999
      nop.i 999
;;
}


{ .mfi
        nop.m 999
        fma.s1           exp_rsq = exp_r, exp_r, f0 
        nop.i 999
}
{ .mfi
        nop.m 999
        fma.s1        exp_rP4pP3 = exp_r, exp_P4, exp_P3               
        nop.i 999
;;
}



{ .mfi
        nop.m 999
        fma.s1           exp_rcube = exp_r, exp_rsq, f0 
        nop.i 999 
}
{ .mfi
        nop.m 999
        fma.s1        exp_P_lo  = exp_r, exp_rP4pP3, exp_P2            
        nop.i 999
;;
}


{ .mfi
        nop.m 999
        fma.s1        exp_P_hi  = exp_rsq, exp_P1, exp_r              
        nop.i 999
}
{ .mfi
        nop.m 999
        fma.s1        exp_S2  = exp_f,exp_T2,f0                       
        nop.i 999
;;
}

{ .mfi
        nop.m 999
        fma.s1        exp_S1  = EXP_2M,exp_T1,f0                      
        nop.i 999
;;
}


{ .mfi
        nop.m 999
        fma.s1        exp_P     = exp_rcube, exp_P_lo, exp_P_hi       
        nop.i 999
;;
}

{ .mfi
        nop.m 999
        fma.s1        exp_S   = exp_S1,exp_S2,f0                      
        nop.i 999
;;
}

{ .bbb
(p12)   br.cond.spnt  L(EXP_CERTAIN_UNDERFLOW)
(p13)   br.cond.spnt  L(EXP_POSSIBLE_UNDERFLOW)
(p14)   br.cond.spnt  L(EXP_POSSIBLE_OVERFLOW)
;;
}


{ .mfb
        nop.m 999
        fma.d      f8 = exp_S, exp_P, exp_S 
        br.ret.sptk     b0 ;;               // Normal path exit 
}


L(EXP_POSSIBLE_OVERFLOW): 

// We got an answer. EXP_MAX_DBL_NORM_ARG < x < EXP_MIN_DBL_OFLOW_ARG
// overflow is a possibility, not a certainty

{ .mfi
	nop.m 999
        fsetc.s2 0x7F,0x42                                          
	nop.i 999 ;;
}

{ .mfi
	nop.m 999
        fma.d.s2      exp_wre_urm_f8 = exp_S, exp_P, exp_S          
	nop.i 999 ;;
}

// We define an overflow when the answer with
//    WRE set
//    user-defined rounding mode
// is ldn +1

// Is the exponent 1 more than the largest double?
// If so, go to ERROR RETURN, else get the answer and 
// leave.

// Largest double is 7FE (biased double)
//                   7FE - 3FF + FFFF = 103FE
// Create + largest_double_plus_ulp
// Create - largest_double_plus_ulp
// Calculate answer with WRE set.

// Cases when answer is ldn+1  are as follows:
//  ldn                   ldn+1
// --+----------|----------+------------
//              | 
//    +inf          +inf      -inf
//                  RN         RN
//                             RZ 

{ .mfi
	nop.m 999
        fsetc.s2 0x7F,0x40                                          
        mov           exp_GR_gt_ln  = 0x103ff ;;                      
}

{ .mfi
        setf.exp      exp_gt_pln    = exp_GR_gt_ln                 
	nop.f 999
	nop.i 999 ;;
}

{ .mfi
	nop.m 999
       fcmp.ge.unc.s1 p6, p0 =  exp_wre_urm_f8, exp_gt_pln 	  
	nop.i 999 ;;
}

{ .mfb
	nop.m 999
	nop.f 999
(p6)   br.cond.spnt L(EXP_CERTAIN_OVERFLOW) ;; // Branch if really overflow
}

{ .mfb
	nop.m 999
       fma.d        f8 = exp_S, exp_P, exp_S                      
       br.ret.sptk     b0 ;;             // Exit if really no overflow
}

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
      mov        GR_Parameter_TAG = 14
      fma.d       FR_RESULT = f9, f9, f0    // Set I,O and +INF result
      br.cond.sptk  __libm_error_region ;;                             
}

L(EXP_POSSIBLE_UNDERFLOW): 

// We got an answer. EXP_MAX_DBL_UFLOW_ARG < x < EXP_MIN_DBL_NORM_ARG
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
//   1.1...10 2^-3fff    1.1...11 2^-3fff    1.0...00 2^-3ffe
//   0.1...11 2^-3ffe                                   (biased, 1)
//    largest dn                               smallest normal

{ .mfi
	nop.m 999
       fsetc.s2 0x7F,0x41                                          
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
       fma.d.s2      exp_ftz_urm_f8 = exp_S, exp_P, exp_S          
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
       fsetc.s2 0x7F,0x40                                          
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
       fcmp.eq.unc.s1 p6, p0 =  exp_ftz_urm_f8, f0 	          
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
	nop.f 999
(p6)   br.cond.spnt L(EXP_CERTAIN_UNDERFLOW) ;; // Branch if really underflow
}
{ .mfb
	nop.m 999
       fma.d        f8 = exp_S, exp_P, exp_S                      
       br.ret.sptk     b0 ;;                // Exit if really no underflow
}

L(EXP_CERTAIN_UNDERFLOW):
{ .mfi
      nop.m 999
      fmerge.s FR_X = f8,f8
      nop.i 999
}
{ .mfb
      mov        GR_Parameter_TAG = 15
      fma.d       FR_RESULT  = exp_S, exp_P, exp_S // Set I,U and tiny result
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
      mov        GR_Parameter_TAG = 15
      fma.d       FR_RESULT = f9, f9, f0    // Set I,U and tiny (+0.0) result
      br.cond.sptk  __libm_error_region ;;                             
}

.endp exp
ASM_SIZE_DIRECTIVE(exp)


.proc __libm_error_region
__libm_error_region:
.prologue
{ .mfi
        add   GR_Parameter_Y=-32,sp             // Parameter 2 value
        nop.f 0
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
        stfd [GR_Parameter_Y] = FR_Y,16         // STORE Parameter 2 on stack
        add GR_Parameter_X = 16,sp              // Parameter 1 address
.save   b0, GR_SAVE_B0                      
        mov GR_SAVE_B0=b0                       // Save b0 
};;
.body
{ .mib
        stfd [GR_Parameter_X] = FR_X                  // STORE Parameter 1 on stack 
        add   GR_Parameter_RESULT = 0,GR_Parameter_Y  // Parameter 3 address 
	nop.b 0                                      
}
{ .mib
        stfd [GR_Parameter_Y] = FR_RESULT             // STORE Parameter 3 on stack
        add   GR_Parameter_Y = -16,GR_Parameter_Y  
        br.call.sptk b0=__libm_error_support#         // Call error handling function
};;
{ .mmi
        nop.m 0
        nop.m 0
        add   GR_Parameter_RESULT = 48,sp
};;
{ .mmi
        ldfd  f8 = [GR_Parameter_RESULT]       // Get return result off stack
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
