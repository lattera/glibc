.file "acosl.s"

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
// 2/07/00  Modified calculation of acos_corr to correct acosl
// 4/04/00  Unwind support added
// 8/15/00  Bundle added after call to __libm_error_support to properly
//          set [the previously overwritten] GR_Parameter_RESULT.
// 12/20/00 Set denormal flag properly.
//
// API
//==============================================================
// double-extended = acosl (double-extended)
// input  floating point f8
// output floating point f8
//
// Registers used
//==============================================================
//
// predicate registers used:
// p6 -> p12
//
// floating-point registers used:
// f8 has input, then output
// f8 -> f15, f32 ->f99
//
// general registers used:
// r32 -> r48
//
// Overview of operation
//==============================================================
// There are three paths
// 1. |x| < 2^-25                 ACOS_TINY
// 2. 2^-25 <= |x| < 1/4          ACOS_POLY
// 3. 1/4 <= |x| < 1              ACOS_ATAN

#include "libm_support.h"

// Assembly macros
//==============================================================

// f8 is input, but acos_V must be put in f8
//    when __libm_atan2_reg is called, f8 must get V
// f9 gets U when __libm_atan2_reg is called


// __libm_atan2_reg returns 
// f8  = Z_hi
// f10 = Z_lo
// f11 = s_lo

acos_Z_hi = f8
acos_Z_lo = f10
acos_S_lo = f11

// When we call __libm_atan2_reg, we must save 
// the following:

acos_corr  = f12
acos_X     = f13
acos_pi_hi = f14
acos_pi_lo = f15

// The rest of the assembly macros

acos_P79                   = f32
acos_P59                   = f33
acos_P39                   = f34
acos_P19                   = f35

acos_P810                  = f36
acos_P610                  = f37
acos_P410                  = f38
acos_P210                  = f39

acos_A1                    = f41
acos_A2                    = f42
acos_A3                    = f43
acos_A4                    = f44
acos_A5                    = f45
acos_A6                    = f46
acos_A7                    = f47
acos_A8                    = f48
acos_A9                    = f49
acos_A10                   = f50

acos_X2                    = f51
acos_X4                    = f52

acos_B                     = f53
acos_Bb                    = f54
acos_A                     = f55
acos_Aa                    = f56

acos_1mA                   = f57

acos_W                     = f58
acos_Ww                    = f59

acos_y0                    = f60
acos_y1                    = f61
acos_y2                    = f62

acos_H                     = f63
acos_Hh                    = f64

acos_t1                    = f65
acos_t2                    = f66
acos_t3                    = f67
acos_t4                    = f68
acos_t5                    = f69

acos_Pseries               = f70
acos_NORM_f8               = f71
acos_ABS_NORM_f8           = f72

acos_2                     = f73
acos_P1P2                  = f74
acos_HALF                  = f75
acos_U                     = f76

acos_1mB                   = f77
acos_V                     = f78 
acos_S                     = f79

acos_BmUU                  = f80 
acos_BmUUpb                = f81 
acos_2U                    = f82
acos_1d2U                  = f83

acos_Dd                    = f84

acos_pi_by_2_hi            = f85
acos_pi_by_2_lo            = f86
acos_xmpi_by_2_lo          = f87
acos_xPmw                  = f88

acos_Uu                    = f89
acos_AmVV                  = f90 
acos_AmVVpa                = f91 

acos_2V                    = f92 
acos_1d2V                  = f93
acos_Vv                    = f94

acos_Vu                    = f95 
acos_Uv                    = f96 

acos_2_Z_hi                = f97
acos_s_lo_Z_lo             = f98
acos_result_lo             = f99

acos_Z_hi                  = f8
acos_Z_lo                  = f10
acos_s_lo                  = f11

acos_GR_17_ones            = r33
acos_GR_16_ones            = r34
acos_GR_signexp_f8         = r35
acos_GR_exp                = r36
acos_GR_true_exp           = r37
acos_GR_fffe               = r38

GR_SAVE_PFS                = r43
GR_SAVE_B0                 = r39
GR_SAVE_GP                 = r41

// r40 is address of table of coefficients
// r42 

GR_Parameter_X             = r44 
GR_Parameter_Y             = r45 
GR_Parameter_RESULT        = r46 
GR_Parameter_TAG                = r47 


// 2^-40:
// A true exponent of -40 is
//                    : -40 + register_bias
//                    : -28 + ffff = ffd7

// A true exponent of 1 is 
//                    : 1 + register_bias
//                    : 1 + ffff = 10000

// Data tables
//==============================================================

#ifdef _LIBC
.rodata
#else
.data
#endif

.align 16

acos_coefficients:
ASM_TYPE_DIRECTIVE(acos_coefficients,@object)
data8  0xc90fdaa22168c234, 0x00003FFF            // pi_by_2_hi
data8  0xc4c6628b80dc1cd1, 0x00003FBF            // pi_by_2_lo
data8  0xc90fdaa22168c234, 0x00004000            // pi_hi
data8  0xc4c6628b80dc1cd1, 0x00003FC0            // pi_lo

data8  0xBB08911F2013961E, 0x00003FF8            // A10
data8  0x981F1095A23A87D3, 0x00003FF8            // A9 
data8  0xBDF09C6C4177BCC6, 0x00003FF8            // A8 
data8  0xE4C3A60B049ACCEA, 0x00003FF8            // A7 
data8  0x8E2789F4E8A8F1AD, 0x00003FF9            // A6 
data8  0xB745D09B2B0E850B, 0x00003FF9            // A5 
data8  0xF8E38E3BC4C50920, 0x00003FF9            // A4 
data8  0xB6DB6DB6D89FCD81, 0x00003FFA            // A3 
data8  0x99999999999AF376, 0x00003FFB            // A2 
data8  0xAAAAAAAAAAAAAA71, 0x00003FFC            // A1
ASM_SIZE_DIRECTIVE(acos_coefficients)


.align 32
.global acosl#
ASM_TYPE_DIRECTIVE(acosl#,@function)

.section .text
.proc  acosl#
.align 32


acosl: 

// After normalizing f8, get its true exponent
{ .mfi
      alloc r32 = ar.pfs,1,11,4,0                                             
(p0)  fnorm.s1    acos_NORM_f8 = f8                                            
(p0)  mov         acos_GR_17_ones = 0x1ffff                                    
}

{ .mmi
(p0)  mov        acos_GR_16_ones = 0xffff                                     
(p0)  addl                 r40   = @ltoff(acos_coefficients), gp
      nop.i 999
}
;;

// Set denormal flag on denormal input with fcmp
{ .mfi
      ld8 r40 = [r40]
      fcmp.eq  p6,p0 = f8,f0
      nop.i 999
}
;;


// Load the constants pi_by_2 and pi.
// Each is stored as hi and lo values
// Also load the coefficients for ACOS_POLY

{ .mmi
(p0) ldfe       acos_pi_by_2_hi = [r40],16 ;;      
(p0) ldfe       acos_pi_by_2_lo = [r40],16      
     nop.i 999 ;;
}

{ .mmi
(p0) ldfe       acos_pi_hi      = [r40],16 ;;      
(p0) ldfe       acos_pi_lo      = [r40],16      
     nop.i 999 ;;
}

{ .mmi
(p0) ldfe       acos_A10        = [r40],16 ;;      
(p0) ldfe       acos_A9         = [r40],16      
     nop.i 999 ;;
}

// Take the absolute value of f8
{ .mmf
      nop.m 999
(p0)  getf.exp   acos_GR_signexp_f8  = acos_NORM_f8                           
(p0)  fmerge.s  acos_ABS_NORM_f8 = f0, acos_NORM_f8 
}

{ .mii
(p0) ldfe       acos_A8         = [r40],16      
     nop.i 999 ;;
(p0) and        acos_GR_exp         = acos_GR_signexp_f8, acos_GR_17_ones ;;    
}

// case 1: |x| < 2^-25         ==> p6   ACOS_TINY
// case 2: 2^-25 <= |x| < 2^-2 ==> p8   ACOS_POLY
// case 3: 2^-2  <= |x| < 1    ==> p9   ACOS_ATAN
// case 4: 1     <= |x|        ==> p11  ACOS_ERROR_RETURN
//  Admittedly |x| = 1 is not an error but this is where that case is
//  handled.

{ .mii
(p0) ldfe       acos_A7         = [r40],16      
(p0) sub        acos_GR_true_exp    = acos_GR_exp, acos_GR_16_ones ;;           
(p0) cmp.ge.unc p6, p7    = -26, acos_GR_true_exp ;;                            
}

{ .mii
(p0) ldfe       acos_A6         = [r40],16      
(p7) cmp.ge.unc p8, p9    = -3,  acos_GR_true_exp ;;                            
(p9) cmp.ge.unc p10, p11  =  -1, acos_GR_true_exp                            
}

{ .mmi
(p0) ldfe       acos_A5         = [r40],16 ;;      
(p0) ldfe       acos_A4         = [r40],16      
      nop.i 999 ;;
}

{ .mmi
(p0) ldfe       acos_A3         = [r40],16 ;;      
(p0) ldfe       acos_A2         = [r40],16      
      nop.i 999 ;;
}

// ACOS_ERROR_RETURN ==> p11 is true
// case 4: |x| >= 1
{ .mib
(p0)  ldfe       acos_A1         = [r40],16      
      nop.i 999
(p11) br.spnt         L(ACOS_ERROR_RETURN) ;; 
}

// ACOS_TINY ==> p6 is true
// case 1: |x| < 2^-25
{ .mfi
      nop.m 999
(p6)  fms.s1        acos_xmpi_by_2_lo = acos_NORM_f8,f1, acos_pi_by_2_lo 
      nop.i 999 ;;
}

{ .mfb
           nop.m 999
(p6)  fms.s0         f8 = acos_pi_by_2_hi,f1, acos_xmpi_by_2_lo                
(p6)  br.ret.spnt   b0 ;;                                                   
}



// ACOS_POLY ==> p8 is true
// case 2: 2^-25 <= |x| < 2^-2                   
{ .mfi
      nop.m 999
(p8)  fms.s1        acos_W       = acos_pi_by_2_hi, f1, acos_NORM_f8     
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fma.s1        acos_X2   = f8,f8, f0                                
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fms.s1        acos_Ww      = acos_pi_by_2_hi, f1, acos_W           
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fma.s1        acos_X4   = acos_X2,acos_X2, f0                      
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fms.s1        acos_Ww      = acos_Ww, f1, acos_NORM_f8             
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fma.s1        acos_P810 = acos_X4, acos_A10, acos_A8               
      nop.i 999
}

// acos_P79  = X4*A9   + A7
// acos_P810 = X4*A10  + A8
{ .mfi
      nop.m 999
(p8)  fma.s1        acos_P79  = acos_X4, acos_A9, acos_A7                
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fma.s1        acos_Ww      = acos_Ww, f1, acos_pi_by_2_lo          
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fma.s1        acos_P610 = acos_X4, acos_P810, acos_A6              
      nop.i 999
}


// acos_P59   = X4*(X4*A9   + A7)  + A5
// acos_P610  = X4*(X4*A10  + A8)  + A6
{ .mfi
      nop.m 999
(p8)  fma.s1        acos_P59  = acos_X4, acos_P79, acos_A5               
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fma.s1        acos_P410 = acos_X4, acos_P610, acos_A4              
      nop.i 999
}

// acos_P39   = X4*(X4*(X4*A9   + A7)  + A5) + A3
// acos_P410  = X4*(X4*(X4*A10  + A8)  + A6) + A4
{ .mfi
      nop.m 999
(p8)  fma.s1        acos_P39  = acos_X4, acos_P59, acos_A3               
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fma.s1        acos_P210 = acos_X4, acos_P410, acos_A2              
      nop.i 999
}

// acos_P19   = X4*(X4*(X4*(X4*A9   + A7)  + A5) + A3) + A1 = P1
// acos_P210  = X4*(X4*(X4*(X4*A10  + A8)  + A6) + A4) + A2 = P2
{ .mfi
      nop.m 999
(p8)  fma.s1        acos_P19  = acos_X4, acos_P39, acos_A1               
      nop.i 999 ;;
}

// acos_P1P2 = Xsq*P2 + P1
// acos_P1P2 = Xsq*(Xsq*P2 + P1)
{ .mfi
      nop.m 999
(p8)  fma.s1        acos_P1P2    = acos_X2, acos_P210, acos_P19          
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fma.s1        acos_P1P2    = acos_X2, acos_P1P2, f0                
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fms.s1        acos_xPmw    = acos_NORM_f8, acos_P1P2, acos_Ww       
      nop.i 999 ;;
}

{ .mfb
      nop.m 999
(p8)  fms.s0         f8           = acos_W, f1, acos_xPmw                 
(p8)  br.ret.spnt   b0 ;;                                                   
}


// ACOS_ATAN
// case 3: 2^-2  <= |x| < 1                      
// case 3: 2^-2  <= |x| < 1    ==> p9   ACOS_ATAN

// Step 1.1:     Get A,B and a,b
// A + a = 1- |X|
// B + b = 1+ |X|
// Note also that we will use  acos_corr (f13)
// and                         acos_W

// Step 2
// Call __libm_atan2_reg


{ .mfi
(p0)  mov    acos_GR_fffe = 0xfffe                      
(p0)  fma.s1 acos_B          = f1,f1,  acos_ABS_NORM_f8                            
(p0)  mov   GR_SAVE_B0 = b0 ;;                                
}

{ .mmf
(p0)  mov   GR_SAVE_GP = gp                                
      nop.m 999
(p0)  fms.s1 acos_A   = f1,f1,  acos_ABS_NORM_f8                            
}

{ .mfi
(p0)  setf.exp       acos_HALF = acos_GR_fffe                   
      nop.f 999
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fms.s1 acos_1mB = f1,f1, acos_B                                       
      nop.i 999 ;;
}

// We want atan2(V,U)
//   so put V in f8 and U in f9
//   but save X in acos_X

{ .mfi
      nop.m 999
(p0)  fmerge.se acos_X = f8, f8                               
      nop.i 999 ;;
}

// Step 1.2:
/////////////////////////
// Get U = sqrt(B)
/////////////////////////

{ .mfi
      nop.m 999
(p0)  frsqrta.s1     acos_y0,p8  = acos_B                                
      nop.i 999
}

{ .mfi
      nop.m 999
(p0)  fms.s1 acos_1mA = f1,f1, acos_A                                       
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1 acos_Bb  = acos_1mB,f1, acos_ABS_NORM_f8                       
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_Hh     = acos_HALF, acos_B, f0                 
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_t1     = acos_y0, acos_y0, f0                  
      nop.i 999
}

{ .mfi
      nop.m 999
(p0)  fms.s1 acos_Aa  = acos_1mA,f1, acos_ABS_NORM_f8                       
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fnma.s1        acos_t2     = acos_t1, acos_Hh, acos_HALF           
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_y1     = acos_t2, acos_y0, acos_y0             
      nop.i 999
}


// Step 1.2:
/////////////////////////
// Get V = sqrt(A)
/////////////////////////
{ .mfi
      nop.m 999
(p0)  frsqrta.s1     acos_y0,p8  = acos_A                                
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_t3     = acos_y1, acos_Hh, f0                  
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_t1     = acos_y0, acos_y0, f0                  
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fnma.s1        acos_t4     = acos_t3, acos_y1, acos_HALF           
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_y2     = acos_t4, acos_y1, acos_y1             
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_S      = acos_B, acos_y2, f0                   
      nop.i 999
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_H      = acos_y2, acos_HALF, f0                
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_t5     = acos_Hh, acos_y2, f0                  
      nop.i 999
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_Hh     = acos_HALF, acos_A, f0                 
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fnma.s1        acos_Dd     = acos_S, acos_S, acos_B                
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fnma.s1        acos_t2     = acos_t1, acos_Hh, acos_HALF           
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_U      = acos_Dd, acos_H, acos_S               
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_y1     = acos_t2, acos_y0, acos_y0             
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_2U       = acos_U, f1, acos_U                  
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_t3     = acos_y1, acos_Hh, f0                  
      nop.i 999
}


// Step 1.3: 
// sqrt(A + a) = V + v
// sqrt(B + b) = U + u

/////////////////////////
// Get u
/////////////////////////

// acos_BmUU   = B - UU
// acos_BmUUpb = (B - UU) + b

{ .mfi
      nop.m 999
(p0)  fnma.s1        acos_BmUU     = acos_U, acos_U, acos_B              
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)   fmerge.se f9 = acos_U, acos_U                           
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fnma.s1        acos_t4     = acos_t3, acos_y1, acos_HALF           
      nop.i 999 ;;
}

// acos_1d2U = frcpa(2U)
{ .mfi
      nop.m 999
(p0)  frcpa.s1       acos_1d2U,p9  = f1, acos_2U                         
      nop.i 999
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_BmUUpb   = acos_BmUU, f1, acos_Bb              
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_y2     = acos_t4, acos_y1, acos_y1             
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
// acos_Uu = ((B - UU) + b) * frcpa(2U)
(p0)  fma.s1         acos_Uu       = acos_BmUUpb, acos_1d2U, f0          
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_S      = acos_A, acos_y2, f0                   
      nop.i 999
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_H      = acos_y2, acos_HALF, f0                
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_t5     = acos_Hh, acos_y2, f0                  
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fnma.s1        acos_Dd     = acos_S, acos_S, acos_A                
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_V      = acos_Dd, acos_H, acos_S               
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_2V       = acos_V, f1, acos_V                  
      nop.i 999
}

// Step 3
/////////////////////////
// Calculate the correction, acos_corr
/////////////////////////
// acos_corr = U*v - (V*u)

{ .mfi
      nop.m 999
(p0)  fma.s1  acos_Vu   = acos_V,acos_Uu, f0                  
      nop.i 999 ;;
}

/////////////////////////
// Get v
/////////////////////////
// acos_AmVV   = A - VV
// acos_AmVVpa = (A - VV) + a

{ .mfi
      nop.m 999
(p0)  fnma.s1        acos_AmVV     = acos_V, acos_V, acos_A              
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)   fmerge.se f8 = acos_V, acos_V                           
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         acos_AmVVpa   = acos_AmVV, f1, acos_Aa              
      nop.i 999 ;;
}

// acos_1d2V = frcpa(2V)
{ .mfi
      nop.m 999
(p0)  frcpa.s1       acos_1d2V,p9  = f1, acos_2V                         
      nop.i 999 ;;
}

// acos_Vv = ((A - VV) + a) * frcpa(2V)
{ .mfi
      nop.m 999
(p0)  fma.s1         acos_Vv       = acos_AmVVpa, acos_1d2V, f0          
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)   fma.s1  acos_Uv   = acos_U,acos_Vv, f0                  
      nop.i 999 ;;
}


.endp acosl#
ASM_SIZE_DIRECTIVE(acosl#)


.proc __libm_callout
__libm_callout:
.prologue
{ .mfi
        nop.m 0
        nop.f 0
.save   ar.pfs,GR_SAVE_PFS
        mov  GR_SAVE_PFS=ar.pfs
}
;;

{ .mfi
        mov GR_SAVE_GP=gp
        nop.f 0
.save   b0, GR_SAVE_B0
        mov GR_SAVE_B0=b0
}

.body
{ .mfb
      nop.m 999
(p0)   fms.s1  acos_corr = acos_Uv,f1, acos_Vu                 
(p0)   br.call.sptk.many  b0=__libm_atan2_reg# ;;                        
}


// p6 ==> X is negative
// p7 ==> x is positive
// We know that |X| >= 1/4

{ .mfi
(p0)   mov gp              = GR_SAVE_GP                           
(p0)   fcmp.lt.unc   p6,p7 = acos_X , f0                       
(p0)   mov b0              = GR_SAVE_B0 ;;                           
}

// acos_2_Z_hi    = 2 * acos_Z_hi
// acos_s_lo_Z_lo = s_lo * Z_lo

{ .mfi
       nop.m 999
(p0)   fma.s1  acos_2_Z_hi      = acos_Z_hi, f1, acos_Z_hi               
(p0)   mov ar.pfs               = GR_SAVE_PFS                                     
}

{ .mfi
      nop.m 999
(p0)   fma.s1  acos_s_lo_Z_lo   = acos_s_lo, acos_Z_lo, f0               
      nop.i 999 ;;
}

// 2 is a constant needed later
{ .mfi
      nop.m 999
(p0)  fma.s1     acos_2 = f1,f1,f1                             
      nop.i 999 ;;
}

// X >= 1/4
// acos_result_lo = 2(s_lo * Z_lo) - corr
// f8             = (2*Z_hi) + (2(s_lo * Z_lo) - corr)

{ .mfi
      nop.m 999
(p7)   fma.s1  acos_result_lo     = acos_s_lo_Z_lo, acos_2, acos_corr      
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p7)  fma.s0   f8                 = acos_2_Z_hi, f1, acos_result_lo        
      nop.i 999
}

// acos_result_lo = (pi_lo - corr)
// acos_result_lo = (pi_lo - corr) + acos_Ww 
{ .mfi
      nop.m 999
(p6)  fms.s1  acos_result_lo     = acos_pi_lo, f1, acos_corr              
      nop.i 999 ;;
}

// X <= -1/4
// acos_W = pi_hi - 2 * Z_hi
{ .mfi
      nop.m 999
(p6)  fnma.s1 acos_W             = acos_2, acos_Z_hi, acos_pi_hi          
      nop.i 999 ;;
}

// acos_Ww = pi_hi - W
// acos_Ww = (pi_hi - W) + (2 * Z_hi)
{ .mfi
      nop.m 999
(p6)  fms.s1  acos_Ww            = acos_pi_hi, f1, acos_W                 
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p6)   fms.s1  acos_Ww            = acos_Ww, f1, acos_2_Z_hi               
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p6)   fma.s1  acos_result_lo     = acos_result_lo, f1, acos_Ww            
      nop.i 999 ;;
}

// acos_Z_lo = ((pi_lo - corr) + acos_Ww) - 2 * (s_lo * Z_lo)
{ .mfi
      nop.m 999
(p6)  fnma.s1  acos_Z_lo          = acos_s_lo_Z_lo, acos_2, acos_result_lo 
      nop.i 999 ;;
}

{ .mfb
      nop.m 999
(p6)  fma.s0   f8                  = acos_W, f1, acos_Z_lo                
(p0)  br.ret.sptk   b0 ;;                          
}
.endp __libm_callout
ASM_SIZE_DIRECTIVE(__libm_callout)

.proc SPECIAL
SPECIAL:
L(ACOS_NAN): 
{ .mfb
      nop.m 999
(p0)  fma.s0 f8 = f8,f1,f0                       
(p0)  br.ret.sptk   b0 ;;                          
}

L(ACOS_ERROR_RETURN): 
// Save ar.pfs, b0, and gp; restore on exit

// qnan snan inf norm     unorm 0 -+
// 1    1    0   0        0     0 11 = 0xc3

// Coming in as X = +- 1
// What should we return?

// If X is 1, return (sign of X)pi/2


{ .mfi
      nop.m 999
(p0)  fcmp.eq.unc p6,p7 = acos_ABS_NORM_f8,f1              
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p6)  fcmp.lt.unc p8,p9 = f8,f0                            
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fma.s0 f8          = acos_pi_hi, f1, acos_pi_lo       
      nop.i 999
}

{ .mfb
      nop.m 999
(p9)  fmerge.s    f8 = f8,f0                               
(p6)  br.ret.spnt   b0 ;;                                     
}

// If X is a NAN, leave
{ .mfi
      nop.m 999
(p0)  fclass.m.unc p12,p0 = f8, 0xc3            
      nop.i 999 ;;
}

{ .mfb
      nop.m 999
(p12) fma.s0 f8 = f8,f1,f0                       
(p12) br.ret.spnt   b0 ;;                          
}

{ .mfi
(p0)   mov   GR_Parameter_TAG = 57 
(p0)   frcpa f10, p6 = f0, f0
nop.i 999
};;

.endp SPECIAL
ASM_SIZE_DIRECTIVE(SPECIAL)

.proc __libm_error_region
__libm_error_region:
.prologue
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
        stfe [GR_Parameter_Y] = f1,16         // Store Parameter 2 on stack
        add GR_Parameter_X = 16,sp            // Parameter 1 address
.save   b0, GR_SAVE_B0
        mov GR_SAVE_B0=b0                     // Save b0
};;

.body
// (3)
{ .mib
        stfe [GR_Parameter_X] = f8              // Store Parameter 1 on stack
        add   GR_Parameter_RESULT = 0,GR_Parameter_Y
        nop.b 0                                 // Parameter 3 address
}
{ .mib
        stfe [GR_Parameter_Y] = f10             // Store Parameter 3 on stack
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

.endp __libm_error_region
ASM_SIZE_DIRECTIVE(__libm_error_region)

.type   __libm_error_support#,@function
.global __libm_error_support#

.type   __libm_atan2_reg#,@function
.global __libm_atan2_reg#
