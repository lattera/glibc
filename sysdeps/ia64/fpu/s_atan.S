.file "atan.s"

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
// 2/02/00: Initial version
// 4/13/00: Improved speed
// 4/19/00: Removed the qualifying predicate from the fmerge.s that
//          takes the absolute value.
// 6/16/00: Reassigned FP registers to eliminate stalls on loads
// 8/30/00: Saved 5 cycles in main path by rearranging large argument logic
//          and delaying use of result of fcmp in load by 1 group
//
// API
//==============================================================
// double atan( double x);
//
// Overview of operation
//==============================================================
// atan(x) = sign(X)pi/2 - atan(1/x)
//
// We have two paths: |x| > 1 and |x| <= 1
//
// |x| >  1
// ==========================================
//
// c  = frcpa(x) which is approximately 1/x
//
// xc = 1- B
// B  = 1-xc
//
// Approximate 1/(1-B)^k by a polynomial in B, poly(B)
// k is 45.
//
// poly(B)     = 1 + r1 B + r2 B^2 + ...+ r10 B^10
//
// c^k         = (1-B)^k/x^k
// c^k/(1-B)^k = 1/x^k 
// c^k poly(B) = 1/x^k

// poly(x)     = series(atan(1/x)) = 1/x - 1/3x^3 + 1/5x^5 - 1/7x^7 .... + 1/45 x^45
//                                 = 1/x^45 ( x^44 - x^42/3 + x^40/5 - x^38/7 ... +1)
//                                 = 1/x^45 ( y^22 - y^21/3 + y^20/5 - y^19/7 ... +1)
//        
//                                 = c^45 poly(B) poly(x)
//                                 = c^45 r(B) q(y)

// q(y) = q0 + q1 y + q2 y^2 + ... + q22 y^22
// where q22 is 1.0

// atan(x) = sign(X)pi/2 -  c^45 r(B) q(y)

// |x| <=  1
// ==========================================
// poly(x)    = series(atan(x))   = x - x^3/3 + x^5/5 + .....
// poly(x)    = series(atan(x))   = x  + x^3(- 1/3 + x^2/5 + ..... +x^47/47)
// poly(x)    = series(atan(x))   = x  + x^3(p0 + x^2/5 + ..... + x^44/47)
// poly(x)    = series(atan(x))   = x  + x^3(p0 + y/5 + ..... + y^22/47)
   
// where p0 is about -1/3.

// atan(x) = poly(x)

#include "libm_support.h"

// Special Values
//==============================================================
// atan(QNAN)  = QNAN
// atan(SNAN)  = quieted SNAN
// atan(+-inf) = +- pi/2     
// atan(+-0)   = +-0



// Registers used
//==============================================================

// predicate registers used: 
// p6 -> p11

// floating-point registers used:  
// f32 -> f127

// general registers used
// r32 -> r37

// Assembly macros
//==============================================================
atan_Pi_by_2                  = f32
atan_S_PI                     = f33
atan_ABS_f8                   = f34

atan_R0                       = f35
atan_R1                       = f36
atan_R2                       = f37
atan_R3                       = f38 
atan_R4                       = f39 
atan_R5                       = f40 
atan_R6                       = f41
atan_R7                       = f42
atan_R8                       = f43 
atan_R9                       = f44 
atan_R10                      = f45 

atan_Q0                       = f46

atan_Q1                       = f47 
atan_Q2                       = f48
atan_Q3                       = f49
atan_Q4                       = f50
atan_Q5                       = f51 
atan_Q6                       = f52 
atan_Q7                       = f53 
atan_Q8                       = f54 
atan_Q9                       = f55 
atan_Q10                      = f56 

atan_Q11                      = f57 
atan_Q12                      = f58
atan_Q13                      = f59
atan_Q14                      = f60
atan_Q15                      = f61 
atan_Q16                      = f62 
atan_Q17                      = f63 
atan_Q18                      = f64 
atan_Q19                      = f65 
atan_Q20                      = f66 
atan_Q21                      = f67 
atan_Q22                      = f68 

// P and Q constants are mutually exclusive 
// so they can share macro definitions
atan_P0                       = f46

atan_P1                       = f47 
atan_P2                       = f48
atan_P3                       = f49
atan_P4                       = f10
atan_P5                       = f11 
atan_P6                       = f12 
atan_P7                       = f13 
atan_P10                      = f103 

atan_P11                      = f114 
atan_P12                      = f58
atan_P13                      = f59
atan_P14                      = f60
atan_P15                      = f61 
atan_P16                      = f62 
atan_P17                      = f63 
atan_P18                      = f64 
atan_P19                      = f65 
atan_P20                      = f14 
atan_P21                      = f99 
atan_P22                      = f68 
// end of P constant macros

atan_C                        = f69
atan_Y                        = f70
atan_B                        = f71
atan_Z                        = f72
atan_V11                      = f73
atan_V12                      = f74

atan_V7                       = f75
atan_V8                       = f76

atan_W13                      = f77
atan_W11                      = f78

atan_V3                       = f79
atan_V4                       = f80

atan_G11                      = f81
atan_G12                      = f82
atan_G7                       = f83
atan_G8                       = f84

atan_Z1                       = f85
atan_W7                       = f86

atan_G3                       = f87
atan_W8                       = f88
atan_V9                       = f89
atan_V10                      = f90

atan_G10                      = f91
atan_W3                       = f92
atan_G4                       = f93
atan_G9                       = f94
                                
atan_G6                       = f95
atan_W4                       = f96
atan_Z2                       = f97
atan_V6                       = f98
                              
atan_V2                       = f99
atan_W6                       = f100
atan_W10                      = f101
atan_Y3                       = f102
                              
atan_G2                       = f103

atan_Y8                       = f104
                              
atan_G5                       = f105
atan_Z3                       = f106
atan_Z4                       = f107
atan_W2                       = f108
atan_V5                       = f109
                            
atan_W5                       = f110
atan_G1                       = f111
atan_Y11                      = f112
                             
atan_Z5                       = f113
atan_Z6                       = f114
atan_V1                       = f115
atan_W1                       = f116
                              
atan_Z7                       = f117
atan_Q                        = f118
atan_Z                        = f119
atan_abs_f8                   = f120                            

atan_V13                      = f121
atan_Xcub                     = f122
atan_Y12                      = f123
atan_P                        = f124

atan_NORM_f8                  = f125

atan_P8                       = f126
atan_P9                       = f127




atan_GR_AD_R                 = r14
atan_GR_AD_Q                 = r15
atan_GR_AD_P                 = r16
atan_GR_10172                = r17 
atan_GR_exp_f8               = r18 
atan_GR_signexp_f8           = r19
atan_GR_exp_mask             = r20




/////////////////////////////////////////////////////////////


#ifdef _LIBC
.rodata
#else
.data
#endif

.align 16

double_atan_constants_R:
ASM_TYPE_DIRECTIVE(double_atan_constants_R,@object)
   data8 0xB36B46B9C5443CED, 0x0000401C  //R8
   data8 0x842633E0D126261F, 0x0000401F  //R9
   data8 0xBE04FFFFFFFF46E0, 0x00004010  //R4
   data8 0xE8C62000244D66E2, 0x00004013  //R5
   data8 0xF2790C001E3789B3, 0x00004016  //R6
   data8 0xDCD2CCF97D7C764F, 0x00004019  //R7
   data8 0xB40000000000000B, 0x00004004  //R1
   data8 0xB265F3D38F5EE28F, 0x00004021  //R10
   data8 0x8160000000000001, 0x00004009  //R2
   data8 0xFD5BFFFFFFFE55CD, 0x0000400C  //R3
   data8 0xC90FDAA22168C235, 0x00003FFF  // pi/2
ASM_SIZE_DIRECTIVE(double_atan_constants_R)

double_atan_constants_Q:
ASM_TYPE_DIRECTIVE(double_atan_constants_Q,@object)
   data8 0xEBD602FA7761BC33, 0x00003FF9  //Q8
   data8 0x8CB1CABD6A91913C, 0x0000BFFA  //Q9
   data8 0x84C665C37D623CD2, 0x00003FF7  //Q4
   data8 0x8DE0D1673DAEA9BC, 0x0000BFF8  //Q5
   data8 0xF658ADBE2C6E6FCC, 0x00003FF8  //Q6

   data8 0xB56307BE1DD3FFB6, 0x0000BFF9  //Q7
   data8 0xAAAAAAAAAAAA8000, 0x0000BFFD  //Q21
   data8 0x8000000000000000, 0x00003FFF  //Q22
   data8 0x924924923A9D710C, 0x0000BFFC  //Q19
   data8 0xCCCCCCCCCC9380E7, 0x00003FFC  //Q20
 
   data8 0xA644DC250EFA2800, 0x00003FED  //Q0
   data8 0x83DEAE24EEBF5E44, 0x0000BFF1  //Q1
   data8 0xC758CCC64793D4EC, 0x00003FF3  //Q2
   data8 0xBFDC0B54E7C89DCE, 0x0000BFF5  //Q3
   data8 0x888855199D1290AF, 0x0000BFFB  //Q15

   data8 0x9D89D3BE514B0178, 0x00003FFB  //Q16
   data8 0xBA2E8B4DEC70282A, 0x0000BFFB  //Q17
   data8 0xE38E38DF9E9FC83B, 0x00003FFB  //Q18
   data8 0x9F8781CC990029D9, 0x00003FFA  //Q10
   data8 0xB0B39472DEBA3C79, 0x0000BFFA  //Q11

   data8 0xC2AFAEF8C85B0BC6, 0x00003FFA  //Q12
   data8 0xD780E539797525DD, 0x0000BFFA  //Q13
   data8 0xF0EDC449AC786DF9, 0x00003FFA  //Q14
ASM_SIZE_DIRECTIVE(double_atan_constants_Q)



double_atan_constants_P:
ASM_TYPE_DIRECTIVE(double_atan_constants_P,@object)
   data8 0xB1899EC590CDB8DF, 0x0000BFFA //P10
   data8 0xA1E79850A67D59B0, 0x00003FFA //P11
   data8 0x911D8B30C2A96E6D, 0x0000BFF3 //P20
   data8 0xB87233C68A640706, 0x00003FF0 //P21
   data8 0xD78E4B82F3C29D7A, 0x0000BFFA //P8

   data8 0xC2EBE37AF932C14F, 0x00003FFA //P9
   data8 0xBA2E8B94AA104DD6, 0x0000BFFB //P4
   data8 0x9D89D7A640B71D38, 0x00003FFB //P5
   data8 0x88887CA2CE9B2A40, 0x0000BFFB //P6
   data8 0xF0F017D57A919C1E, 0x00003FFA //P7

   data8 0xD0D635F230C80E06, 0x0000BFF8 //P16
   data8 0xE847BECA7209B479, 0x00003FF7 //P17
   data8 0xD14C6A2AAE0D5B07, 0x0000BFF6 //P18
   data8 0x915F612A5C469117, 0x00003FF5 //P19
   data8 0x921EDE5FD0DBBBE2, 0x0000BFFA //P12

   data8 0xFFD303C2C8535445, 0x00003FF9 //P13
   data8 0xD30DF50E295386F7, 0x0000BFF9 //P14
   data8 0x9E81F2B1BBD210A8, 0x00003FF9 //P15
   data8 0xAAAAAAAAAAAAA800, 0x0000BFFD //P0
   data8 0xCCCCCCCCCCC7D476, 0x00003FFC //P1

   data8 0x9249249247838066, 0x0000BFFC //P2
   data8 0xE38E38E302290D68, 0x00003FFB //P3
   data8 0xDF7F0A816F7E5025, 0x0000BFEC //P22
ASM_SIZE_DIRECTIVE(double_atan_constants_P)


.align 32
.global atan#

////////////////////////////////////////////////////////



.section .text
.proc  atan#
.align 32

atan: 

{ .mmf
(p0)  addl      atan_GR_AD_P   = @ltoff(double_atan_constants_P), gp
(p0)  addl      atan_GR_AD_Q   = @ltoff(double_atan_constants_Q), gp
(p0)  fmerge.s  atan_ABS_f8  = f0,f8                       
}
;;

{ .mmf
      ld8 atan_GR_AD_P = [atan_GR_AD_P]
      ld8 atan_GR_AD_Q = [atan_GR_AD_Q]
(p0)  frcpa.s1     atan_C,p8 = f1,f8                                  
}
;;

{ .mmf
(p0)  addl      atan_GR_AD_R   = @ltoff(double_atan_constants_R), gp
(p0)  addl      atan_GR_exp_mask = 0x1ffff, r0
(p0)  fma.s1       atan_Y = f8,f8,f0                                  
}
;;

// This fnorm takes faults or sets fault flags
{ .mmf
(p0)  mov       atan_GR_10172 = 0x10172
      ld8 atan_GR_AD_R = [atan_GR_AD_R]
(p0)  fnorm     atan_NORM_f8  = f8
}
;;


// qnan snan inf norm     unorm 0 -+
// 1    1    0   0        0     1 11
// c                      7

// p9 set if we have a NAN or +-0

{ .mmf
(p0)  ldfe      atan_Q8  = [atan_GR_AD_Q],16 
(p0)  ldfe      atan_P10    = [atan_GR_AD_P],16                   
(p0)  fclass.m.unc p9, p0   = f8, 0xc7                         
}
;;


{ .mmi
(p0)  ldfe      atan_Q9  = [atan_GR_AD_Q],16 
(p0)  ldfe      atan_P11    = [atan_GR_AD_P],16                   
      nop.i 999   
}
;;

        
{ .mmf
(p0)  ldfe      atan_Q4      = [atan_GR_AD_Q],16 
(p0)  ldfe      atan_P20     = [atan_GR_AD_P],16                   
(p9)  fma.d.s0  f8          = f8,f1,f0
;;
}

// Exit if we have a NAN or +-0
{ .mmb
(p0)  ldfe      atan_Q5     = [atan_GR_AD_Q],16 
(p0)  ldfe      atan_P21    = [atan_GR_AD_P],16                   
(p9)  br.ret.spnt b0
;;
}


// p6 is TRUE if |x| <= 1
// p7 is TRUE if |x| >  1
{ .mmf
(p0)  ldfe      atan_Q6  = [atan_GR_AD_Q],16 
(p0)  ldfe      atan_P8  = [atan_GR_AD_P],16                   
(p0)  fcmp.le.unc  p6,p7 = atan_ABS_f8, f1 
;;
}


{ .mfi
(p0)  ldfe         atan_Q7  = [atan_GR_AD_Q],16 
(p0)  fma.s1       atan_Z   = atan_C, atan_C, f0                        
      nop.i 999   
}
{ .mfi
(p0)  ldfe         atan_P9  = [atan_GR_AD_P],16                   
(p0)  fnma.s1      atan_B   = atan_C,f8, f1                             
      nop.i 999    ;;
}


{ .mfi
(p0)  ldfe         atan_Q21  = [atan_GR_AD_Q],16 
(p0)  fma.s1       atan_V12  = atan_Y, atan_Y, f0                      
      nop.i 999   
}
{ .mfi
(p0)  ldfe         atan_P4    = [atan_GR_AD_P],16                   
(p0)  fma.s1       atan_Xcub  = f8, atan_Y  , f0                        
      nop.i 999   
;;
}


{ .mmi
(p7)  ldfe      atan_Q22         = [atan_GR_AD_Q],16 
(p6)  ldfe      atan_P5          = [atan_GR_AD_P],16                   
(p6)  cmp.eq.unc  p8,p0 = r0,r0
;;
}


{ .mmi
(p7)  ldfe      atan_Q19   = [atan_GR_AD_Q],16 
(p6)  ldfe      atan_P6    = [atan_GR_AD_P],16                   
(p7)  cmp.eq.unc  p9,p0 = r0,r0
;;
}


{ .mmi
(p7)  ldfe      atan_Q20  = [atan_GR_AD_Q],16 
(p6)  ldfe      atan_P7    = [atan_GR_AD_P],16                   
      nop.i 999   
;;
}

{ .mfi
(p7)  ldfe      atan_Q0  = [atan_GR_AD_Q],16 
(p6)  fma.s1       atan_V13 = atan_Y, atan_P11, atan_P10              
      nop.i 999   
}
{ .mfi
(p6)  ldfe      atan_P16    = [atan_GR_AD_P],16                   
(p7)  fma.s1       atan_V11 = atan_Y, atan_Q9, atan_Q8                
      nop.i 999    ;;
}


{ .mfi
(p7)  ldfe      atan_Q1  = [atan_GR_AD_Q],16 
(p7)  fma.s1       atan_G12 = atan_B, atan_B, f0                      
      nop.i 999   
}
{ .mfi
(p6)  ldfe      atan_P17    = [atan_GR_AD_P],16                   
(p0)  fma.s1       atan_V9  = atan_V12, atan_V12, f0                  
      nop.i 999    ;;
}


{ .mfi
(p7)  ldfe      atan_Q2  = [atan_GR_AD_Q],16 
(p6)  fma.s1       atan_W11 = atan_Y, atan_P21, atan_P20              
      nop.i 999   
}
{ .mfi
(p6)  ldfe      atan_P18    = [atan_GR_AD_P],16                   
(p7)  fma.s1       atan_V7  = atan_Y, atan_Q5, atan_Q4                
      nop.i 999    ;;
}

{ .mfi
(p7)  ldfe      atan_Q3  = [atan_GR_AD_Q],16 
(p7)  fma.s1    atan_Z1  = atan_Z, atan_Z, f0                      
      nop.i 999   
}
{ .mfi
(p6)  ldfe      atan_P19    = [atan_GR_AD_P],16                   
(p7)  fma.s1       atan_Y3  = atan_Y  , atan_V12, f0                  
      nop.i 999    ;;
}

{ .mfi
(p7)  ldfe      atan_R8  = [atan_GR_AD_R],16 
(p6)  fma.s1       atan_V11 = atan_Y, atan_P9, atan_P8                
      nop.i 999   
}
{ .mfi
(p6)  ldfe      atan_P12    = [atan_GR_AD_P],16                   
(p7)  fma.s1       atan_V8  = atan_Y, atan_Q7, atan_Q6                
      nop.i 999    ;;
}

{ .mmi
(p7)  ldfe      atan_R9     = [atan_GR_AD_R],16 
(p6)  ldfe      atan_P13    = [atan_GR_AD_P],16                   
      nop.i 999   
;;
}

{ .mfi
(p7)  ldfe      atan_R4  = [atan_GR_AD_R],16 
(p6)  fma.s1    atan_V7  = atan_Y, atan_P5, atan_P4                 
      nop.i 999   
}
{ .mfi
(p6)  ldfe      atan_P14    = [atan_GR_AD_P],16                   
(p7)  fma.s1       atan_W13 = atan_Y, atan_Q22, atan_Q21              
      nop.i 999    ;;
}


{ .mfi
(p7)  ldfe      atan_R5  = [atan_GR_AD_R],16 
(p6)  fma.s1    atan_Y12 = atan_V9 , atan_V9 , f0                   
      nop.i 999   
}
{ .mfi
(p6)  ldfe      atan_P15    = [atan_GR_AD_P],16                   
(p7)  fma.s1    atan_Y8 = atan_V9 , atan_V9 , f0                   
      nop.i 999    ;;
}


{ .mfi
(p7)  ldfe      atan_R6  = [atan_GR_AD_R],16 
(p6)  fma.s1    atan_V8  = atan_Y, atan_P7, atan_P6                 
      nop.i 999   
}
{ .mfi
(p6)  ldfe      atan_P0     = [atan_GR_AD_P],16                   
(p7)  fma.s1       atan_W11 = atan_Y, atan_Q20, atan_Q19              
      nop.i 999    ;;
}


{ .mfi
(p7)  ldfe      atan_R7  = [atan_GR_AD_R],16 
(p7)  fma.s1    atan_Z2  = atan_Z1 , atan_Z1,  f0                  
      nop.i 999   
}
{ .mfi
(p6)  ldfe      atan_P1     = [atan_GR_AD_P],16                   
(p6)  fma.s1       atan_V10 = atan_V12, atan_V13, atan_V11             
      nop.i 999    ;;
}

{ .mfi
(p7)  ldfe      atan_Q15 = [atan_GR_AD_Q],16 
(p6)  fma.s1    atan_W7  = atan_Y, atan_P17, atan_P16               
      nop.i 999   
}
{ .mfi
(p6)  ldfe      atan_P2     = [atan_GR_AD_P],16                   
(p7)  fma.s1       atan_V3  = atan_Y, atan_Q1 , atan_Q0               
      nop.i 999    ;;
}

{ .mfi
(p7)  ldfe      atan_Q16 = [atan_GR_AD_Q],16 
(p7)  fma.s1    atan_G9  = atan_G12, atan_G12, f0                  
      nop.i 999   
}
{ .mfi
(p6)  ldfe      atan_P3     = [atan_GR_AD_P],16                   
(p7)  fma.s1       atan_V6  = atan_V12, atan_V8,  atan_V7             
      nop.i 999    ;;
}


{ .mfi
(p7)  ldfe      atan_R1     = [atan_GR_AD_R],16 
(p6)  fma.s1       atan_W8  = atan_Y, atan_P19, atan_P18               
      nop.i 999   
}
{ .mfi
(p6)  ldfe      atan_P22    = [atan_GR_AD_P],16                   
(p7)  fma.s1       atan_V4  = atan_Y, atan_Q3 , atan_Q2               
      nop.i 999    ;;
}


{ .mfi
      getf.exp     atan_GR_signexp_f8  = atan_NORM_f8
(p7)  fma.s1       atan_Y11 = atan_Y3 , atan_Y8 , f0                  
      nop.i 999   
}
{ .mfi
(p7)  ldfe      atan_Q17    = [atan_GR_AD_Q],16 
(p6)  fma.s1       atan_V6  = atan_V12, atan_V8,  atan_V7             
      nop.i 999    ;;
}


{ .mfi
(p7)  ldfe      atan_Q18    = [atan_GR_AD_Q],16 
(p6)  fma.s1       atan_W3  = atan_Y, atan_P13, atan_P12               
      nop.i 999   
}
{ .mfi
(p7)  ldfe      atan_R10    = [atan_GR_AD_R],16 
(p7)  fma.s1       atan_G11 = atan_B, atan_R9 , atan_R8               
      nop.i 999    ;;
}


{ .mfi
(p7)  ldfe      atan_Q10    = [atan_GR_AD_Q],16 
(p7)  fma.s1    atan_Z3     = atan_Z1 , atan_Z2 , f0                  
(p0)  and       atan_GR_exp_f8 = atan_GR_signexp_f8,atan_GR_exp_mask
}
{ .mfi
(p7)  ldfe      atan_R2     = [atan_GR_AD_R],16 
(p7)  fma.s1       atan_Z4  = atan_Z2 , atan_Z2 , f0                  
      nop.i 999    ;;
}


{ .mfi
(p7)  ldfe      atan_Q11 = [atan_GR_AD_Q],16 
(p6)  fma.s1    atan_W4  = atan_Y, atan_P15, atan_P14               
      nop.i 999
}
{ .mfi
(p7)  ldfe      atan_R3     = [atan_GR_AD_R],16 
(p7)  fma.s1       atan_G7  = atan_B, atan_R5 , atan_R4               
(p0)  cmp.le.unc   p11,p0  = atan_GR_10172,atan_GR_exp_f8 
;;
}


{ .mmf
(p9)  ldfe      atan_Q12 = [atan_GR_AD_Q],16 
(p0)  ldfe      atan_S_PI     = [atan_GR_AD_R],16 
(p8)  fma.s1       atan_W6  = atan_V12, atan_W8,  atan_W7             
;;
}



{ .mfi
(p9)  ldfe      atan_Q13 = [atan_GR_AD_Q],16 
(p8)  fma.s1       atan_V3  = atan_Y, atan_P1 , atan_P0                
(p11) cmp.ne.and p6,p7 = r0,r0
}
{ .mfi
      nop.m 999
(p8)  fma.s1       atan_V5  = atan_V9 , atan_V10, atan_V6             
      nop.i 999    ;;
}


.pred.rel "mutex",p6,p7,p11
{ .mfi
(p7)  ldfe      atan_Q14 = [atan_GR_AD_Q],16 
(p6)  fma.s1       atan_Y12 = atan_V9 , atan_Y12, f0                   
      nop.i 999    
}
{ .mfi
      nop.m 999
(p7)  fma.s1    atan_G8  = atan_B, atan_R7 , atan_R6               
      nop.i 999    ;;
}


{ .mfi
      nop.m 999
(p6)  fma.s1       atan_V4  = atan_Y, atan_P3 , atan_P2                
      nop.i 999    
}
{ .mfi
      nop.m 999
(p7)  fma.s1       atan_W7  = atan_Y, atan_Q16, atan_Q15              
      nop.i 999    ;;
}


{ .mfi
      nop.m 999
(p6)  fma.s1       atan_W10 = atan_V12, atan_P22, atan_W11            
      nop.i 999    
}
{ .mfi
      nop.m 999
(p7)  fma.s1       atan_G3  = atan_B, atan_R1 , f1                    
      nop.i 999    ;;
}


{ .mfi
      nop.m 999
(p6)  fma.s1    atan_W2  = atan_V12, atan_W4 , atan_W3             
      nop.i 999    
}
{ .mfi
      nop.m 999
(p7)  fma.s1       atan_V2  = atan_V12, atan_V4 , atan_V3              
      nop.i 999    ;;
}

{ .mfi
      nop.m 999
(p7)  fma.s1       atan_W8  = atan_Y, atan_Q18, atan_Q17              
      nop.i 999   
}
{ .mfi
      nop.m 999
(p7)  fma.s1       atan_G10 = atan_G12, atan_R10, atan_G11            
      nop.i 999    ;;
}

{ .mfi
      nop.m 999
(p7)  fma.s1       atan_V10 = atan_V12, atan_Q10, atan_V11            
      nop.i 999    
}
{ .mfi
      nop.m 999
(p7)  fma.s1       atan_G6  = atan_G12, atan_G8 , atan_G7             
      nop.i 999    ;;
}


{ .mfi
      nop.m 999
(p6)  fma.s1       atan_V2  = atan_V12, atan_V4,  atan_V3             
      nop.i 999    
}
{ .mfi
      nop.m 999
(p7)  fma.s1       atan_G4  = atan_B  , atan_R3 , atan_R2             
      nop.i 999    ;;
}


{ .mfi
      nop.m 999
(p6)  fma.s1       atan_W5  = atan_V9 , atan_W10, atan_W6             
      nop.i 999    
}
{ .mfi
      nop.m 999
(p7)  fma.s1       atan_W3  = atan_Y  , atan_Q12, atan_Q11            
      nop.i 999    ;;
}


{ .mfi
      nop.m 999
(p7)  fma.s1       atan_Z5  = atan_Z3 , atan_Z4 , f0                  
      nop.i 999    
}
{ .mfi
      nop.m 999
(p7)  fma.s1       atan_W10 = atan_V12, atan_W13, atan_W11            
      nop.i 999    ;;
}


{ .mfi
      nop.m 999
(p7)  fma.s1       atan_W4  = atan_Y  , atan_Q14, atan_Q13            
      nop.i 999    
}
{ .mfi
      nop.m 999
(p7)  fma.s1       atan_W6  = atan_V12, atan_W8,  atan_W7             
      nop.i 999    ;;
}

{ .mfi
      nop.m 999
(p7)  fma.s1       atan_V5  = atan_V9 , atan_V10, atan_V6             
      nop.i 999    
}
{ .mfi
      nop.m 999
(p7)  fma.s1       atan_G5  = atan_G9 , atan_G10, atan_G6             
      nop.i 999    ;;
}


{ .mfi
      nop.m 999
(p6)  fma.s1       atan_V1  = atan_V9 , atan_V5 , atan_V2             
      nop.i 999    
}
{ .mfi
      nop.m 999
(p7)  fma.s1       atan_G2  = atan_G12, atan_G4 , atan_G3             
      nop.i 999    ;;
}


{ .mfi
      nop.m 999
(p6)  fma.s1       atan_W1  = atan_V9 , atan_W5 , atan_W2             
      nop.i 999    
}
{ .mfi
      nop.m 999
(p7)  fma.s1       atan_Z6  = atan_Z4 , atan_C  , f0                  
      nop.i 999    ;;
}

{ .mfi
      nop.m 999
(p0)  fmerge.s    atan_S_PI = f8, atan_S_PI
      nop.i 999     ;;
}


{ .mfi
      nop.m 999
(p7)  fma.s1       atan_W5  = atan_V9 , atan_W10, atan_W6             
      nop.i 999    
}
{ .mfi
      nop.m 999
(p7)  fma.s1       atan_W2  = atan_V12, atan_W4 , atan_W3             
      nop.i 999    ;;
}


{ .mfi
      nop.m 999
(p7)  fma.s1       atan_G1  = atan_G9 , atan_G5 , atan_G2             
      nop.i 999    
}
{ .mfi
      nop.m 999
(p7)  fma.s1       atan_V1  = atan_V9 , atan_V5 , atan_V2             
      nop.i 999    ;;
}


{ .mfi
      nop.m 999
(p6)  fma.s1       atan_P   = atan_Y12, atan_W1 , atan_V1              
      nop.i 999    
}
{ .mfi
      nop.m 999
(p7)  fma.s1       atan_Z7  = atan_Z5 , atan_Z6 , f0                  
      nop.i 999    ;;
}


{ .mfi
      nop.m 999
(p7)  fma.s1       atan_W1  = atan_V9 , atan_W5 , atan_W2             
      nop.i 999    ;;
}


{ .mfi
      nop.m 999
(p11) fma.d.s0  f8 = atan_S_PI,f1,f0
      nop.i 999
}
{ .mfi
      nop.m 999
(p7)  fma.s1       atan_Z   = atan_G1 , atan_Z7 , f0                  
      nop.i 999    ;;
}


{ .mfi
      nop.m 999
(p7)  fma.s1       atan_Q   = atan_Y11, atan_W1 , atan_V1             
      nop.i 999    ;;
}


{ .mfi
      nop.m 999
(p6)  fma.d.s0    f8       = atan_P  , atan_Xcub  , f8               
      nop.i 999    
}
{ .mfb
      nop.m 999
(p7)  fnma.d.s0    f8       = atan_Z  , atan_Q  , atan_S_PI           
(p0)  br.ret.sptk    b0    ;;
}

.endp atan
ASM_SIZE_DIRECTIVE(atan)
