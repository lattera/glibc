.file "nextafterl.s"


// Copyright (c) 2000 - 2003, Intel Corporation
// All rights reserved.
//
// Contributed 2000 by the Intel Numerics Group, Intel Corporation
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// * The name of Intel Corporation may not be used to endorse or promote
// products derived from this software without specific prior written
// permission.

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
// http://www.intel.com/software/products/opensource/libraries/num.htm.
//
// History
//==============================================================
// 02/02/00 Initial version 
// 03/03/00 Modified to conform to C9X, and improve speed of main path
// 03/14/00 Fixed case where x is a power of 2, and x > y, improved speed
// 04/04/00 Unwind support added
// 05/12/00 Fixed erroneous denormal flag setting for exponent change cases 1,3
// 08/15/00 Bundle added after call to __libm_error_support to properly
//          set [the previously overwritten] GR_Parameter_RESULT.
// 09/09/00 Updated fcmp so that qnans do not raise invalid.
// 12/15/00 Fixed case of smallest long double normal to largest denormal,
//          now adhere to C99 for two zero args, and fixed flag settings 
//          for several cases
// 05/20/02 Cleaned up namespace and sf0 syntax
// 02/10/03 Reordered header: .section, .global, .proc, .align
//
// API
//==============================================================
// long double nextafterl( long double x, long double y );
// input  floating point f8, f9
// output floating point f8
//
// Registers used
//==============================================================
nextafter_GR_max_pexp     = r14
nextafter_GR_min_pexp     = r15
nextafter_GR_exp          = r16
nextafter_GR_sig          = r17
nextafter_GR_lnorm_sig    = r18
nextafter_GR_sign_mask    = r19
nextafter_GR_exp_mask     = r20
nextafter_GR_sden_sig     = r21
nextafter_GR_new_sig      = r22
nextafter_GR_new_exp      = r23
nextafter_GR_lden_sig     = r24
nextafter_GR_snorm_sig    = r25
nextafter_GR_exp1         = r26
nextafter_GR_x_exp        = r27
// r36-39 parameters for libm_error_support

GR_SAVE_B0                = r34
GR_SAVE_GP                = r35
GR_SAVE_PFS               = r32

GR_Parameter_X            = r36
GR_Parameter_Y            = r37
GR_Parameter_RESULT       = r38

NEXTAFTER_lnorm_sig       = f10
NEXTAFTER_lnorm_exp       = f11
NEXTAFTER_lnorm           = f12
NEXTAFTER_sden_sig        = f13
NEXTAFTER_den_exp         = f14
NEXTAFTER_sden            = f15
NEXTAFTER_snorm_exp       = f32
NEXTAFTER_save_f8         = f33
NEXTAFTER_new_exp         = f34
NEXTAFTER_new_sig         = f35
NEXTAFTER_lden_sig        = f36
NEXTAFTER_snorm_sig       = f37
NEXTAFTER_exp1            = f38
NEXTAFTER_tmp             = f39

//
// Overview of operation
//==============================================================
// nextafterl determines the next representable value 
// after x in the direction of y. 


.section .text
GLOBAL_LIBM_ENTRY(nextafterl)

// Extract signexp from x
// Is x < y ?  p10 if yes, p11 if no
// Form smallest denormal significand = ulp size
{ .mfi
      getf.exp nextafter_GR_exp      = f8
      fcmp.lt.s1 p10,p11 = f8, f9                
      addl nextafter_GR_sden_sig = 0x1, r0
}
// Form largest normal significand 0xffffffffffffffff
// Form smallest normal exponent
{ .mfi
      addl nextafter_GR_lnorm_sig = -0x1,r0
      nop.f 999
      addl nextafter_GR_min_pexp = 0x0c001, r0 ;;
}

// Extract significand from x
// Is x=y?   This fcmp also sets Invalid and Denormal if required
// Form largest normal exponent
{ .mfi
      getf.sig nextafter_GR_sig      = f8
      fcmp.eq.s0 p6,p0 = f8, f9
      addl nextafter_GR_max_pexp = 0x13ffe, r0
}
// Move largest normal significand to fp reg for special cases
{ .mfi
      setf.sig NEXTAFTER_lnorm_sig = nextafter_GR_lnorm_sig
      nop.f 999
      addl nextafter_GR_sign_mask = 0x20000, r0 ;;
}

// Move smallest denormal significand and exp to fp regs
// Is x=nan?
// Set p12 and p13 based on whether significand increases or decreases
// It increases (p12 set) if x<y and x>=0 or if x>y and x<0
// It decreases (p13 set) if x<y and x<0  or if x>y and x>=0
{ .mfi
      setf.sig NEXTAFTER_sden_sig = nextafter_GR_sden_sig
      fclass.m  p8,p0 = f8, 0xc3           
(p10) cmp.lt p12,p13 = nextafter_GR_exp, nextafter_GR_sign_mask
}
// Move smallest normal exp to fp regs
{ .mfi
      setf.exp NEXTAFTER_snorm_exp = nextafter_GR_min_pexp
      nop.f 999
(p11) cmp.ge p12,p13 = nextafter_GR_exp, nextafter_GR_sign_mask ;;
}

.pred.rel "mutex",p12,p13

// Form expected new significand, adding or subtracting 1 ulp increment
// If x=y set result to y
// Form smallest normal significand and largest denormal significand
{ .mfi
(p12) add nextafter_GR_new_sig = nextafter_GR_sig, nextafter_GR_sden_sig
(p6)  fmerge.s f8=f9,f9
      dep.z nextafter_GR_snorm_sig = 1,63,1 // 0x8000000000000000
}
{ .mlx
(p13) sub nextafter_GR_new_sig = nextafter_GR_sig, nextafter_GR_sden_sig
      movl nextafter_GR_lden_sig = 0x7fffffffffffffff ;;
}

// Move expected result significand and signexp to fp regs
// Is y=nan?
// Form new exponent in case result exponent needs incrementing or decrementing
{ .mfi
      setf.exp NEXTAFTER_new_exp = nextafter_GR_exp
      fclass.m  p9,p0 = f9, 0xc3           
(p12) add nextafter_GR_exp1 = 1, nextafter_GR_exp
}
{ .mib
      setf.sig NEXTAFTER_new_sig = nextafter_GR_new_sig
(p13) add nextafter_GR_exp1 = -1, nextafter_GR_exp
(p6)  br.ret.spnt    b0 ;;             // Exit if x=y
}

// Move largest normal signexp to fp reg for special cases
// Is x=zero?
{ .mfi
      setf.exp NEXTAFTER_lnorm_exp = nextafter_GR_max_pexp
      fclass.m  p7,p0 = f8, 0x7
      nop.i 999
}
{ .mfb
      setf.exp NEXTAFTER_den_exp = nextafter_GR_min_pexp
(p8)  fma.s0 f8 = f8,f1,f9                     
(p8)  br.ret.spnt    b0 ;;             // Exit if x=nan
}

// Move exp+-1 and smallest normal significand to fp regs for special cases
// Is x=inf?
{ .mfi
      setf.exp NEXTAFTER_exp1 = nextafter_GR_exp1
      fclass.m  p6,p0 = f8, 0x23           
      addl nextafter_GR_exp_mask = 0x1ffff, r0
}
{ .mfb
      setf.sig NEXTAFTER_snorm_sig = nextafter_GR_snorm_sig
(p9)  fma.s0 f8 = f8,f1,f9                     
(p9)  br.ret.spnt    b0 ;;             // Exit if y=nan
}

// Move largest denormal significand to fp regs for special cases
// Save x
{ .mfb
      setf.sig NEXTAFTER_lden_sig = nextafter_GR_lden_sig
      mov NEXTAFTER_save_f8 = f8
(p7)  br.cond.spnt NEXTAFTER_ZERO ;;   // Exit if x=0   
}

// Mask off the sign to get x_exp
{ .mfb
      and nextafter_GR_x_exp = nextafter_GR_exp_mask, nextafter_GR_exp
      nop.f 999
(p6)  br.cond.spnt NEXTAFTER_INF ;;   // Exit if x=inf   
}

// Check 5 special cases when significand rolls over:
//  1 sig size incr, x_sig=max_sig, x_exp < max_exp
//     Set p6, result is sig=min_sig, exp++
//  2 sig size incr, x_sig=max_sig, x_exp >= max_exp
//     Set p7, result is inf, signal overflow
//  3 sig size decr, x_sig=min_sig, x_exp > min_exp
//     Set p8, result is sig=max_sig, exp--
//  4 sig size decr, x_sig=min_sig, x_exp = min_exp
//     Set p9, result is sig=max_den_sig, exp same, signal underflow and inexact
//  5 sig size decr, x_sig=min_den_sig, x_exp = min_exp
//     Set p10, result is zero, sign of x, signal underflow and inexact
//
{ .mmi
(p12) cmp.eq.unc p6,p0 = nextafter_GR_new_sig, r0
(p13) cmp.eq.unc p9,p10 = nextafter_GR_new_sig, nextafter_GR_lden_sig
      nop.i 999
;;
}

{ .mmi
(p6)  cmp.lt.unc p6,p7 = nextafter_GR_x_exp, nextafter_GR_max_pexp
(p10) cmp.eq.unc p10,p0 = nextafter_GR_new_sig, r0
(p9)  cmp.le.unc p9,p8 = nextafter_GR_x_exp, nextafter_GR_min_pexp 
;;
}

// Create small normal in case need to generate underflow flag
{ .mfi
      nop.m 999
      fmerge.se NEXTAFTER_tmp = NEXTAFTER_snorm_exp, NEXTAFTER_lnorm_sig
      nop.i 999
}
// Branch if cases 1, 2, 3
{ .bbb
(p6)  br.cond.spnt NEXTAFTER_EXPUP
(p7)  br.cond.spnt NEXTAFTER_OVERFLOW
(p8)  br.cond.spnt NEXTAFTER_EXPDOWN ;;
}

// Branch if cases 4, 5
{ .mbb
      nop.m 999
(p9)  br.cond.spnt NEXTAFTER_NORM_TO_DENORM
(p10) br.cond.spnt NEXTAFTER_UNDERFLOW_TO_ZERO
;;
}

// Here if no special cases
// Set p6 if result will be a denormal, so can force underflow flag
//    Case 1:  x_exp=min_exp, x_sig=unnormalized
//    Case 2:  x_exp<min_exp
{ .mfi
      cmp.lt p6,p7 = nextafter_GR_x_exp, nextafter_GR_min_pexp
      fmerge.se f8 = NEXTAFTER_new_exp, NEXTAFTER_new_sig
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
      nop.f 999
(p6)  tbit.z p6,p0 = nextafter_GR_new_sig, 63 ;;
}

NEXTAFTER_COMMON_FINISH:
// Force underflow and inexact if denormal result
{ .mfi
      nop.m 999
(p6)  fma.s0 NEXTAFTER_tmp = NEXTAFTER_tmp,NEXTAFTER_tmp,f0
      nop.i 999 ;;
}

// Final normalization to result precision and exit
{ .mfb
      nop.m 999
      fnorm.s0 f8 = f8
      br.ret.sptk b0;;
}

//Special cases
NEXTAFTER_EXPUP:
{ .mfb
      cmp.lt p6,p7 = nextafter_GR_x_exp, nextafter_GR_min_pexp
      fmerge.se f8 = NEXTAFTER_exp1, NEXTAFTER_snorm_sig
      br.cond.sptk NEXTAFTER_COMMON_FINISH ;;
}

NEXTAFTER_EXPDOWN:
{ .mfb
      cmp.lt p6,p7 = nextafter_GR_x_exp, nextafter_GR_min_pexp
      fmerge.se f8 = NEXTAFTER_exp1, NEXTAFTER_lnorm_sig
      br.cond.sptk NEXTAFTER_COMMON_FINISH ;;
}

NEXTAFTER_NORM_TO_DENORM:
{ .mfi
      nop.m 999
      fmerge.se f8 = NEXTAFTER_exp1, NEXTAFTER_lden_sig
      nop.i 999
}
// Force underflow and inexact
{ .mfb
      nop.m 999
      fma.s0 NEXTAFTER_tmp = NEXTAFTER_tmp,NEXTAFTER_tmp,f0
      br.ret.sptk b0 ;;
}

NEXTAFTER_UNDERFLOW_TO_ZERO:
{ .mfb
      cmp.eq p6,p0 = r0,r0
      fmerge.s f8 = NEXTAFTER_save_f8,f0
      br.cond.sptk NEXTAFTER_COMMON_FINISH ;;
}

NEXTAFTER_INF: 
// Here if f8 is +- infinity
// INF
// if f8 is +inf, no matter what y is return  largest long double
// if f8 is -inf, no matter what y is return -largest long double

// Create largest long double
{ .mfi
      nop.m 999
      fmerge.se NEXTAFTER_lnorm = NEXTAFTER_lnorm_exp,NEXTAFTER_lnorm_sig
      nop.i 999 ;;
}

{ .mfb
      nop.m 999
      fmerge.s f8 = f8,NEXTAFTER_lnorm                
      br.ret.sptk    b0 ;;                        
}

NEXTAFTER_ZERO: 

// Here if f8 is +- zero
// ZERO
// if f8 is zero and y is +, return + smallest long double denormal 
// if f8 is zero and y is -, return - smallest long double denormal 

{ .mfi
      nop.m 999
      fmerge.se NEXTAFTER_sden = f0,NEXTAFTER_sden_sig
      nop.i 999 ;;
}

// Create small normal to generate underflow flag
{ .mfi
      nop.m 999
      fmerge.se NEXTAFTER_tmp = NEXTAFTER_snorm_exp, NEXTAFTER_lnorm_sig
      nop.i 999 ;;
}

// Add correct sign from direction arg
{ .mfi
      nop.m 999
      fmerge.s f8 = f9,NEXTAFTER_sden                
      nop.i 999 ;;
}

// Force underflow and inexact flags
{ .mfb
      nop.m 999
      fma.s0 NEXTAFTER_tmp = NEXTAFTER_tmp,NEXTAFTER_tmp,f0
      br.ret.sptk    b0 ;;                        
}

GLOBAL_LIBM_END(nextafterl)
// Stack operations when calling error support.
//       (1)               (2)                          (3) (call)              (4)
//   sp   -> +          psp -> +                     psp -> +                   sp -> +
//           |                 |                            |                         |
//           |                 | <- GR_Y               R3 ->| <- GR_RESULT            | -> f8
//           |                 |                            |                         |
//           | <-GR_Y      Y2->|                       Y2 ->| <- GR_Y                 |
//           |                 |                            |                         |
//           |                 | <- GR_X               X1 ->|                         |
//           |                 |                            |                         |
//  sp-64 -> +          sp ->  +                     sp ->  +                         +
//    save ar.pfs          save b0                                               restore gp
//    save gp                                                                    restore ar.pfs



LOCAL_LIBM_ENTRY(__libm_error_region)
NEXTAFTER_OVERFLOW: 
// Here if f8 is finite, but result will be infinite
// Use frcpa to generate infinity of correct sign
// Call error support to report possible range error
.prologue

{ .mfi
      alloc          r32=ar.pfs,2,2,4,0
      frcpa.s1 f8,p6 = NEXTAFTER_save_f8, f0
      nop.i 999 ;;
}

// Create largest long double
{ .mfi
      nop.m 999
      fmerge.se NEXTAFTER_lnorm = NEXTAFTER_lnorm_exp,NEXTAFTER_lnorm_sig
      nop.i 999 ;;
}

// Force overflow and inexact flags to be set
{ .mfi
      mov           r39 = 153      // Error code
      fma.s0  NEXTAFTER_tmp = NEXTAFTER_lnorm,NEXTAFTER_lnorm,f0
      nop.i 999
}
;;

// (1)
{ .mfi
        add   GR_Parameter_Y=-32,sp             // Parameter 2 value
        nop.f 0
.save   ar.pfs,GR_SAVE_PFS
        mov  GR_SAVE_PFS=ar.pfs                 // Save ar.pfs
}
{ .mfi
.fframe 64
        add sp=-64,sp                          // Create new stack
        nop.f 0
        mov GR_SAVE_GP=gp                      // Save gp
};;


// (2)
{ .mmi
        stfe [GR_Parameter_Y] = f9,16         // STORE Parameter 2 on stack
        add GR_Parameter_X = 16,sp            // Parameter 1 address
.save   b0, GR_SAVE_B0
        mov GR_SAVE_B0=b0                     // Save b0
};;

.body
// (3)
{ .mib
        stfe [GR_Parameter_X] = NEXTAFTER_save_f8              // STORE Parameter 1 on stack
        add   GR_Parameter_RESULT = 0,GR_Parameter_Y           // Parameter 3 address
        nop.b 0                                
}
{ .mib
        stfe [GR_Parameter_Y] = f8              // STORE Parameter 3 on stack
        add   GR_Parameter_Y = -16,GR_Parameter_Y
        br.call.sptk b0=__libm_error_support#   // Call error handling function
};;
{ .mmi
        nop.m 0
        nop.m 0
        add   GR_Parameter_RESULT = 48,sp
};;

// (4)
{ .mmi
        ldfe  f8 = [GR_Parameter_RESULT]       // Get return result off stack
.restore sp
        add   sp = 64,sp                       // Restore stack pointer
        mov   b0 = GR_SAVE_B0                  // Restore return address
};;
{ .mib
        mov   gp = GR_SAVE_GP                  // Restore gp
        mov   ar.pfs = GR_SAVE_PFS             // Restore ar.pfs
        br.ret.sptk     b0                     // Return
};;

LOCAL_LIBM_END(__libm_error_region)


.type   __libm_error_support#,@function
.global __libm_error_support#

