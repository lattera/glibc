.file "asinl.s"

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
// 4/04/00  Unwind support added
// 8/15/00  Bundle added after call to __libm_error_support to properly
//          set [the previously overwritten] GR_Parameter_RESULT.
//
// API
//==============================================================
// long double = asinl(long double)
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
// f32 -> f87, f8 -> f13, f32 -> f87
//
// general registers used:
// r32 -> r47
//
// Overview of operation
//==============================================================
// There are three paths
// 1. |x| < 2^-40                 ASIN_TINY
// 2. 2^-40 <= |x| < 1/4          ASIN_POLY
// 3. 1/4 <= |x| < 1              ASIN_ATAN

#include "libm_support.h"

// Assembly macros
//==============================================================
FR_RESULT = f10
FR_X = f8
FR_Y = f1
asin_P79                   = f32
asin_P59                   = f33
asin_P39                   = f34
asin_P19                   = f35

asin_P810                  = f36
asin_P610                  = f37
asin_P410                  = f38
asin_P210                  = f39

asin_A1                    = f41
asin_A2                    = f42
asin_A3                    = f43
asin_A4                    = f44
asin_A5                    = f45
asin_A6                    = f46
asin_A7                    = f47
asin_A8                    = f48
asin_A9                    = f49
asin_A10                   = f50

asin_X2                    = f51
asin_X4                    = f52

asin_B                     = f53
asin_Bb                    = f54
asin_C                     = f55
asin_Cc                    = f56
asin_D                     = f57

asin_W                     = f58
asin_Ww                    = f59

asin_y0                    = f60
asin_y1                    = f61
asin_y2                    = f62

asin_H                     = f63
asin_Hh                    = f64

asin_t1                    = f65
asin_t2                    = f66
asin_t3                    = f67
asin_t4                    = f68
asin_t5                    = f69

asin_Pseries               = f70
asin_NORM_f8               = f71
asin_ABS_NORM_f8           = f72

asin_2m100                 = f73
asin_P1P2                  = f74
asin_HALF                  = f75
asin_1mD                   = f76

asin_1mB                   = f77
asin_1mBmC                 = f78 
asin_S                     = f79

asin_BmWW                  = f80 
asin_BmWWpb                = f81 
asin_2W                    = f82 
asin_1d2W                  = f83 
asin_Dd                    = f84

asin_XWw                   = f85 
asin_low                   = f86

asin_pi_by_2               = f87
asin_pi_by_2_lo            = f88

asin_GR_17_ones            = r33
asin_GR_16_ones            = r34
asin_GR_signexp_f8         = r35
asin_GR_exp                = r36
asin_GR_true_exp           = r37
asin_GR_ff9b               = r38 

GR_SAVE_B0              = r39
GR_SAVE_SP              = r40
GR_SAVE_PFS             = r33 
// r33 can be used safely.
// r40 is address of table of coefficients
// Later it is used to save sp across calls 
GR_SAVE_GP              = r41
asin_GR_fffe               = r42 
asin_GR_retval             = r43 

GR_Parameter_X                 = r44 
GR_Parameter_Y                 = r45 
GR_Parameter_RESULT            = r46 
GR_Parameter_TAG               = r47 


// 2^-40:
// A true exponent of -40 is
//                    : -40 + register_bias
//                    : -28 + ffff = ffd7

// A true exponent of -100 is 
//                    : -100 + register_bias
//                    : -64 + ffff = ff9b

// Data tables
//==============================================================

#ifdef _LIBC
.rodata
#else
.data
#endif

.align 16

asin_coefficients:
ASM_TYPE_DIRECTIVE(asin_coefficients,@object)
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

data8  0xc90fdaa22168c234, 0x00003FFF            // pi_by_2_hi
data8  0xc4c6628b80dc1cd1, 0x00003FBF            // pi_by_2_lo
ASM_SIZE_DIRECTIVE(asin_coefficients)

.align 32
.global asinl#

.section .text
.proc  asinl#
.align 32


asinl: 

{ .mfi
      alloc r32 = ar.pfs,1,11,4,0                        
(p0)  fnorm      asin_NORM_f8 = f8                       
(p0)  mov        asin_GR_17_ones = 0x1ffff               
}

{ .mii
(p0)  mov        asin_GR_16_ones = 0xffff                
(p0)  mov        asin_GR_ff9b = 0xff9b ;;                   
      nop.i 999
}


{ .mmi
(p0)  setf.exp  asin_2m100 = asin_GR_ff9b                                      
(p0)  addl           r40   = @ltoff(asin_coefficients), gp
      nop.i 999
}
;;

{ .mmi
      ld8 r40 = [r40]
      nop.m 999
      nop.i 999
}
;;



// Load the constants

{ .mmi
(p0) ldfe       asin_A10 = [r40],16 ;;      
(p0) ldfe       asin_A9  = [r40],16      
      nop.i 999 ;;
}

{ .mmi
(p0) ldfe       asin_A8  = [r40],16 ;;      
(p0) ldfe       asin_A7  = [r40],16      
      nop.i 999 ;;
}

{ .mmi
(p0) ldfe       asin_A6  = [r40],16 ;;      
(p0)  getf.exp   asin_GR_signexp_f8  = asin_NORM_f8                            
      nop.i 999
}

{ .mmi
(p0) ldfe       asin_A5  = [r40],16 ;;      
(p0) ldfe       asin_A4  = [r40],16      
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0) fmerge.s   asin_ABS_NORM_f8 = f0, asin_NORM_f8            
(p0)  and        asin_GR_exp         = asin_GR_signexp_f8, asin_GR_17_ones ;;     
}

// case 1: |x| < 2^-40         ==> p6 (includes x = +-0)
// case 2: 2^-40 <= |x| < 2^-2 ==> p8
// case 3: 2^-2  <= |x| < 1    ==> p9
// case 4: 1  <= |x|           ==> p11
//   In case 4, we pick up the special case x = +-1 and return +-pi/2

{ .mii
(p0) ldfe       asin_A3  = [r40],16      
(p0)  sub        asin_GR_true_exp    = asin_GR_exp, asin_GR_16_ones ;;            
(p0)  cmp.ge.unc p6, p7    = -41, asin_GR_true_exp ;;             
}

{ .mii
(p0) ldfe       asin_A2  = [r40],16      
(p7)  cmp.ge.unc p8, p9    = -3,  asin_GR_true_exp ;;             
(p9)  cmp.ge.unc p10, p11  = -1,  asin_GR_true_exp              
}

{ .mmi
(p0) ldfe       asin_A1  = [r40],16 ;;      
(p0) ldfe       asin_pi_by_2  = [r40],16 
      nop.i 999
}

// case 4: |x| >= 1
{ .mib
      nop.m 999
      nop.i 999
(p11) br.spnt         L(ASIN_ERROR_RETURN) ;;                         
}

// case 1: |x| < 2^-40
{ .mfb
      nop.m 999
(p6)  fma.s0         f8 = asin_2m100,f8,f8                       
(p6)  br.ret.spnt   b0 ;;                                          
}


// case 2: 2^-40 <= |x| < 2^-2 ==> p8
{ .mfi
      nop.m 999
(p8)  fma.s1        asin_X2   = f8,f8, f0                       
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fma.s1        asin_X4   = asin_X2,asin_X2, f0             
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fma.s1        asin_P810 = asin_X4, asin_A10, asin_A8      
      nop.i 999
}

{ .mfi
      nop.m 999
(p8)  fma.s1        asin_P79  = asin_X4, asin_A9, asin_A7       
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fma.s1        asin_P610 = asin_X4, asin_P810, asin_A6     
      nop.i 999
}

{ .mfi
      nop.m 999
(p8)  fma.s1        asin_P59  = asin_X4, asin_P79, asin_A5      
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fma.s1        asin_P410 = asin_X4, asin_P610, asin_A4     
      nop.i 999
}

{ .mfi
      nop.m 999
(p8)  fma.s1        asin_P39  = asin_X4, asin_P59, asin_A3      
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fma.s1        asin_P210 = asin_X4, asin_P410, asin_A2     
      nop.i 999
}

{ .mfi
      nop.m 999
(p8)  fma.s1        asin_P19  = asin_X4, asin_P39, asin_A1      
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fma.s1        asin_P1P2    = asin_X2, asin_P210, asin_P19 
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)  fma.s1        asin_P1P2    = asin_X2, asin_P1P2, f0       
      nop.i 999 ;;
}

{ .mfb
      nop.m 999
(p8)  fma.s0        f8 = asin_NORM_f8, asin_P1P2, asin_NORM_f8  
(p8)  br.ret.spnt   b0 ;;                                          
}

// case 3: 2^-2  <= |x| < 1    
// 1- X*X is computed as B + b
// Step 1.1:     Get B and b

// atan2 will return
//   f8  = Z_hi
//   f10 = Z_lo
//   f11 = s_lo


{ .mfi
(p0)  mov            asin_GR_fffe = 0xfffe                      
(p0)   fmerge.se f8 = asin_ABS_NORM_f8, asin_ABS_NORM_f8                                   
nop.i 0
};;

{ .mmf
nop.m 0
(p0)   setf.exp       asin_HALF = asin_GR_fffe                   
(p0)   fmerge.se f12 = asin_NORM_f8, asin_NORM_f8 ;;                         
}


{ .mfi
      nop.m 999
(p0)  fcmp.lt.unc.s1 p6,p7 = asin_ABS_NORM_f8, asin_HALF        
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p7)  fma.s1         asin_D   = f1,f1,asin_ABS_NORM_f8          
      nop.i 999
}

{ .mfi
      nop.m 999
(p7)  fms.s1         asin_C   = f1,f1,asin_ABS_NORM_f8          
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p7)  fma.s1         asin_B   = asin_C, asin_D, f0              
      nop.i 999
}

{ .mfi
      nop.m 999
(p7)  fms.s1         asin_1mD = f1,f1,asin_D                    
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p7)  fma.s1         asin_Dd  = asin_1mD,f1, asin_ABS_NORM_f8   
      nop.i 999
}

{ .mfi
      nop.m 999
(p7)  fms.s1         asin_Bb  = asin_C, asin_D, asin_B          
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p7)  fma.s1         asin_Bb  = asin_C, asin_Dd, asin_Bb        
      nop.i 999
}

{ .mfi
      nop.m 999
(p6)  fma.s1         asin_C   = asin_ABS_NORM_f8, asin_ABS_NORM_f8, f0     
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p6)  fms.s1         asin_B   = f1, f1, asin_C                             
      nop.i 999
}

{ .mfi
      nop.m 999
(p6)  fms.s1         asin_Cc  = asin_ABS_NORM_f8, asin_ABS_NORM_f8, asin_C 
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         asin_Hh     = asin_HALF, asin_B, f0                   
      nop.i 999
}

{ .mfi
      nop.m 999
(p6)  fms.s1         asin_1mB = f1, f1, asin_B                             
      nop.i 999 ;;
}

// Step 1.2: 
// sqrt(B + b) is computed as W + w
// Get W

{ .mfi
      nop.m 999
(p0)  frsqrta.s1     asin_y0,p8  = asin_B                                  
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p6)  fms.s1         asin_1mBmC = asin_1mB, f1, asin_C                     
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         asin_t1     = asin_y0, asin_y0, f0                    
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p6)  fms.s1         asin_Bb  = asin_1mBmC, f1, asin_Cc                    
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fnma.s1        asin_t2     = asin_t1, asin_Hh, asin_HALF             
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         asin_y1     = asin_t2, asin_y0, asin_y0               
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         asin_t3     = asin_y1, asin_Hh, f0                    
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fnma.s1        asin_t4     = asin_t3, asin_y1, asin_HALF             
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         asin_y2     = asin_t4, asin_y1, asin_y1               
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         asin_S      = asin_B, asin_y2, f0                     
      nop.i 999
}

{ .mfi
      nop.m 999
(p0)  fma.s1         asin_H      = asin_y2, asin_HALF, f0                  
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         asin_t5     = asin_Hh, asin_y2, f0                    
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fnma.s1        asin_Dd     = asin_S, asin_S, asin_B                  
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         asin_W      = asin_Dd, asin_H, asin_S                 
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         asin_2W       = asin_W, f1, asin_W                    
      nop.i 999
}

// Step 1.3
// Get w
{ .mfi
      nop.m 999
(p0)  fnma.s1        asin_BmWW     = asin_W, asin_W, asin_B                
      nop.i 999 ;;
}

// Step 2
// asin(x) = atan2(X,sqrt(1-X*X))
//         = atan2(X, W) -Xw
// corr = Xw
// asin(x) = Z_hi + (s_lo*Z_lo - corr)
// Call atan2(X, W)
// Save W in f9 
// Save X in f12 
// Save w in f13

{ .mfi
      nop.m 999
(p0)   fmerge.se f9 = asin_W, asin_W                                      
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         asin_BmWWpb   = asin_BmWW, f1, asin_Bb                
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  frcpa.s1       asin_1d2W,p9  = f1, asin_2W                           
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p0)  fma.s1         asin_Ww       = asin_BmWWpb, asin_1d2W, f0            
      nop.i 999 ;;
}
.endp asinl
ASM_SIZE_DIRECTIVE(asinl)

.proc __libm_callout
__libm_callout:
.prologue
{ .mfi
        nop.m 0
        nop.f 0
.save   ar.pfs,GR_SAVE_PFS
        mov  GR_SAVE_PFS=ar.pfs                 // Save ar.pfs
};;
{ .mfi
        mov GR_SAVE_GP=gp                       // Save gp
        nop.f 0
.save   b0, GR_SAVE_B0
        mov GR_SAVE_B0=b0                       // Save b0
}
.body
{.mfb
        nop.m 0
(p0)    fmerge.se f13 = asin_Ww, asin_Ww                                   
(p0)    br.call.sptk.many  b0=__libm_atan2_reg#                  
};;
{ .mfi
        mov   gp = GR_SAVE_GP                  // Restore gp
(p0)    fma.s1  asin_XWw  = asin_ABS_NORM_f8,f13,f0             
        mov   b0 = GR_SAVE_B0                  // Restore return address
};;
// asin_XWw = Xw = corr
// asin_low = (s_lo * Z_lo - corr)
// f8       = Z_hi + (s_lo * Z_lo - corr)

{ .mfi
        nop.m 999
(p0)    fms.s1  asin_low  = f11, f10, asin_XWw                                
        mov   ar.pfs = GR_SAVE_PFS             // Restore ar.pfs
};;

{ .mfi
      nop.m 999
(p0)   fma.s0  f8        = f8, f1, asin_low                                
      nop.i 999 ;;
}

{ .mfb
      nop.m 999
(p0)   fmerge.s f8 = f12,f8 
(p0)  br.ret.sptk   b0 ;;                                                    
}
.endp __libm_callout
ASM_SIZE_DIRECTIVE(__libm_callout)

.proc SPECIAL
SPECIAL:
L(ASIN_ERROR_RETURN): 

// If X is 1, return (sign of X)pi/2

{ .mfi
      nop.m 999
(p0)  fcmp.eq.unc p6,p7 = asin_ABS_NORM_f8,f1   
      nop.i 999 ;;
}

{ .mfb
(p6) ldfe          asin_pi_by_2_lo  = [r40] 
(p6) fmerge.s      asin_pi_by_2 = f8,asin_pi_by_2          
     nop.b 0;;
}

// If X is a NAN, leave
// qnan snan inf norm     unorm 0 -+
// 1    1    0   0        0     0 11
{ .mfb
      nop.m 999
(p6)  fma.s0     f8 = f8,asin_pi_by_2_lo,asin_pi_by_2              
(p6)  br.ret.spnt   b0                           
}
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
(p0)   mov   GR_Parameter_TAG = 60                   
(p0)   frcpa f10, p6 = f0, f0                   
nop.i 0
};;
.endp SPECIAL
ASM_SIZE_DIRECTIVE(SPECIAL)

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
        stfe [GR_Parameter_Y] = FR_Y,16         // Store Parameter 2 on stack
        add GR_Parameter_X = 16,sp              // Parameter 1 address
.save   b0, GR_SAVE_B0
        mov GR_SAVE_B0=b0                       // Save b0
};;
.body
{ .mib
        stfe [GR_Parameter_X] = FR_X            // Store Parameter 1 on stack
        add   GR_Parameter_RESULT = 0,GR_Parameter_Y
        nop.b 0                                 // Parameter 3 address
}
{ .mib
        stfe [GR_Parameter_Y] = FR_RESULT       // Store Parameter 3 on stack
        add   GR_Parameter_Y = -16,GR_Parameter_Y
        br.call.sptk b0=__libm_error_support#   // Call error handling function
};;
{ .mmi
        nop.m 0
        nop.m 0
        add   GR_Parameter_RESULT = 48,sp
};;
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
