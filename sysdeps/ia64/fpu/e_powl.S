.file "powl.s"

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
// *********************************************************************
//
// Function:   powl(x,y), where
//                         y
//             powl(x,y) = x , for double extended precision x and y values
//
// *********************************************************************
//
// History: 
// 2/02/00  (Hand Optimized)
// 4/04/00  Unwind support added
// 8/15/00  Bundle added after call to __libm_error_support to properly
//          set [the previously overwritten] GR_Parameter_RESULT.
// 1/22/01  Corrected results for powl(1,inf), powl(1,nan), and
//          powl(snan,0) to be 1 per C99, not nan.  Fixed many flag settings.
// 2/06/01  Call __libm_error support if over/underflow when y=2.
//
// *********************************************************************
//
// Resources Used:
//
//    Floating-Point Registers: 
//                        f8  (Input and Return Value)
//                        f9-f15,f32-f63,f99 
//
//    General Purpose Registers:
//                        Locals r32 - r61
//                        Parameters to __libm_error_support r62,r63,r64,r65
//
//    Predicate Registers: p6-p15
//
// *********************************************************************
//
//  Special Cases and IEEE special conditions:
//
//    Denormal fault raised on denormal inputs
//    Overflow exceptions raised when appropriate for pow 
//    Underflow exceptions raised when appropriate for pow 
//    (Error Handling Routine called for overflow and Underflow)
//    Inexact raised when appropriate by algorithm
//
//  1.  (anything) ** NatVal or (NatVal) ** anything  is NatVal
//  2.  X or Y unsupported or sNaN                    is qNaN/Invalid
//  3.  (anything) ** 0  is 1
//  4.  (anything) ** 1  is itself
//  5.  (anything except 1) ** qNAN is qNAN
//  6.  qNAN ** (anything except 0) is qNAN
//  7.  +-(|x| > 1) **  +INF is +INF
//  8.  +-(|x| > 1) **  -INF is +0
//  9.  +-(|x| < 1) **  +INF is +0
//  10. +-(|x| < 1) **  -INF is +INF
//  11. +-1         ** +-INF is +1
//  12. +0 ** (+anything except 0, NAN)               is +0
//  13. -0 ** (+anything except 0, NAN, odd integer)  is +0
//  14. +0 ** (-anything except 0, NAN)               is +INF/div_0
//  15. -0 ** (-anything except 0, NAN, odd integer)  is +INF/div_0
//  16. -0 ** (odd integer) = -( +0 ** (odd integer) )
//  17. +INF ** (+anything except 0,NAN) is +INF
//  18. +INF ** (-anything except 0,NAN) is +0
//  19. -INF ** (anything except NAN)  = -0 ** (-anything)
//  20. (-anything) ** (integer) is (-1)**(integer)*(+anything**integer)
//  21. (-anything except 0 and inf) ** (non-integer) is qNAN/Invalid
//  22. X or Y denorm/unorm and denorm/unorm operand trap is enabled,
//      generate denorm/unorm fault except if invalid or div_0 raised.
//
// *********************************************************************
// 
//  Algorithm
//  =========
//
//  Special Cases
//
//    If Y = 2,    return X*X.
//    If Y = 0.5,  return sqrt(X).
//
//  Compute log(X) to extra precision.
//  
//  ker_log_80( X, logX_hi, logX_lo, Safe );
//
//   ...logX_hi + logX_lo approximates log(X) to roughly 80 
//   ...significant bits of accuracy.
//
//  Compute Y*log(X) to extra precision.
//
//    P_hi := Y * logX_hi
//    P_lo := Y * logX_hi - P_hi	...using FMA
//    P_lo := Y * logX_lo + P_lo	...using FMA
//
//  Compute exp(P_hi + P_lo)
//
//    Flag := 2; 
//    Expo_Range := 2; (assuming double-extended power function)
//    ker_exp_64( P_hi, P_lo, Flag, Expo_Range, 
//                Z_hi, Z_lo, scale, Safe )
//
//    scale := sgn * scale
//
//    If (Safe) then ...result will not over/underflow
//       return scale*Z_hi + (scale*Z_lo)
//       quickly
//    Else
//       take necessary precaution in computing 
//       scale*Z_hi + (scale*Z_lo)
//       to set possible exceptions correctly.
//    End If
//
//  Case_Y_Special
//
//   ...Follow the order of the case checks
//
//   If Y is +-0, return +1 without raising any exception.
//   If Y is +1,  return X  without raising any exception.
//   If Y is qNaN, return Y without exception.
//   If X is qNaN, return X without exception.
//
//   At this point, X is real and Y is +-inf. 
//   Thus |X| can only be 1, strictly bigger than 1, or 
//   strictly less than 1.
//
//   If |X| < 1, then
//   return ( Y == +inf?  +0 : +inf )
//   elseif |X| > 1, then
//   return ( Y == +inf? +0 : +inf )
//   else
//   goto Case_Invalid
//
//  Case_X_Special
//
//   ...Follow the order of the case checks
//   ...Note that Y is real, finite, non-zero, and not +1.
//
//   If X is qNaN, return X without exception.
//    
//   If X is +-0, 
//   return ( Y > 0 ? +0 : +inf )
//
//   If X is +inf
//   return ( Y > 0 ? +inf : +0 )
//
//   If X is -inf
//   return -0 ** -Y
//   return ( Y > 0 ? +inf : +0 )
//
//  Case_Invalid 
//
//   Return 0 * inf to generate a quiet NaN together
//   with an invalid exception.
// 
//  Implementation
//  ==============
//
//   We describe the quick branch since this part is important
//   in reaching the normal case efficiently.
//
//  STAGE 1
//  -------
//   This stage contains two threads. 
//
//   Stage1.Thread1
//
//     fclass.m   X_excep,  X_ok   = X, (NatVal or s/qNaN) or
//				  +-0, +-infinity
//
//     fclass.nm  X_unsupp, X_supp = X, (NatVal or s/qNaN) or
//				  +-(0, unnorm, norm, infinity)
//
//     X_norm := fnorm( X ) with traps disabled
//
//     If (X_excep)  goto Filtering (Step 2)
//     If (X_unsupp) goto Filtering (Step 2)
//
//     Stage1.Thread2
//     ..............    
//
//     fclass.m   Y_excep,  Y_ok   = Y, (NatVal or s/qNaN) or
//				  +-0, +-infinity
//
//     fclass.nm  Y_unsupp, Y_supp = Y, (NatVal or s/qNaN) or
//				  +-(0, unnorm, norm, infinity)
//
//     Y_norm := fnorm( Y ) with traps disabled
//
//     If (Y_excep)  goto Filtering (Step 2)
//     If (Y_unsupp) goto Filtering (Step 2)
//
// 
//  STAGE 2
//  -------
//  This stage contains two threads.
//
//     Stage2.Thread1		
//     ..............	
//
//     Set X_lt_0 if X < 0 (using fcmp)
//     sgn := +1.0
//     If (X_lt_0) goto Filtering (Step 2)
//
//     Stage2.Thread2
//     ..............
//
//     Set Y_is_1 if Y = +1 (using fcmp)
//     If (Y_is_1) goto Filtering (Step 2)
//
//   STAGE 3
//   -------
//   This stage contains two threads.
//
//
//   Stage3.Thread1		
//   .............. 	
//
//     X := fnorm(X) in prevailing traps
//
//
//     Stage3.Thread2		
//     ..............	
//
//     Y := fnorm(Y) in prevailing traps
//
//   STAGE 4
//   -------
//
//   Go to Case_Normal.
//

#include "libm_support.h"

#ifdef _LIBC
.rodata
#else
.data
#endif

// Inv_L, L_hi, L_lo 
.align 64
Constants_exp_64_Arg:
ASM_TYPE_DIRECTIVE(Constants_exp_64_Arg,@object)
data4 0x5C17F0BC,0xB8AA3B29,0x0000400B,0x00000000 
data4 0x00000000,0xB17217F4,0x00003FF2,0x00000000
data4 0xF278ECE6,0xF473DE6A,0x00003FD4,0x00000000
ASM_SIZE_DIRECTIVE(Constants_exp_64_Arg)

.align 64
Constants_exp_64_Exponents:
ASM_TYPE_DIRECTIVE(Constants_exp_64_Exponents,@object)
data4 0x0000007E,0x00000000,0xFFFFFF83,0xFFFFFFFF
data4 0x000003FE,0x00000000,0xFFFFFC03,0xFFFFFFFF
data4 0x00003FFE,0x00000000,0xFFFFC003,0xFFFFFFFF
data4 0x00003FFE,0x00000000,0xFFFFC003,0xFFFFFFFF
data4 0xFFFFFFE2,0xFFFFFFFF,0xFFFFFFC4,0xFFFFFFFF
data4 0xFFFFFFBA,0xFFFFFFFF,0xFFFFFFBA,0xFFFFFFFF
ASM_SIZE_DIRECTIVE(Constants_exp_64_Exponents)

.align 64
Constants_exp_64_A:
ASM_TYPE_DIRECTIVE(Constants_exp_64_A,@object)
// Reversed 
data4 0xB1B736A0,0xAAAAAAAB,0x00003FFA,0x00000000
data4 0x90CD6327,0xAAAAAAAB,0x00003FFC,0x00000000
data4 0xFFFFFFFF,0xFFFFFFFF,0x00003FFD,0x00000000
ASM_SIZE_DIRECTIVE(Constants_exp_64_A)

.align 64
Constants_exp_64_P:
ASM_TYPE_DIRECTIVE(Constants_exp_64_P,@object)
// Reversed 
data4 0x43914A8A,0xD00D6C81,0x00003FF2,0x00000000
data4 0x30304B30,0xB60BC4AC,0x00003FF5,0x00000000
data4 0x7474C518,0x88888888,0x00003FF8,0x00000000
data4 0x8DAE729D,0xAAAAAAAA,0x00003FFA,0x00000000
data4 0xAAAAAF61,0xAAAAAAAA,0x00003FFC,0x00000000
data4 0x000004C7,0x80000000,0x00003FFE,0x00000000
ASM_SIZE_DIRECTIVE(Constants_exp_64_P)

.align 64
Constants_exp_64_T1:
ASM_TYPE_DIRECTIVE(Constants_exp_64_T1,@object)
data4 0x3F800000,0x3F8164D2,0x3F82CD87,0x3F843A29 
data4 0x3F85AAC3,0x3F871F62,0x3F88980F,0x3F8A14D5 
data4 0x3F8B95C2,0x3F8D1ADF,0x3F8EA43A,0x3F9031DC
data4 0x3F91C3D3,0x3F935A2B,0x3F94F4F0,0x3F96942D
data4 0x3F9837F0,0x3F99E046,0x3F9B8D3A,0x3F9D3EDA
data4 0x3F9EF532,0x3FA0B051,0x3FA27043,0x3FA43516
data4 0x3FA5FED7,0x3FA7CD94,0x3FA9A15B,0x3FAB7A3A
data4 0x3FAD583F,0x3FAF3B79,0x3FB123F6,0x3FB311C4
data4 0x3FB504F3,0x3FB6FD92,0x3FB8FBAF,0x3FBAFF5B
data4 0x3FBD08A4,0x3FBF179A,0x3FC12C4D,0x3FC346CD
data4 0x3FC5672A,0x3FC78D75,0x3FC9B9BE,0x3FCBEC15
data4 0x3FCE248C,0x3FD06334,0x3FD2A81E,0x3FD4F35B
data4 0x3FD744FD,0x3FD99D16,0x3FDBFBB8,0x3FDE60F5
data4 0x3FE0CCDF,0x3FE33F89,0x3FE5B907,0x3FE8396A
data4 0x3FEAC0C7,0x3FED4F30,0x3FEFE4BA,0x3FF28177
data4 0x3FF5257D,0x3FF7D0DF,0x3FFA83B3,0x3FFD3E0C
ASM_SIZE_DIRECTIVE(Constants_exp_64_T1)

.align 64
Constants_exp_64_T2:
ASM_TYPE_DIRECTIVE(Constants_exp_64_T2,@object)
data4 0x3F800000,0x3F80058C,0x3F800B18,0x3F8010A4 
data4 0x3F801630,0x3F801BBD,0x3F80214A,0x3F8026D7 
data4 0x3F802C64,0x3F8031F2,0x3F803780,0x3F803D0E 
data4 0x3F80429C,0x3F80482B,0x3F804DB9,0x3F805349 
data4 0x3F8058D8,0x3F805E67,0x3F8063F7,0x3F806987 
data4 0x3F806F17,0x3F8074A8,0x3F807A39,0x3F807FCA 
data4 0x3F80855B,0x3F808AEC,0x3F80907E,0x3F809610 
data4 0x3F809BA2,0x3F80A135,0x3F80A6C7,0x3F80AC5A 
data4 0x3F80B1ED,0x3F80B781,0x3F80BD14,0x3F80C2A8 
data4 0x3F80C83C,0x3F80CDD1,0x3F80D365,0x3F80D8FA 
data4 0x3F80DE8F,0x3F80E425,0x3F80E9BA,0x3F80EF50 
data4 0x3F80F4E6,0x3F80FA7C,0x3F810013,0x3F8105AA 
data4 0x3F810B41,0x3F8110D8,0x3F81166F,0x3F811C07 
data4 0x3F81219F,0x3F812737,0x3F812CD0,0x3F813269 
data4 0x3F813802,0x3F813D9B,0x3F814334,0x3F8148CE 
data4 0x3F814E68,0x3F815402,0x3F81599C,0x3F815F37
ASM_SIZE_DIRECTIVE(Constants_exp_64_T2)
  
.align 64
Constants_exp_64_W1:
ASM_TYPE_DIRECTIVE(Constants_exp_64_W1,@object)
data4 0x00000000,0x00000000,0x171EC4B4,0xBE384454
data4 0x4AA72766,0xBE694741,0xD42518F8,0xBE5D32B6
data4 0x3A319149,0x3E68D96D,0x62415F36,0xBE68F4DA
data4 0xC9C86A3B,0xBE6DDA2F,0xF49228FE,0x3E6B2E50
data4 0x1188B886,0xBE49C0C2,0x1A4C2F1F,0x3E64BFC2
data4 0x2CB98B54,0xBE6A2FBB,0x9A55D329,0x3E5DC5DE
data4 0x39A7AACE,0x3E696490,0x5C66DBA5,0x3E54728B
data4 0xBA1C7D7D,0xBE62B0DB,0x09F1AF5F,0x3E576E04
data4 0x1A0DD6A1,0x3E612500,0x795FBDEF,0xBE66A419
data4 0xE1BD41FC,0xBE5CDE8C,0xEA54964F,0xBE621376
data4 0x476E76EE,0x3E6370BE,0x3427EB92,0x3E390D1A 
data4 0x2BF82BF8,0x3E1336DE,0xD0F7BD9E,0xBE5FF1CB 
data4 0x0CEB09DD,0xBE60A355,0x0980F30D,0xBE5CA37E 
data4 0x4C082D25,0xBE5C541B,0x3B467D29,0xBE5BBECA 
data4 0xB9D946C5,0xBE400D8A,0x07ED374A,0xBE5E2A08 
data4 0x365C8B0A,0xBE66CB28,0xD3403BCA,0x3E3AAD5B 
data4 0xC7EA21E0,0x3E526055,0xE72880D6,0xBE442C75 
data4 0x85222A43,0x3E58B2BB,0x522C42BF,0xBE5AAB79 
data4 0x469DC2BC,0xBE605CB4,0xA48C40DC,0xBE589FA7 
data4 0x1AA42614,0xBE51C214,0xC37293F4,0xBE48D087 
data4 0xA2D673E0,0x3E367A1C,0x114F7A38,0xBE51BEBB 
data4 0x661A4B48,0xBE6348E5,0x1D3B9962,0xBDF52643  
data4 0x35A78A53,0x3E3A3B5E,0x1CECD788,0xBE46C46C 
data4 0x7857D689,0xBE60B7EC,0xD14F1AD7,0xBE594D3D 
data4 0x4C9A8F60,0xBE4F9C30,0x02DFF9D2,0xBE521873 
data4 0x55E6D68F,0xBE5E4C88,0x667F3DC4,0xBE62140F 
data4 0x3BF88747,0xBE36961B,0xC96EC6AA,0x3E602861 
data4 0xD57FD718,0xBE3B5151,0xFC4A627B,0x3E561CD0 
data4 0xCA913FEA,0xBE3A5217,0x9A5D193A,0x3E40A3CC 
data4 0x10A9C312,0xBE5AB713,0xC5F57719,0x3E4FDADB 
data4 0xDBDF59D5,0x3E361428,0x61B4180D,0x3E5DB5DB 
data4 0x7408D856,0xBE42AD5F,0x31B2B707,0x3E2A3148
ASM_SIZE_DIRECTIVE(Constants_exp_64_W1)

.align 64
Constants_exp_64_W2:
ASM_TYPE_DIRECTIVE(Constants_exp_64_W2,@object)
data4 0x00000000,0x00000000,0x37A3D7A2,0xBE641F25 
data4 0xAD028C40,0xBE68DD57,0xF212B1B6,0xBE5C77D8 
data4 0x1BA5B070,0x3E57878F,0x2ECAE6FE,0xBE55A36A 
data4 0x569DFA3B,0xBE620608,0xA6D300A3,0xBE53B50E 
data4 0x223F8F2C,0x3E5B5EF2,0xD6DE0DF4,0xBE56A0D9 
data4 0xEAE28F51,0xBE64EEF3,0x367EA80B,0xBE5E5AE2 
data4 0x5FCBC02D,0x3E47CB1A,0x9BDAFEB7,0xBE656BA0 
data4 0x805AFEE7,0x3E6E70C6,0xA3415EBA,0xBE6E0509 
data4 0x49BFF529,0xBE56856B,0x00508651,0x3E66DD33 
data4 0xC114BC13,0x3E51165F,0xC453290F,0x3E53333D 
data4 0x05539FDA,0x3E6A072B,0x7C0A7696,0xBE47CD87 
data4 0xEB05C6D9,0xBE668BF4,0x6AE86C93,0xBE67C3E3 
data4 0xD0B3E84B,0xBE533904,0x556B53CE,0x3E63E8D9 
data4 0x63A98DC8,0x3E212C89,0x032A7A22,0xBE33138F 
data4 0xBC584008,0x3E530FA9,0xCCB93C97,0xBE6ADF82 
data4 0x8370EA39,0x3E5F9113,0xFB6A05D8,0x3E5443A4 
data4 0x181FEE7A,0x3E63DACD,0xF0F67DEC,0xBE62B29D 
data4 0x3DDE6307,0x3E65C483,0xD40A24C1,0x3E5BF030  
data4 0x14E437BE,0x3E658B8F,0xED98B6C7,0xBE631C29 
data4 0x04CF7C71,0x3E6335D2,0xE954A79D,0x3E529EED
data4 0xF64A2FB8,0x3E5D9257,0x854ED06C,0xBE6BED1B 
data4 0xD71405CB,0x3E5096F6,0xACB9FDF5,0xBE3D4893 
data4 0x01B68349,0xBDFEB158,0xC6A463B9,0x3E628D35 
data4 0xADE45917,0xBE559725,0x042FC476,0xBE68C29C 
data4 0x01E511FA,0xBE67593B,0x398801ED,0xBE4A4313 
data4 0xDA7C3300,0x3E699571,0x08062A9E,0x3E5349BE 
data4 0x755BB28E,0x3E5229C4,0x77A1F80D,0x3E67E426 
data4 0x6B69C352,0xBE52B33F,0x084DA57F,0xBE6B3550 
data4 0xD1D09A20,0xBE6DB03F,0x2161B2C1,0xBE60CBC4 
data4 0x78A2B771,0x3E56ED9C,0x9D0FA795,0xBE508E31 
data4 0xFD1A54E9,0xBE59482A,0xB07FD23E,0xBE2A17CE 
data4 0x17365712,0x3E68BF5C,0xB3785569,0x3E3956F9
ASM_SIZE_DIRECTIVE(Constants_exp_64_W2)

.align 64
Constants_log_80_P:
ASM_TYPE_DIRECTIVE(Constants_log_80_P,@object)
// 1/2, P_8, P_7, ..., P_1  
data4 0x00000000, 0x80000000, 0x00003FFE, 0x00000000
data4 0x3B1042BC, 0xCCCE8B88, 0x0000BFFB, 0x00000000
data4 0xCADC2149, 0xE38997B7, 0x00003FFB, 0x00000000
data4 0xB1ACB090, 0xFFFFFFFE, 0x0000BFFB, 0x00000000
data4 0x06481C81, 0x92492498, 0x00003FFC, 0x00000000
data4 0xAAAAB0EF, 0xAAAAAAAA, 0x0000BFFC, 0x00000000
data4 0xCCC91416, 0xCCCCCCCC, 0x00003FFC, 0x00000000
data4 0x00000000, 0x80000000, 0x0000BFFD, 0x00000000
data4 0xAAAAAAAB, 0xAAAAAAAA, 0x00003FFD
ASM_SIZE_DIRECTIVE(Constants_log_80_P)

.align 64
Constants_log_80_Q:
ASM_TYPE_DIRECTIVE(Constants_log_80_Q,@object)
// log2_hi, log2_lo, Q_6, Q_5, Q_4, Q_3, Q_2, Q_1 
data4 0x00000000,0xB1721800,0x00003FFE,0x00000000
data4 0x4361C4C6,0x82E30865,0x0000BFE2,0x00000000
data4 0xA51BE0AF,0x92492453,0x00003FFC,0x00000000
data4 0xA0CFD29F,0xAAAAAB73,0x0000BFFC,0x00000000
data4 0xCCCE3872,0xCCCCCCCC,0x00003FFC,0x00000000
data4 0xFFFFB4FB,0xFFFFFFFF,0x0000BFFC,0x00000000
data4 0xAAAAAAAB,0xAAAAAAAA,0x00003FFD,0x00000000
data4 0x00000000,0x80000000,0x0000BFFE,0x00000000
ASM_SIZE_DIRECTIVE(Constants_log_80_Q)

.align 64
Constants_log_80_Z_G_H_h1:
ASM_TYPE_DIRECTIVE(Constants_log_80_Z_G_H_h1,@object)
// Z1 - 16 bit fixed, G1 and H1 IEEE single, h1 IEEE double   
data4 0x00008000,0x3F800000,0x00000000,0x00000000
data4 0x00000000,0x00000000,0x00000000,0x00000000 
data4 0x00007879,0x3F70F0F0,0x3D785196,0x00000000
data4 0xEBA0E0D1,0x8B1D330B,0x00003FDA,0x00000000
data4 0x000071C8,0x3F638E38,0x3DF13843,0x00000000
data4 0x9EADD553,0xE2AF365E,0x00003FE2,0x00000000
data4 0x00006BCB,0x3F579430,0x3E2FF9A0,0x00000000
data4 0x752F34A2,0xF585FEC3,0x0000BFE3,0x00000000  
data4 0x00006667,0x3F4CCCC8,0x3E647FD6,0x00000000
data4 0x893B03F3,0xF3546435,0x00003FE2,0x00000000  
data4 0x00006187,0x3F430C30,0x3E8B3AE7,0x00000000 
data4 0x39CDD2AC,0xBABA62E0,0x00003FE4,0x00000000 
data4 0x00005D18,0x3F3A2E88,0x3EA30C68,0x00000000 
data4 0x457978A1,0x8718789F,0x00003FE2,0x00000000
data4 0x0000590C,0x3F321640,0x3EB9CEC8,0x00000000 
data4 0x3185E56A,0x9442DF96,0x0000BFE4,0x00000000 
data4 0x00005556,0x3F2AAAA8,0x3ECF9927,0x00000000 
data4 0x2BBE2CBD,0xCBF9A4BF,0x00003FE4,0x00000000 
data4 0x000051EC,0x3F23D708,0x3EE47FC5,0x00000000 
data4 0x852D5935,0xF3537535,0x00003FE3,0x00000000 
data4 0x00004EC5,0x3F1D89D8,0x3EF8947D,0x00000000 
data4 0x46CDF32F,0xA1F1E699,0x0000BFDF,0x00000000 
data4 0x00004BDB,0x3F17B420,0x3F05F3A1,0x00000000 
data4 0xD8484CE3,0x84A61856,0x00003FE4,0x00000000 
data4 0x00004925,0x3F124920,0x3F0F4303,0x00000000
data4 0xFF28821B,0xC7DD97E0,0x0000BFE2,0x00000000    
data4 0x0000469F,0x3F0D3DC8,0x3F183EBF,0x00000000 
data4 0xEF1FD32F,0xD3C4A887,0x00003FE3,0x00000000  
data4 0x00004445,0x3F088888,0x3F20EC80,0x00000000 
data4 0x464C76DA,0x84672BE6,0x00003FE5,0x00000000 
data4 0x00004211,0x3F042108,0x3F29516A,0x00000000
data4 0x18835FB9,0x9A43A511,0x0000BFE5,0x00000000 
ASM_SIZE_DIRECTIVE(Constants_log_80_Z_G_H_h1)

.align 64
Constants_log_80_Z_G_H_h2:
ASM_TYPE_DIRECTIVE(Constants_log_80_Z_G_H_h2,@object)
// Z2 - 16 bit fixed, G2 and H2 IEEE single, h2 IEEE double   
data4 0x00008000,0x3F800000,0x00000000,0x00000000 
data4 0x00000000,0x00000000,0x00000000,0x00000000 
data4 0x00007F81,0x3F7F00F8,0x3B7F875D,0x00000000 
data4 0x211398BF,0xAD08B116,0x00003FDB,0x00000000
data4 0x00007F02,0x3F7E03F8,0x3BFF015B,0x00000000 
data4 0xC376958E,0xB106790F,0x00003FDE,0x00000000 
data4 0x00007E85,0x3F7D08E0,0x3C3EE393,0x00000000 
data4 0x79A7679A,0xFD03F242,0x0000BFDA,0x00000000 
data4 0x00007E08,0x3F7C0FC0,0x3C7E0586,0x00000000 
data4 0x05E7AE08,0xF03F81C3,0x0000BFDF,0x00000000 
data4 0x00007D8D,0x3F7B1880,0x3C9E75D2,0x00000000 
data4 0x049EB22F,0xD1B87D3C,0x00003FDE,0x00000000
data4 0x00007D12,0x3F7A2328,0x3CBDC97A,0x00000000 
data4 0x3A9E81E0,0xFABC8B95,0x00003FDF,0x00000000 
data4 0x00007C98,0x3F792FB0,0x3CDCFE47,0x00000000
data4 0x7C4B5443,0xF5F3653F,0x00003FDF,0x00000000 
data4 0x00007C20,0x3F783E08,0x3CFC15D0,0x00000000 
data4 0xF65A1773,0xE78AB204,0x00003FE0,0x00000000  
data4 0x00007BA8,0x3F774E38,0x3D0D874D,0x00000000 
data4 0x7B8EF695,0xDB7CBFFF,0x0000BFE0,0x00000000 
data4 0x00007B31,0x3F766038,0x3D1CF49B,0x00000000 
data4 0xCF773FB3,0xC0241AEA,0x0000BFE0,0x00000000 
data4 0x00007ABB,0x3F757400,0x3D2C531D,0x00000000 
data4 0xC9539FDF,0xFC8F4D48,0x00003FE1,0x00000000  
data4 0x00007A45,0x3F748988,0x3D3BA322,0x00000000 
data4 0x954665C2,0x9CD035FB,0x0000BFE1,0x00000000  
data4 0x000079D1,0x3F73A0D0,0x3D4AE46F,0x00000000 
data4 0xDD367A30,0xEC9017C7,0x00003FE1,0x00000000 
data4 0x0000795D,0x3F72B9D0,0x3D5A1756,0x00000000 
data4 0xCB11189C,0xEE6625D3,0x0000BFE1,0x00000000 
data4 0x000078EB,0x3F71D488,0x3D693B9D,0x00000000 
data4 0xBE11C424,0xA49C8DB5,0x0000BFE0,0x00000000
ASM_SIZE_DIRECTIVE(Constants_log_80_Z_G_H_h2)
 
.align 64
Constants_log_80_h3_G_H:
ASM_TYPE_DIRECTIVE(Constants_log_80_h3_G_H,@object)
// h3 IEEE double extended, H3 and G3 IEEE single   
data4 0x112666B0,0xAAACAAB1,0x00003FD3,0x3F7FFC00 
data4 0x9B7FAD21,0x90051030,0x00003FD8,0x3F7FF400
data4 0xF4D783C4,0xA6B46F46,0x00003FDA,0x3F7FEC00 
data4 0x11C6DDCA,0xDA148D88,0x0000BFD8,0x3F7FE400 
data4 0xCA964D95,0xCE65C1D8,0x0000BFD8,0x3F7FDC00
data4 0x23412D13,0x883838EE,0x0000BFDB,0x3F7FD400 
data4 0x983ED687,0xB7E5CFA1,0x00003FDB,0x3F7FCC08 
data4 0xE3C3930B,0xDBE23B16,0x0000BFD9,0x3F7FC408 
data4 0x48AA4DFC,0x9B92F1FC,0x0000BFDC,0x3F7FBC10 
data4 0xCE9C8F7E,0x9A8CEB15,0x0000BFD9,0x3F7FB410 
data4 0x0DECE74A,0x8C220879,0x00003FDC,0x3F7FAC18 
data4 0x2F053150,0xB25CA912,0x0000BFDA,0x3F7FA420
data4 0xD9A5BE20,0xA5876555,0x00003FDB,0x3F7F9C20 
data4 0x2053F087,0xC919BB6E,0x00003FD9,0x3F7F9428 
data4 0x041E9A77,0xB70BDA79,0x00003FDC,0x3F7F8C30 
data4 0xEA1C9C30,0xF18A5C08,0x00003FDA,0x3F7F8438 
data4 0x796D89E5,0xA3790D84,0x0000BFDD,0x3F7F7C40 
data4 0xA2915A3A,0xE1852369,0x0000BFDD,0x3F7F7448 
data4 0xA39ED868,0xD803858F,0x00003FDC,0x3F7F6C50 
data4 0x9417EBB7,0xB2EEE356,0x0000BFDD,0x3F7F6458 
data4 0x9BB0D07F,0xED5C1F8A,0x0000BFDC,0x3F7F5C68 
data4 0xE87C740A,0xD6D201A0,0x0000BFDD,0x3F7F5470 
data4 0x1CA74025,0xE8DEBF5E,0x00003FDC,0x3F7F4C78 
data4 0x1F34A7EB,0x9A995A97,0x0000BFDC,0x3F7F4488
data4 0x359EED97,0x9CB0F742,0x0000BFDA,0x3F7F3C90 
data4 0xBBC6A1C8,0xD6F833C2,0x0000BFDD,0x3F7F34A0 
data4 0xE71090EC,0xE1F68F2A,0x00003FDC,0x3F7F2CA8 
data4 0xC160A74F,0xD1881CF1,0x0000BFDB,0x3F7F24B8 
data4 0xD78CB5A4,0x9AD05AE2,0x00003FD6,0x3F7F1CC8 
data4 0x9A77DC4B,0xE658CB8E,0x0000BFDD,0x3F7F14D8 
data4 0x6BD6D312,0xBA281296,0x00003FDC,0x3F7F0CE0 
data4 0xF95210D0,0xB478BBEB,0x0000BFDB,0x3F7F04F0 
data4 0x38800100,0x39400480,0x39A00640,0x39E00C41 // H's start here 
data4 0x3A100A21,0x3A300F22,0x3A4FF51C,0x3A6FFC1D 
data4 0x3A87F20B,0x3A97F68B,0x3AA7EB86,0x3AB7E101
data4 0x3AC7E701,0x3AD7DD7B,0x3AE7D474,0x3AF7CBED 
data4 0x3B03E1F3,0x3B0BDE2F,0x3B13DAAA,0x3B1BD766 
data4 0x3B23CC5C,0x3B2BC997,0x3B33C711,0x3B3BBCC6 
data4 0x3B43BAC0,0x3B4BB0F4,0x3B53AF6D,0x3B5BA620 
data4 0x3B639D12,0x3B6B9444,0x3B7393BC,0x3B7B8B6D 
ASM_SIZE_DIRECTIVE(Constants_log_80_h3_G_H)

.align 64
Constant_half:
ASM_TYPE_DIRECTIVE(Constant_half,@object)
data4 0x00000000,0x80000000,0x00003FFE
ASM_SIZE_DIRECTIVE(Constant_half)

GR_Expo_Range       = r32
GR_Flag             = r33
GR_Table_Ptr        = r34

GR_Table_Ptr1       = r35
GR_BIAS             = r35

GR_Index1           = r36
GR_sign_mask        = r36

GR_Index2           = r37
GR_Expo_X           = r37

GR_signif_Z         = r38
GR_M                = r38

GR_X_0              = r39
GR_Mask             = r39

GR_X_1              = r40
GR_W1_ptr           = r40

GR_W2_ptr           = r41
GR_X_2              = r41

GR_Z_1              = r42
GR_M2               = r42

GR_M1               = r43
GR_Z_2              = r43

GR_N                = r44
GR_k                = r44

GR_Big_Pos_Exp      = r45


GR_BIAS_p_k         = r47
GR_BIASed_exp_y     = r47

GR_Big_Neg_Exp      = r48
GR_Index3           = r48
GR_temp             = r48

GR_vsm_expo         = r49
GR_y_sign           = r49

GR_T1_ptr           = r50
GR_T2_ptr           = r51
GR_N_fix            = r52
GR_exp_y            = r53
GR_signif_y         = r54
GR_exp_and_sign_y   = r55
GR_low_order_bit    = r56
GR_get_exp_mask     = r57
GR_exponent_zero    = r58

// ** Registers for unwind support

GR_SAVE_PFS         = r59
GR_SAVE_B0          = r60
GR_SAVE_GP          = r61
GR_Parameter_X      = r62 
GR_Parameter_Y      = r63 
GR_Parameter_RESULT = r64 
GR_Parameter_TAG    = r65 

FR_X      = f8
FR_Y      = f9
FR_RESULT = f99

// **

FR_Input_X          = f8
FR_Output           = f8
FR_Input_Y          = f9

FR_Neg              = f10
FR_P_hi             = f10
FR_X                = f10

FR_Half             = f11
FR_h_3              = f11
FR_poly_hi          = f11

FR_Sgn              = f12

FR_Neg_X            = f13
FR_half_W           = f13

FR_X_cor            = f14
FR_P_lo             = f14

FR_W                = f15

FR_X_lo             = f32

FR_S                = f33
FR_W3               = f33

FR_Y_hi             = f34
FR_logx_hi          = f34

FR_Z                = f35
FR_logx_lo          = f35
FR_GS_hi            = f35
FR_Y_lo             = f35

FR_r_cor            = f36
FR_Scale            = f36

FR_G_1              = f37
FR_G                = f37
FR_Wsq              = f37
FR_L_Inv            = f37
FR_temp             = f37

FR_H_1              = f38
FR_H                = f38
FR_W4               = f38
FR_float_N          = f38

FR_h                = f39
FR_h_1              = f39
FR_N                = f39
FR_P_7              = f39

FR_G_2              = f40
FR_P_8              = f40
FR_L_hi             = f40

FR_H_2              = f41
FR_L_lo             = f41
FR_A_1              = f41

FR_h_2              = f42
FR_P_6              = f42

FR_abs_W            = f43
FR_W1               = f43

FR_G_3              = f44
FR_P_8              = f44
FR_T1               = f44

FR_log2_hi          = f45
FR_W2               = f45

FR_GS_lo            = f46
FR_T2               = f46

FR_W_1_p1           = f47
FR_H_3              = f47

FR_float_N          = f48

FR_P_4              = f49
FR_A_2              = f49

FR_Q_4              = f50
FR_r4               = f50

FR_Q_3              = f51
FR_A_3              = f51

FR_Q_2              = f52
FR_P_2              = f52

FR_Q_1              = f53
FR_P_1              = f53
FR_T                = f53

FR_Wp1              = f54
FR_Q_5              = f54
FR_P_3              = f54

FR_Q_6              = f55

FR_log2_lo          = f56
FR_Two              = f56

FR_Big              = f57

FR_neg_2_mK         = f58
FR_NBig             = f58

FR_r                = f59

FR_poly_lo          = f60

FR_poly             = f61

FR_P_5              = f62

FR_rsq              = f63

FR_Result           = f99
FR_Result_small     = f100
FR_Result_big       = f101

.section .text
.proc powl#
.global powl#
.align 64 

powl: 
{ .mfi
alloc  GR_Expo_Range = ar.pfs,0,30,4,0
(p0)   fclass.m.unc p7, p13 =  FR_Input_Y, 0x1E7 
nop.i 0
}
{ .mfi
(p0)   getf.exp GR_exp_and_sign_y = FR_Input_Y 
//
//     Save State
//
(p0)   fclass.m.unc p6, p12 =  FR_Input_X, 0x1E7 
nop.i 0
};;
{ .mfi
(p0)   getf.sig GR_signif_y = FR_Input_Y 
(p0)   fcmp.eq.unc.s1 p12, p13 =  FR_Input_X, f1 
nop.i 0
}
{ .mfi
	nop.m 999
//
//     Check for y = 1 
//     Identify EM unsupporteds.
//     Load FR_half = .5
//
(p0)   fadd.s1 FR_Two = f1, f1 
//
//     Load 1/2 in GP register
//
nop.i 0 
}
;;

{ .mmi
      nop.m 999
(p0)  addl           GR_Table_Ptr   = @ltoff(Constant_half#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Ptr = [GR_Table_Ptr]
      nop.m 999
      nop.i 999
}
;;

{ .mlx
(p0)   ldfe FR_Half =[GR_Table_Ptr],0
(p0)   movl GR_get_exp_mask = 0x1FFFF ;; 
}

{ .mfi
	nop.m 999
(p0)   fclass.nm.unc p9, p15 =  FR_Input_Y, 0x1FF 
//
//     Create FR_Two = 2
//     Get exp and significand of Y
//     Crate Masks
//     sgn = 1
//
(p0)   and GR_exp_y = GR_get_exp_mask,GR_exp_and_sign_y
}
{ .mlx
	nop.m 999
(p0)   movl GR_exponent_zero = 0xFFFF ;; 
}
{ .mfi
	nop.m 999
(p0)   mov FR_Sgn = f1 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fcmp.eq.unc.s1 p10, p11 =  FR_Input_Y, f1 
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//
//     Identify NatVals, NaNs, Infs, and Zeros.
//     Load Half
//
(p0)   fclass.nm.unc p8, p14 =  FR_Input_X, 0x1FF 
//      
//     Remove sign bit from exponent of y.
//     Check for x = 1 
//
(p6)   br.cond.spnt L(POWL_64_SPECIAL) ;; 
}
{ .mib
	nop.m 999
	nop.i 999
(p7)   br.cond.spnt L(POWL_64_SPECIAL) ;; 
}
{ .mib
	nop.m 999
	nop.i 999
(p8)   br.cond.spnt L(POWL_64_UNSUPPORT) ;; 
}
{ .mib
	nop.m 999
	nop.i 999
(p9)   br.cond.spnt L(POWL_64_UNSUPPORT) ;; 
}
{ .mfi
(p0)   cmp.lt.unc  p9, p0 = GR_exp_y,GR_exponent_zero 
(p0)   fcmp.lt.unc.s1 p6, p13  =  FR_Input_X, f0 
//
//     Branch on Infs, Nans, Zeros, and Natvals
//     Check to see that exponent < 0
//
(p0)   sub GR_exp_y = GR_exp_y,GR_exponent_zero
}
//     x not zero, is y ==2? 
{ .mfi
	nop.m 999
(p11)  fcmp.eq.unc.s1 p7, p14 =  FR_Input_Y, FR_Two 
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p9)   fcmp.lt.unc.s1 p9, p0   =  FR_Input_X, f0 
(p7)   br.cond.spnt L(POWL_64_SQUARE) ;;   // Branch if x not zero and y=2
}
{ .mfi
	nop.m 999
(p6)   fmerge.ns FR_Neg_X = FR_Input_X, FR_Input_X 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p10)  fmpy.s0 FR_Result = FR_Input_X, f1 
//
//     For y = 1, compute result = x 
//     For x = 1, compute 1 
//     When Y is one return X and possible raise 
//     denormal operand exception.
//     Remove exponent BIAS
//
(p6)   shl GR_exp_and_sign_y=  GR_signif_y,GR_exp_y ;; 
}
{ .mfi
(p9)   or  GR_exp_and_sign_y = 0xF,GR_signif_y 
(p12)  fma.s0 FR_Result = FR_Input_Y, f0, f1 
	nop.i 999 ;;
}
{ .mii
	nop.m 999
(p6)   extr.u GR_exp_y = GR_exp_and_sign_y,63,1 ;; 
(p6)   cmp.ne.unc  p9, p0 =  GR_exp_y, r0 
}
{ .mii
	nop.m 999
//
//     Both predicates can be set. 
//     Don't consider  y's < 1.
//
(p6)   shl GR_signif_y=  GR_exp_and_sign_y,1 ;; 
//
//     Is shift off integer part of y.
//     Get y's  even or odd bit.
//
(p6)   cmp.ne.unc  p8, p0  =  GR_signif_y, r0 
}
{ .mib
	nop.m 999
	nop.i 999
//
//     Is the fractional part of the y = 0?
//     Is the integer even or odd. 
//
(p10)  br.cond.spnt L(POWL_64_RETURN) ;; 
}
{ .mib
	nop.m 999
	nop.i 999
(p12)  br.cond.spnt L(POWL_64_RETURN) ;; 
}
{ .mib
	nop.m 999
	nop.i 999
(p8)   br.cond.spnt L(POWL_64_XNEG) ;;
}
{ .mfi
	nop.m 999
(p9)   fmerge.ns FR_Sgn = FR_Sgn, FR_Sgn 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fcmp.eq.unc.s0 p11, p0 =  FR_Input_Y, FR_Half 
	nop.i 999 ;;
}
//
//     Raise possible denormal operand exception for both
//     X and Y.
//
{ .mfb
	nop.m 999
//
//     Branch for (x < 0) and Y not an integer.
//
(p0)   fcmp.eq.unc.s0 p12, p0 =  FR_Input_X, f1 
//
//     For x < 0 and y integer, make x positive 
//     For x < 0 and y odd integer,, set sign = -1.
//
(p11)  br.cond.spnt L(POWL_64_SQRT) ;; 
}
{ .mmf
(p0)   cmp.eq.unc  p15, p14 =  r0, r0 
	nop.m 999
(p13)  fnorm.s1 FR_Z = FR_Input_X ;; 
}
{ .mfi
	nop.m 999
(p6)   fnorm.s1 FR_Z = FR_Neg_X 
	nop.i 999
}
;;

//
//     Branch to embedded sqrt(x)
//
//
//     Computes ln( x ) to extra precision 
//     Input  FR 1: FR_X            
//     Output FR 2: FR_Y_hi  
//     Output FR 3: FR_Y_lo  
//     Output PR 1: PR_Safe  
//

{ .mmi
      nop.m 999
(p0)  addl           GR_Table_Ptr   = @ltoff(Constants_log_80_Z_G_H_h1#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Ptr = [GR_Table_Ptr]
      nop.m 999
      nop.i 999
}
;;


{ .mlx
	nop.m 999
(p0)   movl GR_BIAS = 0x000000000000FFFF ;; 
}
{ .mfi
	nop.m 999
(p0)   fsub.s1 FR_W = FR_Z, f1 
	nop.i 999 ;;
}
//     
//     Z = Norm(X) - both + and - case 
//     Set Safe = True
//     
{ .mmb
(p0)   getf.sig GR_signif_Z = FR_Z 
(p0)   getf.exp GR_N =  FR_Z 
	nop.b 999 ;;
}
{ .mii
	nop.m 999
//     
//     Get significand of Z 
//     W = Z - 1
//     
(p0)   extr.u GR_Index1 = GR_signif_Z, 59, 4 ;;  
//     
//     Index1 = High order 4 bits of Z
//     X_0  = High order 15 bit of Z 
//
(p0)   shl GR_Index1 = GR_Index1,5 ;; 
}
{ .mfi
	nop.m 999
//     
//     Add offset to Index1 ptr.
//     
(p0)   fabs FR_abs_W =  FR_W 
//     
//     BIAS = 0x000...FFFF
//     Adjust Index1 ptr ( x 32) .
//     
(p0)   add GR_Index1 = GR_Index1,GR_Table_Ptr  
}
{ .mmi
	nop.m 999 ;;
(p0)   ld2 GR_Z_1 =[GR_Index1],4
(p0)   extr.u GR_X_0 = GR_signif_Z, 49, 15  
}
;;

{ .mmi
      nop.m 999
(p0)  addl           GR_Table_Ptr   = @ltoff(Constants_log_80_Z_G_H_h2#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Ptr = [GR_Table_Ptr]
      nop.m 999
      nop.i 999
}
;;


{ .mmi
(p0)   ldfs  FR_G_1 = [GR_Index1],4 ;; 
(p0)   ldfs  FR_H_1 = [GR_Index1],8 
	nop.i 999 ;;
}
//
//     Adjust Index2 (x 32). 
//
{ .mfi
(p0)   ldfe FR_h_1 = [GR_Index1],0 
	nop.f 999
(p0)   pmpyshr2.u GR_X_1 = GR_X_0,GR_Z_1,15 ;; 
}
{ .mmi
	nop.m 999 ;;
//
//     load Z_1 from Index1
//     abs_W = |W|
//     Point to Table2 
//
(p0)   getf.exp GR_M = FR_abs_W 
//
//     M = M - BIAS 
//     Load G_1 
//     N = exponent of Z 
//
       nop.i 999;;
}
{ .mmi
       nop.m 999
       nop.m 999
       nop.i 999;;
}
{ .mmi
       nop.m 999
       nop.m 999
       nop.i 999;;
}
{ .mmi
        nop.m 999
        nop.m 999
(p0)   extr.u GR_Index2 = GR_X_1, 6, 4 ;;  
}
{ .mii
	nop.m 999
//     
//     Extract Index2 
//     Load H_1
//     Is -8 > M ?
//
(p0)   shl GR_Index2=GR_Index2,5 ;; 
(p0)   add GR_Index2 = GR_Index2, GR_Table_Ptr  
}
//
//     M = exponent of abs_W
//     X_1 = X_0 * Z_1 
//     
{ .mii
(p0)   sub GR_M = GR_M, GR_BIAS  
	nop.i 999 ;;
(p0)   cmp.gt.unc  p7, p14 =  -8, GR_M 
}
{ .mib
	nop.m 999
	nop.i 999
(p7)   br.cond.spnt L(LOGL80_NEAR) ;; 
}
//
//     Load h_1
//     Possible branch out.  
//     Add offset of table to Index2 
//
{ .mfi
(p0)   ld2 GR_Z_2 =[GR_Index2],4
(p0)   fmerge.se FR_S =  f1,FR_Z
(p0)   sub GR_N = GR_N, GR_BIAS  
}
;;

{ .mmi
      nop.m 999
(p0)  addl           GR_Table_Ptr   = @ltoff(Constants_log_80_h3_G_H#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Ptr = [GR_Table_Ptr]
      nop.m 999
      nop.i 999
}
;;

//     
//     load Z_2 
//     N - BIAS 
//     Point to Table 3.
//     S = merging of Z and 1.0
//     
{ .mmi
(p0)   ldfs  FR_G_2 = [GR_Index2],4 
(p0)   setf.sig FR_float_N = GR_N 
(p0)   add GR_Table_Ptr1 = 0x200,GR_Table_Ptr ;;  
}
//
//     load G_2 
//     X_2 = X_1 * Z_2 
//     Add offset to Table 2 ptr.
//     float_N = significand of N
//
{ .mmi
(p0)   ldfs  FR_H_2 = [GR_Index2],8 ;; 
//
//     load H_2 
//     G = G * G_2
//
(p0)   ldfe FR_h_2 = [GR_Index2],0 
(p0)   pmpyshr2.u GR_X_2 = GR_X_1,GR_Z_2,15 ;; 
}
{ .mmi
       nop.m 999
       nop.m 999
       nop.i 999;;
}
{ .mmi
       nop.m 999
       nop.m 999
       nop.i 999;;
}
{ .mmi
        nop.m 999
        nop.m 999
        nop.i 999;;
}
{ .mii
	nop.m 999
	nop.i 999 ;;
(p0)   extr.u GR_Index3 = GR_X_2, 1, 5 ;;  
}
{ .mfi
(p0)   shladd GR_Table_Ptr1 = GR_Index3,2,GR_Table_Ptr1 
	nop.f 999
//
//     h = h_1 + h_2  
//     Adjust Index3 
//
(p0)   shladd GR_Index3 = GR_Index3,4,GR_Table_Ptr ;; 
}
{ .mmb
	nop.m 999
(p0)   ldfe FR_h_3 = [GR_Index3],12 
	nop.b 999 ;;
}
{ .mmf
(p0)   ldfs FR_H_3 = [GR_Table_Ptr1],0 
//
//     float_N = Make N a fp number
//     Load h_3
//     Get pointer to Q table.     
//
(p0)   ldfs  FR_G_3 = [GR_Index3],0 
(p0)   fmpy.s1 FR_G = FR_G_1, FR_G_2 
}
;;

{ .mmi
      nop.m 999
(p0)  addl           GR_Table_Ptr   = @ltoff(Constants_log_80_Q#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Ptr = [GR_Table_Ptr]
      nop.m 999
      nop.i 999
}
;;



{ .mfi
(p0)   ldfe FR_log2_hi = [GR_Table_Ptr],16
(p0)   fadd.s1 FR_H = FR_H_1, FR_H_2 
	nop.i 999 ;;
}
{ .mmf
	nop.m 999
//
//     G = G_1 * G_2 * G_3 
//
(p0)   ldfe FR_log2_lo = [GR_Table_Ptr],16 
//
//     load h_2 
//     H = H_1 + H_2 
//     Get Index3
//
(p0)   fadd.s1 FR_h = FR_h_1, FR_h_2 ;; 
}
//
//     Load log2_lo part
//     r = G*S -1
//
{ .mfi
(p0)   ldfe FR_Q_6 = [GR_Table_Ptr],16 
//
//     Load H_3
//
(p0)   fcvt.xf FR_float_N = FR_float_N 
	nop.i 999 ;;
}
//
//     Load Q_6
//
{ .mmi
(p0)   ldfe FR_Q_5 = [GR_Table_Ptr],16 ;; 
(p0)   ldfe FR_Q_4 = [GR_Table_Ptr],16 
	nop.i 999 ;;
}
{ .mmi
(p0)   ldfe FR_Q_3 = [GR_Table_Ptr],16 ;; 
(p0)   ldfe FR_Q_2 = [GR_Table_Ptr],16 
	nop.i 999 ;;
}
{ .mmf
	nop.m 999
//
//     poly_lo = Q_5 + r * Q_6
//     Load Q_2
//     rsq = r * r 
//
(p0)   ldfe FR_Q_1 = [GR_Table_Ptr],16 
//
//     h = h_1 + h_2 + h_3   
//     H = H_1 + H_2 + H_3 
//     Load G_3.
//     Begin Loading Q's -  load log2_hi part
//
(p0)   fmpy.s1 FR_G = FR_G, FR_G_3 
}
{ .mfi
	nop.m 999
(p0)   fadd.s1 FR_H = FR_H, FR_H_3 
	nop.i 999 
}
;;

//
//     Y_lo = poly + Y_lo 
//

{ .mmi
      nop.m 999
(p0)  addl           GR_Table_Ptr   = @ltoff(Constants_exp_64_Arg#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Ptr = [GR_Table_Ptr]
      nop.m 999
      nop.i 999
}
;;


{ .mfi
	nop.m 999
(p0)   fadd.s1 FR_h = FR_h, FR_h_3 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Load Q_5
//
(p0)   fmpy.s1 FR_GS_hi = FR_G, FR_S 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fms.s1 FR_r = FR_G, FR_S, f1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fma.s1 FR_poly_lo = FR_r, FR_Q_6, FR_Q_5 
	nop.i 999
}
{ .mfi
	nop.m 999
//
//     GS_hi = G*S
//     Load Q_4
//
(p0)   fsub.s1 FR_r_cor = FR_GS_hi, f1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fms.s1 FR_GS_lo = FR_G, FR_S, FR_GS_hi 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fma.s1 FR_poly = FR_r, FR_Q_2, FR_Q_1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Load Q_3
//     r_cor = GS_hi -1
//     GS_lo  = G*S - GS_hi
//
(p0)   fmpy.s1 FR_rsq = FR_r, FR_r 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fma.s1 FR_G = FR_float_N, FR_log2_hi, FR_H 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     poly = poly_hi + rsq * poly_lo 
//     Tbl = float_N*log2_hi + H
//
(p0)   fma.s1 FR_Y_lo = FR_float_N, FR_log2_lo, FR_h 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     r_cor = r_cor - r
//     poly_hi = r * Q_2 + Q_1
//
(p0)   fma.s1 FR_poly_lo = FR_r, FR_poly_lo, FR_Q_4 
	nop.i 999
}
{ .mfi
	nop.m 999
//
//     Load Q_1
//
(p0)   fsub.s1 FR_r_cor = FR_r_cor, FR_r 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
// 
//     Y_lo = float_N*log2_lo + h
// 
(p0)   fadd.s1 FR_Y_hi = FR_G, FR_r 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     poly_lo = Q_4 + r * poly_lo;;
//     r_cor = r_cor + GS_lo;;
//
(p0)   fma.s1 FR_poly_lo = FR_r, FR_poly_lo, FR_Q_3 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fadd.s1 FR_r_cor = FR_r_cor, FR_GS_lo 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fadd.s1 FR_r_cor = FR_r_cor, FR_Y_lo 
	nop.i 999
}
{ .mfi
	nop.m 999
//
//     poly_lo = Q_3 + r * poly_lo;;
//
(p0)   fma.s1 FR_poly = FR_rsq, FR_poly_lo, FR_poly 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fsub.s1 FR_Y_lo = FR_G, FR_Y_hi 
	nop.i 999
}
{ .mmi
(p0)   ldfe FR_L_Inv = [GR_Table_Ptr],16 ;; 
(p0)   ldfe FR_L_hi = [GR_Table_Ptr],16 
	nop.i 999 ;;
}
{ .mfi
(p0)   ldfe FR_L_lo = [GR_Table_Ptr],16 
	nop.f 999
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Y_hi = Tbl + r 
//     r_cor = r_cor + Y_lo 
//
(p0)   fma.s1 FR_poly = FR_rsq, FR_poly, FR_r_cor 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//     Y_lo = Tbl - Y_hi 
//     poly = rsq * poly + r_cor
//
(p0)   fadd.s1 FR_Y_lo = FR_Y_lo, FR_r 
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//
//     Y_lo =  Y_lo + r  
//
(p0)   fadd.s1 FR_Y_lo = FR_Y_lo, FR_poly 
//
//     Load L_Inv
//     Load L_hi
//     Load L_lo
//     all long before they are needed.
//     They are used in LOGL_RETURN PATH
//
br.cond.sptk L(LOGL_RETURN) ;; 
}
L(LOGL80_NEAR): 
//
//     Branch LOGL80_NEAR
//

{ .mmi
      nop.m 999
(p0)  addl           GR_Table_Ptr   = @ltoff(Constants_log_80_P#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Ptr = [GR_Table_Ptr]
      nop.m 999
      nop.i 999
}
;;

{ .mfi
	nop.m 999
(p0)   fmpy.s1 FR_Wsq = FR_W, FR_W 
(p0)   add GR_Table_Ptr1 = 0x50,GR_Table_Ptr  
}
//
//     Adjust ptr to 1/2 
//     Adjust Ptr1 to P_4
//
{ .mmi
(p0)   ldfe FR_Half = [GR_Table_Ptr],16 ;; 
(p0)   ldfe FR_P_4 = [GR_Table_Ptr1],16 
	nop.i 999
}
//
//     Load 1/2 
//
{ .mmi
(p0)   ldfe FR_P_8 = [GR_Table_Ptr],16 ;; 
(p0)   ldfe FR_P_3 = [GR_Table_Ptr1],16 
	nop.i 999
}
{ .mmi
(p0)   ldfe FR_P_7 = [GR_Table_Ptr],16 ;; 
(p0)   ldfe FR_P_2 = [GR_Table_Ptr1],16 
	nop.i 999
}
//
//     Load P_7
//     half_W = .5 * W
//     Load P_3
//
{ .mmi
(p0)   ldfe FR_P_6 = [GR_Table_Ptr],16 ;; 
(p0)   ldfe FR_P_1 = [GR_Table_Ptr1],16 
	nop.i 999 ;;
}
//
//     Load P_6
//     Wsq = w * w
//     poly = w*P_4 + P_3 
//     Load P_2
//
{ .mfi
(p0)   ldfe FR_P_5 = [GR_Table_Ptr],16 
//
//     Load P_5
//     poly_lo =  w * P_8 + P_7 
//     Y_hi = w - (1/2)w*w
//     Load P_1
//
(p0)   fmpy.s1 FR_W4 = FR_Wsq, FR_Wsq 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fmpy.s1 FR_W3 = FR_Wsq, FR_W 
	nop.i 999 
}
;;

//
//     Y_lo = W3 * poly + Y_lo
//

{ .mmi
      nop.m 999
(p0)  addl           GR_Table_Ptr   = @ltoff(Constants_exp_64_Arg#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Ptr = [GR_Table_Ptr]
      nop.m 999
      nop.i 999
}
;;


{ .mmi
(p0)   ldfe FR_L_Inv = [GR_Table_Ptr],16 ;; 
(p0)   ldfe FR_L_hi = [GR_Table_Ptr],16 
	nop.i 999 ;;
}
{ .mfi
(p0)   ldfe FR_L_lo = [GR_Table_Ptr],16 
//
//     Load P_8
//     Load P_4
//
(p0)   fmpy.s1 FR_half_W = FR_Half, FR_W 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fma.s1 FR_poly_lo = FR_W, FR_P_8,FR_P_7 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fma.s1 FR_poly = FR_W, FR_P_4, FR_P_3 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fnma.s1 FR_Y_hi = FR_W, FR_half_W, FR_W 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     W4 = Wsq * Wsq
//     poly = w *poly + P_2
//
(p0)   fma.s1 FR_poly_lo = FR_W, FR_poly_lo, FR_P_6 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fma.s1 FR_poly = FR_W, FR_poly, FR_P_2 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fsub.s1 FR_Y_lo = FR_W, FR_Y_hi 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     poly = w * poly + P_1
//     w3 = wsq * w
//
(p0)   fma.s1 FR_poly_lo = FR_W, FR_poly_lo, FR_P_5 
	nop.i 999
}
{ .mfi
	nop.m 999
//
//     poly_lo = w * poly_lo + P_6
//     Y_lo = W - Y_hi
//
(p0)   fma.s1 FR_poly = FR_W, FR_poly, FR_P_1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fnma.s1 FR_Y_lo = FR_W, FR_half_W, FR_Y_lo 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     poly_lo = w * poly_lo + 
//     Y_lo = Y_lo - w * (1/2)w
//
(p0)   fma.s1 FR_poly = FR_poly_lo, FR_W4, FR_poly 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Y_lo = (W-Y_hi) - w * (1/2)w
//     poly =  W4* poly_lo + poly 
//
(p0)   fma.s1 FR_Y_lo = FR_poly, FR_W3, FR_Y_lo 
	nop.i 999 ;;
}
L(LOGL_RETURN): 
{ .mfi
(p0)   add GR_Expo_Range = 0x2,r0  
//
//     Load L_Inv
//     Load L_hi
//     Load L_lo
//     all long before they are needed.
//
//
//     kernel_log_80 computed ln(X)
//     and return logX_hi and logX_lo as results.
//     PR_pow_Safe set as well. 
//
(p0)   fmpy.s1 FR_X_lo = FR_Input_Y, FR_logx_lo 
//
//     Compute Y * (logX_hi + logX_lo) 
//     P_hi -> X 
//     P_lo -> X_cor 
//     (Manipulate names so that inputs are in
//     the place kernel_exp expects them)
//     Set GR_Flag to 2 
//     Set GR_Expo_Range to Double
// 
//     This function computes exp( x  + x_cor) 
//     Input  FR 1: FR_X            
//     Input  FR 2: FR_X_cor  
//     Input  GR 1: GR_Flag  
//     Input  GR 2: GR_Expo_Range  
//     Output FR 3: FR_Y_hi  
//     Output FR 4: FR_Y_lo  
//     Output FR 5: FR_Scale  
//     Output PR 1: PR_Safe  
// 
(p0)   cmp.eq.unc  p15, p0 =  r0, r0 
}
;;

{ .mmi
(p0)  addl           GR_W1_ptr   = @ltoff(Constants_exp_64_W1#), gp
(p0)  addl           GR_W2_ptr   = @ltoff(Constants_exp_64_W2#), gp
(p0)  add GR_Flag = 0x2,r0  
}
;;

{ .mmi
      ld8 GR_W1_ptr = [GR_W1_ptr]
      ld8 GR_W2_ptr = [GR_W2_ptr]
(p0)   cmp.ne.unc  p7, p0 =  0x1, GR_Flag 
}
;;

{ .mlx
	nop.m 999
(p0)   movl GR_Mask = 0x1FFFF ;; 
}


{ .mlx
	nop.m 999
(p0)   movl GR_BIAS = 0x0FFFF ;; 
}
{ .mfi
	nop.m 999
//
//     X_lo =  Y * logX_lo
//
(p0)   fma.s1 FR_P_hi = FR_Input_Y, FR_logx_hi,FR_X_lo 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Set Safe=True 
//     Flag is always 2 for this routine
//
(p0)   fmpy.s1 FR_float_N = FR_X, FR_L_Inv 
	nop.i 999
}
{ .mfi
	nop.m 999
//
//     X_hi  = Y * logX_hi + X_lo
//     Set GR_Flag = 2 for exp(x + xcor)
//
(p0)   fms.s1 FR_P_lo= FR_Input_Y, FR_logx_hi, FR_P_hi 
	nop.i 999 ;;
}
{ .mmi
	nop.m 999 ;;
(p0)   getf.exp GR_Expo_X = FR_X 
	nop.i 999 ;;
}
{ .mfi
(p0)   and GR_Expo_X = GR_Expo_X, GR_Mask  
//
//     Calculate unBIASed exponent of X
//     Point to Table of W1s
//     Point to Table of W2s
//
(p0)   fcvt.fx.s1 FR_N = FR_float_N 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fadd.s1 FR_P_lo = FR_P_lo, FR_X_lo 
//
//     Float_N = X * L_Inv
//     Create exponent BIAS
//     Get BIASed exponent of X
//
(p0)   sub GR_Expo_X = GR_Expo_X, GR_BIAS ;;  
}
{ .mib
(p0)   cmp.gt.unc  p9, p0  =  -6, GR_Expo_X 
	nop.i 999
//
//     N = fcvt.fx(float_N)
//     If -6 > Expo_X, set P9
//
(p9)   br.cond.spnt L(EXPL_SMALL) 
}
;;

//
//     If expo_X < -6 goto exp_small
//
{ .mmi
      nop.m 999
(p0)  addl           GR_T1_ptr   = @ltoff(Constants_exp_64_T1#), gp
(p0)  cmp.lt.unc  p10, p0 =  14, GR_Expo_X 
}
;;

{ .mmi
      ld8 GR_T1_ptr = [GR_T1_ptr]
      nop.m 999
      nop.i 999
}
;;

{ .mib
	nop.m 999
	nop.i 999
//
//     If 14 < Expo_X, set P10
//     Create pointer to T1 table
//  
(p10)  br.cond.spnt L(EXPL_HUGE) ;;
}


{ .mmi
(p0)  addl           GR_Table_Ptr   = @ltoff(Constants_exp_64_Exponents#), gp
(p0)  addl           GR_T2_ptr   = @ltoff(Constants_exp_64_T2#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Ptr = [GR_Table_Ptr]
      ld8 GR_T2_ptr = [GR_T2_ptr]
      nop.i 999
}
;;


{ .mmi
(p0)   shladd GR_Table_Ptr = GR_Expo_Range,4,GR_Table_Ptr ;;  
//
//     Adjust T1_ptr by x 4 for single-precision values
//     Adjust T2_ptr by x 4 for single-precision values
//
(p0)   ld8 GR_Big_Pos_Exp = [GR_Table_Ptr],8
	nop.i 999 ;;
}
//
//     Load double W1
//     Load +max exponent
//
{ .mfi
(p0)   ld8 GR_Big_Neg_Exp = [GR_Table_Ptr],0
//
//     If 14 < Expo_X, goto exp_huge
//
(p0)   fcvt.xf FR_float_N = FR_N 
	nop.i 999 
}
;;

//
//     Load double W2
//     Load -max exponent
//     Load ptr to A's
//

{ .mmi
(p0)  getf.sig GR_N_fix = FR_N 
(p0)  addl           GR_Table_Ptr   = @ltoff(Constants_exp_64_A#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Ptr = [GR_Table_Ptr]
      nop.m 999
      nop.i 999
}
;;

//
//     Load single T1
//     Load single T2
//     W_1_p1 = W_1 + 1
//
{ .mmi
(p0)   ldfe FR_A_3 = [GR_Table_Ptr],16 ;; 
//
//     Load A_3
//     if k > big_pos_exp, set p14 and Safe=False
//
(p0)   ldfe FR_A_2 = [GR_Table_Ptr],16 
(p0)   extr.u GR_M1 = GR_N_fix, 6, 6  
}
{ .mmi
	nop.m 999 ;;
(p0)   shladd GR_W1_ptr = GR_M1,3,GR_W1_ptr  
//
//     float_N = fcvt.xf(N) 
//     N_fix = significand of N
//     Create pointer to T2 table
//
(p0)   extr.u GR_M2 = GR_N_fix, 0, 6  
}
//
//     r = r + X_cor
//     Adjust W1_ptr by x 8 for double-precision values
//     Adjust W2_ptr by x 8 for double-precision values
//     Adjust Table_ptr by Expo_Rangex16 
//
{ .mmi
(p0)   shladd GR_T1_ptr = GR_M1,2,GR_T1_ptr ;;  
(p0)   ldfd  FR_W1 = [GR_W1_ptr],0 
(p0)   shladd GR_W2_ptr = GR_M2,3,GR_W2_ptr  
}
//
//     Load ptr to A's
//
{ .mfi
(p0)   ldfs  FR_T1 = [GR_T1_ptr],0 
(p0)   fnma.s1 FR_r = FR_L_hi, FR_float_N, FR_X 
(p0)   shladd GR_T2_ptr = GR_M2,2,GR_T2_ptr ;; 
}
{ .mmi
(p0)   ldfd  FR_W2 = [GR_W2_ptr],0 
(p0)   ldfs  FR_T2 = [GR_T2_ptr],0 
//
//     r = x - L_hi * float_N
//     M2 = extr.u(N_fix,0,6)
//     M1 = extr.u(N_fix,6,6)
//
(p0)   extr GR_k = GR_N_fix, 12, 52 ;;  
}
//
//     Load A_1
//     poly = A_3 * r + A_2
//     rsq = r*r
//
{ .mii
(p0)   add GR_BIAS_p_k = GR_BIAS, GR_k  
(p0)   cmp.gt.unc  p14,p15 = GR_k,GR_Big_Pos_Exp ;; 
(p15)  cmp.lt p14,p15 = GR_k,GR_Big_Neg_Exp
}
//
//     BIAS_p_K = BIAS + k
//     T = T1 * T2
//
{ .mfi
(p0)   setf.exp FR_Scale = GR_BIAS_p_k  
	nop.f 999
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fnma.s1 FR_r = FR_L_lo, FR_float_N, FR_r 
	nop.i 999
}
//
//     W = W_1_p1 * W2 + W1
//
{ .mfi
(p0)   ldfe FR_A_1 = [GR_Table_Ptr],16 
	nop.f 999
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fadd.s1 FR_W_1_p1 = FR_W1, f1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     k = extr.u(N_fix,0,6)
//     r = r - N * L_lo
//     Load ptr to Table of exponent thresholds.
//
(p0)   fadd.s1 FR_r = FR_r, FR_X_cor 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fmpy.s1 FR_T = FR_T1, FR_T2 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     if k < big_neg_exp, set p14 and Safe=False
//     Load A_2
//
(p0)   fma.s1 FR_W = FR_W2, FR_W_1_p1, FR_W1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fma.s1 FR_poly = FR_r, FR_A_3, FR_A_2 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fmpy.s1 FR_rsq = FR_r, FR_r 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   mov FR_Y_hi = FR_T 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Scale = set_exp(BIAS_p_k)
//     poly = r * poly + A_1
//
(p0)   fadd.s1 FR_Wp1 = FR_W, f1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fma.s1 FR_poly = FR_r, FR_poly, FR_A_1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fma.s1 FR_poly = FR_rsq, FR_poly,FR_r  
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Wp1 =  W + 1
//     poly = rsq * poly + rk
//
(p0)   fma.s1 FR_Y_lo = FR_Wp1, FR_poly, FR_W 
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//
//     Y_lo = poly * Wp1 + W 
//     Y_hi = T
//
(p0)   fmpy.s1 FR_Y_lo = FR_Y_lo, FR_T 
//
//     Y_lo = T * Y_lo
//
(p0)   br.cond.sptk L(EXPL_RETURN) ;; 
}

L(EXPL_SMALL): 

//
//     r4 = rsq * rsq
//

{ .mmi
      nop.m 999
(p0)  addl           GR_Table_Ptr1   = @ltoff(Constants_exp_64_P), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Ptr1 = [GR_Table_Ptr1]
      nop.m 999
      nop.i 999
}
;;

{ .mmf
	nop.m 999
(p0)   ldfe FR_P_6 = [GR_Table_Ptr1],16 
//
//     Return  
//
(p0)   fadd.s1 FR_r = FR_X,f0 ;; 
}

{ .mmi
      nop.m 999
(p0)  addl           GR_Table_Ptr   = @ltoff(Constants_exp_64_Exponents#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Ptr = [GR_Table_Ptr]
(p0)  ldfe FR_P_5 = [GR_Table_Ptr1],16 
      nop.i 999
}
;;

//
//     Is input very small? 
//     Load P_5
//
{ .mii
(p0)   ldfe FR_P_4 = [GR_Table_Ptr1],16 
(p0)   add GR_Table_Ptr = 0x040,GR_Table_Ptr ;;  
(p0)   shladd GR_Table_Ptr = GR_Expo_Range,3,GR_Table_Ptr ;;  
}
{ .mmb
(p0)   ldfe FR_P_3 = [GR_Table_Ptr1],16 
//
//     Adjust ptr.
//
(p0)   ld8  GR_vsm_expo = [GR_Table_Ptr],0
	nop.b 999 ;;
}
{ .mfi
	nop.m 999
//
//     r = X (don't seem to need X_Cor) 
//     Load the threshold exponents
//
(p0)   fmpy.s1 FR_rsq = FR_r, FR_r 
	nop.i 999 ;;
}
//
//     Load the negative integer
//     Load P_5
//
{ .mfi
(p0)   cmp.lt.unc  p12, p0 =  GR_Expo_X, GR_vsm_expo 
	nop.f 999
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//
//     rsq = r * r
//     Offset into exponents
//
(p0)   fmpy.s1 FR_r4 = FR_rsq, FR_rsq 
(p12)  br.cond.spnt L(EXPL_VERY_SMALL) ;; 
}
{ .mfi
(p0)   ldfe FR_P_2 = [GR_Table_Ptr1],16 
//
//     Load p4,p3,p2,p1
//
(p0)   fma.s1 FR_poly_lo = FR_P_6, FR_r, FR_P_5 
//
//     Y_lo = r4 * poly_lo + poly_hi
//     Scale = 1.0
//
(p0)   add GR_temp = 0x1,r0 ;;  
}
{ .mmf
	nop.m 999
(p0)   ldfe FR_P_1 = [GR_Table_Ptr1],0 
(p0)   mov FR_Scale = f1 
}
//
//     Begin creating lsb to perturb final result
//
{ .mfi
(p0)   setf.sig FR_temp = GR_temp 
(p0)   mov FR_Y_hi = f1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     poly_lo = p_5 + p_6 * r
//     poly_hi = p_1 + p_2 * r
//
(p0)   fma.s1 FR_poly_lo = FR_poly_lo, FR_r, FR_P_4 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     poly_lo = p_4 + poly_lo * r
//     poly_hi = r + poly_hi * rsq
//
(p0)   fma.s1 FR_poly_lo = FR_poly_lo, FR_r, FR_P_3 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fma.s1 FR_poly_hi = FR_P_2, FR_r, FR_P_1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fma.s1 FR_poly_hi = FR_poly_hi, FR_rsq, FR_r 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//  
//     poly_lo = p_3 + poly_lo * r
//     Y_hi = 1, always
//  
(p0)   fma.s1 FR_Y_lo = FR_poly_lo, FR_r4, FR_poly_hi 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Set lsb in fp register
// 
(p0)   for FR_temp = FR_Y_lo,FR_temp 
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//
//     Toggle on last bit of Y_lo
//
(p0)   fmerge.se FR_Y_lo = FR_Y_lo,FR_temp 
//
//     Set lsb of Y_lo to 1
//
(p0)   br.cond.sptk L(EXPL_RETURN) ;; 
}
L(EXPL_VERY_SMALL): 
{ .mfi
	nop.m 999
(p0)   mov FR_Y_lo = FR_r 
(p0)   cmp.eq.unc  p15, p0 =  r0, r0 
}
{ .mfi
       nop.m 999
(p0)   mov FR_Scale = f1
       nop.i 999
};;
{ .mfb
	nop.m 999
(p0)   mov FR_Y_hi = f1 
//
//     If flag_not_1, 
//     Y_hi = 1.0 
//     Y_lo = X + X_cor
//     PR_Safe = true
//
(p0)   br.cond.sptk L(EXPL_RETURN) ;; 
}
L(EXPL_HUGE): 
{ .mfi
	nop.m 999
//
//     Return for flag=2 
//
(p0)   fcmp.gt.unc.s1 p12, p13 =  FR_X, f0 
(p0)   cmp.eq.unc  p14, p15 =  r0, r0 ;; 
}
{ .mlx
	nop.m 999
//
//     Set Safe to false
//     Is x > 0
//
(p12)  movl GR_Mask = 0x15DC0 ;; 
}
{ .mlx
(p12)  setf.exp FR_Y_hi = GR_Mask 
(p13)  movl GR_Mask = 0xA240 ;; 
}
{ .mlx
(p13)  setf.exp FR_Y_hi = GR_Mask 
//     
//     x > 0: Create mask for Y_hi = 2**(24,000) 
//     x <= 0: Create mask for Y_hi = 2**(-24,000) 
//
(p13)  movl GR_temp = 0xA1DC ;; 
}
{ .mfi
(p13)  setf.exp FR_Y_lo = GR_temp 
//
//     x < =0: Create mask for 2**(-24,100)
//     x <= 0: Y_lo = w**(-24,100)
//
(p12)  mov FR_Y_lo = f1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p12)  mov FR_Scale =  FR_Y_hi 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     x > 0: Y_lo = 1.0
//     x > 0: Scale = 2**(24,000) 
//
(p13)  mov FR_Scale = FR_Y_hi 
	nop.i 999 ;;
}
L(EXPL_RETURN): 
{ .mfi
	nop.m 999
//
//     Scale = 2**(24,000)
//
//
//     exp(y *ln(x)) almost complete 
//     FR_Scale is Scale
//     f34 is Z_hi 
//     f35 is Z_lo 
//
(p0)   fmpy.s1 FR_Sgn = FR_Scale, FR_Sgn  
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     sgn * scale 
//
(p0)   fmpy.s1 FR_Y_lo = FR_Y_lo,FR_Sgn   
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//
//     Z_lo * (sgn * scale) 
//
(p0)   fma.s0 FR_Result = FR_Y_hi, FR_Sgn, FR_Y_lo  
//
//     Z_hi * (sgn * scale)  + Z_lo
//
(p15)  br.cond.sptk L(POWL_64_RETURN) ;;
}
{ .mfi
	nop.m 999
(p0)   fsetc.s3 0x7F,0x01
	nop.i 999
}
{ .mlx
	nop.m 999
//
//     Z_hi * (sgn * scale)  + Z_lo with wre & td
//     Z_hi * (sgn * scale)  + Z_lo with fz  & td
//
(p0)   movl GR_T1_ptr = 0x00000000013FFF ;;
}
{ .mfi
	nop.m 999
(p0)   fma.s3  FR_Result_small = FR_Y_hi, FR_Sgn, FR_Y_lo  
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fsetc.s3 0x7F,0x40
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Return if no danger of over of underflow.
//
(p0)   fsetc.s2 0x7F,0x42
	nop.i 999;;
}
{ .mfi
	nop.m 999
//
//     S0 user supplied status
//     S2 user supplied status + WRE + TD  (Overflows)
//     S3 user supplied status + FZ + TD   (Underflows)
//
(p0)   fma.s2  FR_Result_big = FR_Y_hi, FR_Sgn, FR_Y_lo  
	nop.i 999 ;;
}
//
//     S0 user supplied status
//     S2 user supplied status + WRE + TD  (Overflows)
//     S3 user supplied status + FZ + TD   (Underflows)
//
//
//     If (Safe) is true, then
//        Compute result using user supplied status field.
//        No overflow or underflow here, but perhaps inexact.
//        Return
//     Else
//       Determine if overflow or underflow  was raised.
//       Fetch +/- overflow threshold for IEEE single, double,
//       double extended
//
{ .mfi
(p0)   setf.exp FR_Big = GR_T1_ptr
(p0)   fsetc.s2 0x7F,0x40
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fclass.m.unc   p11, p0 =  FR_Result_small, 0x00F
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fmerge.ns FR_NBig = FR_Big, FR_Big
	nop.i 999
}
{ .mfi
	nop.m 999
//
//     Create largest double exponent + 1.
//     Create smallest double exponent - 1.
//     Identify denormals
//
(p0)   fcmp.ge.unc.s1 p8, p0 = FR_Result_big , FR_Big
	nop.i 999 ;;
}
{ .mii
	nop.m 999
	nop.i 999 ;;
//
//     fcmp:   resultS2 <= - overflow threshold  
//     fclass: resultS3 is denorm/unorm/0       
//
(p8)   mov   GR_Parameter_TAG = 18 ;;
}
{ .mfb
	nop.m 999
//
//     fcmp:   resultS2 >= + overflow threshold  
//
(p0)   fcmp.le.unc.s1 p9, p0 = FR_Result_big, FR_NBig
(p8)   br.cond.spnt __libm_error_region ;;
}
{ .mii
	nop.m 999
	nop.i 999 ;;
(p9)   mov   GR_Parameter_TAG = 18
}
{ .mib
	nop.m 999
	nop.i 999
(p9)   br.cond.spnt __libm_error_region ;;
}
//
//     Report that pow overflowed - either +Inf, or -Inf
//
{ .mmb
(p11)  mov   GR_Parameter_TAG = 19
	nop.m 999
(p11)  br.cond.spnt __libm_error_region ;;
}
{ .mib
	nop.m 999
	nop.i 999
//
//     Report that pow underflowed
//
(p0)   br.cond.sptk L(POWL_64_RETURN) ;;
}


L(POWL_64_SQUARE):
// Here if x not zero and y=2.
// Must call __libm_error_support for overflow or underflow
//
//     S0 user supplied status
//     S2 user supplied status + WRE + TD  (Overflows)
//     S3 user supplied status + FZ + TD   (Underflows)
//
{ .mfi
	nop.m 999
(p0)   fma.s0 FR_Result = FR_Input_X, FR_Input_X, f0
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fsetc.s3 0x7F,0x01
	nop.i 999
}
{ .mlx
	nop.m 999
(p0)   movl GR_T1_ptr = 0x00000000013FFF ;;
}
{ .mfi
	nop.m 999
(p0)   fma.s3 FR_Result_small = FR_Input_X, FR_Input_X, f0
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fsetc.s3 0x7F,0x40
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Return if no danger of over of underflow.
//
(p0)   fsetc.s2 0x7F,0x42
	nop.i 999;;
}
{ .mfi
	nop.m 999
(p0)   fma.s2 FR_Result_big = FR_Input_X, FR_Input_X, f0
	nop.i 999 ;;
}
//
//     S0 user supplied status
//     S2 user supplied status + WRE + TD  (Overflows)
//     S3 user supplied status + FZ + TD   (Underflows)
//
//
//     If (Safe) is true, then
//        Compute result using user supplied status field.
//        No overflow or underflow here, but perhaps inexact.
//        Return
//     Else
//       Determine if overflow or underflow  was raised.
//       Fetch +/- overflow threshold for IEEE single, double,
//       double extended
//
{ .mfi
(p0)   setf.exp FR_Big = GR_T1_ptr
(p0)   fsetc.s2 0x7F,0x40
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fclass.m.unc   p11, p0 =  FR_Result_small, 0x00F
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fmerge.ns FR_NBig = FR_Big, FR_Big
	nop.i 999
}
{ .mfi
	nop.m 999
//
//     Create largest double exponent + 1.
//     Create smallest double exponent - 1.
//     Identify denormals
//
(p0)   fcmp.ge.unc.s1 p8, p0 = FR_Result_big , FR_Big
	nop.i 999 ;;
}
{ .mii
	nop.m 999
	nop.i 999 ;;
//
//     fcmp:   resultS2 <= - overflow threshold  
//     fclass: resultS3 is denorm/unorm/0       
//
(p8)   mov   GR_Parameter_TAG = 18 ;;
}
{ .mfb
	nop.m 999
//
//     fcmp:   resultS2 >= + overflow threshold  
//
(p0)   fcmp.le.unc.s1 p9, p0 = FR_Result_big, FR_NBig
(p8)   br.cond.spnt __libm_error_region ;;
}
{ .mii
	nop.m 999
	nop.i 999 ;;
(p9)   mov   GR_Parameter_TAG = 18
}
{ .mib
	nop.m 999
	nop.i 999
(p9)   br.cond.spnt __libm_error_region ;;
}
//
//     Report that pow overflowed - either +Inf, or -Inf
//
{ .mmb
(p11)  mov   GR_Parameter_TAG = 19
	nop.m 999
(p11)  br.cond.spnt __libm_error_region ;;
}
{ .mib
	nop.m 999
	nop.i 999
//
//     Report that pow underflowed
//
(p0)   br.cond.sptk L(POWL_64_RETURN) ;;
}




L(POWL_64_SPECIAL): 
{ .mfi
	nop.m 999
(p0)   fcmp.eq.s1 p15, p0 =  FR_Input_X, f1  // Is x=+1
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fclass.m.unc p14, p0 =  FR_Input_Y, 0x023 
	nop.i 999 ;;
}

{ .mfi
	nop.m 999
(p15)   fcmp.eq.unc.s0 p6,p0 = FR_Input_Y, f0 // If x=1, flag invalid if y=SNaN
	nop.i 999 
}
{ .mfb
	nop.m 999
(p15)   fmpy.s0 FR_Result = f1,f1        // If x=1, result=1
(p15)   br.cond.spnt L(POWL_64_RETURN) ;;   // Exit if x=1
}

{ .mfi
	nop.m 999
(p0)   fclass.m.unc p13, p0 =  FR_Input_X, 0x023 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fclass.m.unc p8, p0 =  FR_Input_X, 0x143 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fclass.m.unc p9, p0 =  FR_Input_Y, 0x143 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fclass.m.unc p10, p0 =  FR_Input_X, 0x083 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fclass.m.unc p11, p0 =  FR_Input_Y, 0x083 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fclass.m.unc p6, p0 =  FR_Input_Y, 0x007 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fcmp.eq.unc.s1 p7, p0 =  FR_Input_Y, f1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     set p13 if x +/- Inf 
//     set p14 if y +/- Inf 
//     set p8  if x  Natval or +/-SNaN  
//     set p9  if y  Natval or +/-SNaN  
//     set p10 if x QNaN
//     set p11 if y QNaNs
//     set p6  if y is +/-0 
//     set p7  if y is 1
//
(p8)   fmpy.s0 FR_Result = FR_Input_Y, FR_Input_X 
(p6)   cmp.ne p8,p0 = r0,r0 ;;  // Don't exit if x=snan, y=0 ==> result=+1
}
{ .mfb
	nop.m 999
(p9)   fmpy.s0 FR_Result = FR_Input_Y, FR_Input_X 
(p8)   br.cond.spnt L(POWL_64_RETURN) ;; 
}
{ .mfb
	nop.m 999
(p10)  fmpy.s0 FR_Result = FR_Input_X, f0 
(p9)   br.cond.spnt L(POWL_64_RETURN) ;; 
}
{ .mfi
	nop.m 999
//
//     Produce result for SNaN and NatVals and return
//
(p6)   fclass.m.unc p15, p0 =  FR_Input_X,0x007 
	nop.i 999
}
{ .mfi
	nop.m 999
//
//     If Y +/- 0, set p15 if x +/- 0
//
(p6)   fclass.m.unc p8, p0 =  FR_Input_X,0x0C3 
	nop.i 999 ;;
}

{ .mfi
	nop.m 999
(p6)   fcmp.eq.s0 p9,p0 = FR_Input_X, f0 // If y=0, flag if x denormal
	nop.i 999
}
{ .mfi
	nop.m 999
(p6)   fadd.s0 FR_Result = f1, f0
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
// 
//     Set p8 if y = +/-0 and X is a QNaN/SNaN
//     If y = +/-0, let result = 1.0
// 
(p7)   fmpy.s0 FR_Result = FR_Input_X,f1
//
//     If y == 1, result = x * 1 
//
(p15)  mov GR_Parameter_TAG = 20 
}
{ .mib
	nop.m 999
	nop.i 999
(p15)  br.cond.spnt __libm_error_region ;;
}
{ .mib
	nop.m 999
//
//     If x and y are both zero, result = 1.0 and call error
//     support. 
//
(p8)   mov GR_Parameter_TAG = 23 
(p8)   br.cond.spnt __libm_error_region ;;
}
{ .mib
	nop.m 999
	nop.i 999
//
//     If y = +/-0 and x is a QNaN, result = 1.0 and call error
//     support. 
//
(p6)   br.cond.spnt L(POWL_64_RETURN) ;; 
}

// If x=0, y=-inf, go to the X_IS_ZERO path
{ .mfb
	nop.m 999
(p14)  fcmp.eq.unc.s1 p0,p14 = FR_Input_X,f0 
(p7)   br.cond.spnt L(POWL_64_RETURN) ;; 
}

{ .mfi
	nop.m 999
//
//     Produce all results for x**0 and x**1 
//     Let all the result x ** 0 == 1 and return
//     Let all x ** 1 == x and return
//
(p10)  fmpy.s0 FR_Result = FR_Input_Y,FR_Input_X
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p11)  fmpy.s0 FR_Result = FR_Input_Y,FR_Input_X
(p10)  br.cond.spnt L(POWL_64_RETURN) ;;
}
{ .mib
	nop.m 999
	nop.i 999
(p11)  br.cond.spnt L(POWL_64_RETURN) ;;
}
{ .mib
	nop.m 999
	nop.i 999
//
//     Return result for x or y QNaN input with QNaN result 
//
(p14)  br.cond.spnt L(POWL_64_Y_IS_INF) ;;
}
{ .mib
	nop.m 999
	nop.i 999
(p13)  br.cond.spnt L(POWL_64_X_IS_INF) ;;
}
L(POWL_64_X_IS_ZERO): 
{ .mmb
(p0)   getf.sig GR_signif_y = FR_Input_Y 
(p0)   getf.exp GR_BIASed_exp_y = FR_Input_Y
	nop.b 999 ;;
}
{ .mlx
	nop.m 999
(p0)   movl GR_Mask = 0x1FFFF
}
{ .mlx
	nop.m 999
(p0)   movl GR_y_sign = 0x20000 ;;
}
//
//     Get BIASed exp and significand of y
//
{ .mfi
(p0)   and GR_exp_y = GR_Mask,GR_BIASed_exp_y
	nop.f 999
(p0)   and GR_y_sign = GR_y_sign,GR_BIASed_exp_y
}
{ .mlx
	nop.m 999
(p0)   movl GR_BIAS = 0xFFFF ;;
}
{ .mfi
(p0)   cmp.lt.unc  p9, p8 = GR_exp_y,GR_BIAS
	nop.f 999
//
//     Maybe y is < 1 already, so
//     can never be an integer.
//     Remove sign bit from exponent.
//
(p0)   sub GR_exp_y = GR_exp_y,GR_BIAS ;;
}
{ .mii
	nop.m 999
	nop.i 999 ;;
//
//     Remove exponent BIAS
//
(p8)   shl GR_exp_y=  GR_signif_y,GR_exp_y ;;
}
{ .mfi
(p9)   or  GR_exp_y=  0xF,GR_signif_y
	nop.f 999
	nop.i 999 ;;
}
{ .mii
	nop.m 999
//
//     Shift significand of y looking for nonzero bits
//     For y > 1, shift signif_y exp_y bits to the left
//     For y < 1, turn on 4 low order bits of significand of y 
//     so that the fraction will always be non-zero
//
(p0)   shl GR_signif_y=  GR_exp_y,1 ;;
(p0)   extr.u GR_low_order_bit = GR_exp_y,63,1
}
//
//     Integer part of y  shifted off.
//     Get y's low even or odd bit - y might not be an int.
//
{ .mii
(p0)   cmp.eq.unc  p13,p0  =  GR_signif_y, r0
(p0)   cmp.eq.unc  p8,p9 =  GR_y_sign, r0 ;;
//
//     Is y an int?
//     Is y positive
//
(p13)  cmp.ne.unc  p13,p0 =  GR_low_order_bit, r0 ;;
}
//
//     Is y and int and odd?
//
{ .mfb
(p13)  cmp.eq.unc  p13,p14 =  GR_y_sign, r0
(p8)   fcmp.eq.s0 p12,p0 = FR_Input_Y, f0 // If x=0 and y>0 flag if y denormal
	nop.b 999 ;;
}
{ .mfb
	nop.m 999
//
//     Is y and int and odd and positive?
//
(p13)  mov FR_Result = FR_Input_X 
(p13)  br.cond.sptk L(POWL_64_RETURN) ;;
}
{ .mfi
	nop.m 999
//
//     Return +/-0 when x=+/-0 and y is and odd pos. int
//
(p14)  frcpa.s0 FR_Result, p10 = f1, FR_Input_X
(p14)  mov GR_Parameter_TAG = 21
}
{ .mib
	nop.m 999
	nop.i 999
(p14)  br.cond.spnt __libm_error_region ;;
}

{ .mfb
	nop.m 999
//
//     Return +/-0 when x=+/-Inf and y is and odd neg int
//     and raise dz exception
//
(p8)   mov FR_Result = f0
(p8)   br.cond.sptk L(POWL_64_RETURN) ;;
}
{ .mfi
	nop.m 999
//
//     Return +0 when x=+/-0 and y > 0  and not odd.
//
(p9)   frcpa.s0 FR_Result, p10 = f1,f0
(p9)   mov GR_Parameter_TAG = 21
}
{ .mib
	nop.m 999
	nop.i 999
(p9)   br.cond.sptk __libm_error_region ;;
}
L(POWL_64_X_IS_INF): 
{ .mfi
(p0)   getf.exp GR_exp_y = FR_Input_Y
(p0)   fclass.m.unc p13, p0 =  FR_Input_X,0x022 
(p0)   mov GR_Mask = 0x1FFFF ;;
}

{ .mfi
(p0)   getf.sig GR_signif_y = FR_Input_Y
(p0)   fcmp.eq.s0 p9,p0 = FR_Input_Y, f0 // Flag if y denormal
       nop.i 999 ;;
}

//
//     Get exp and significand of y
//     Create exponent mask and sign mask
//
{ .mlx
(p0)   and GR_low_order_bit = GR_Mask,GR_exp_y
(p0)   movl GR_BIAS = 0xFFFF
}
{ .mmi
	nop.m 999 ;;
//
//     Remove sign bit from exponent.
//
(p0)   cmp.lt.unc  p9, p8 = GR_low_order_bit,GR_BIAS
//
//     Maybe y is < 1 already, so 
//     isn't an int.
//
(p0)   sub GR_low_order_bit = GR_low_order_bit,GR_BIAS
}
{ .mlx
	nop.m 999
(p0)   movl GR_sign_mask = 0x20000 ;;
}
{ .mfi
(p0)   and GR_sign_mask = GR_sign_mask,GR_exp_y
//
//     Return +Inf when x=+/-0 and y < 0 and not odd and raise
//     divide-by-zero exception.
//
(p0)   fclass.m.unc p11, p0 =  FR_Input_X,0x021 
	nop.i 999 ;;
}
{ .mmi
	nop.m 999 ;;
//
//     Is shift off integer part of y.
//     Get y's even or odd bit - y might not be an int.
//
(p11)  cmp.eq.unc  p11,p12 = GR_sign_mask, r0
//
//     Remove exponent BIAS
//
(p8)   shl GR_exp_y = GR_signif_y,GR_low_order_bit ;;
}
{ .mfi
(p9)   or  GR_exp_y = 0xF,GR_signif_y
//
//     Is y positive or negative when x is +Inf?
//     Is y and int when x = -Inf 
//
(p11)  mov FR_Result = FR_Input_X 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p12)  mov FR_Result = f0
	nop.i 999 ;;
}
{ .mii
	nop.m 999
//
//     Shift signficand looking for nonzero bits 
//     For y non-ints, upset the significand.
//
(p0)   shl GR_signif_y = GR_exp_y,1 ;;
(p13)  cmp.eq.unc  p13,p0  = GR_signif_y, r0
}
{ .mii
	nop.m 999
(p0)   extr.u GR_low_order_bit = GR_exp_y,63,1 ;;
(p13)  cmp.ne.unc  p13,p0 = GR_low_order_bit, r0
}
{ .mib
	nop.m 999
	nop.i 999
(p11)  br.cond.sptk L(POWL_64_RETURN) ;; 
}
{ .mib
	nop.m 999
	nop.i 999
(p12)  br.cond.sptk L(POWL_64_RETURN) ;; 
}
//
//     Return Inf for y > 0
//     Return +0  for y < 0
//     Is y even or odd?
//
{ .mii
(p13)  cmp.eq.unc  p13,p10 = GR_sign_mask, r0
(p0)   cmp.eq.unc  p8,p9 = GR_sign_mask, r0 ;;
	nop.i 999
}
{ .mfi
	nop.m 999
//
//     For x = -inf, y is and int, positive  
//     and odd 
//     Is y positive in general?
//
(p13)  mov FR_Result = FR_Input_X
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p10)  fmerge.ns FR_Result = f0, f0 
(p13)  br.cond.sptk L(POWL_64_RETURN) ;; 
}
{ .mib
	nop.m 999
	nop.i 999
(p10)  br.cond.sptk L(POWL_64_RETURN) ;; 
}
{ .mfi
	nop.m 999
//
//     Return -Inf for x = -inf and y > 0 and odd int.
//     Return -0   for x = -inf and y < 0 and odd int.
//
(p8)   fmerge.ns FR_Result = FR_Input_X, FR_Input_X 
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p9)   mov FR_Result = f0
(p8)   br.cond.sptk L(POWL_64_RETURN) ;; 
}
{ .mib
	nop.m 999
	nop.i 999
(p9)   br.cond.sptk L(POWL_64_RETURN) ;; 
}
L(POWL_64_Y_IS_INF): 
{ .mfi
	nop.m 999
//
//     Return Inf for x = -inf and y > 0 not an odd int.
//     Return +0  for x = -inf and y < 0 and not an odd int.
//
(p0)   fclass.m.unc p8, p0 =  FR_Input_Y, 0x021
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fclass.m.unc p9, p0 =  FR_Input_Y, 0x022
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fabs FR_X = FR_Input_X
	nop.i 999 ;;
}

{ .mfi
	nop.m 999
(p0)   fcmp.eq.s0 p10,p0 = FR_Input_X, f0 // flag if x denormal
	nop.i 999 ;;
}

{ .mfi
	nop.m 999
//
//     Find y = +/- Inf
//     Compute |x|
//
(p8)   fcmp.lt.unc.s1 p6, p0  =  FR_X, f1
	nop.i 999
}
{ .mfi
	nop.m 999
(p8)   fcmp.gt.unc.s1 p7, p0  =  FR_X, f1
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p9)   fcmp.lt.unc.s1 p12, p0 =  FR_X, f1
	nop.i 999
}
{ .mfi
	nop.m 999
(p9)   fcmp.gt.unc.s1 p13, p0 =  FR_X, f1
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     For y = +Inf and |x| < 1  returns 0
//     For y = +Inf and |x| > 1  returns Inf
//     For y = -Inf and |x| < 1  returns Inf
//     For y = -Inf and |x| > 1  returns 0
//
(p6)   mov FR_Result = f0
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p7)   mov FR_Result = FR_Input_Y 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p12)  fmpy.s0 FR_Result = FR_Input_Y, FR_Input_Y
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p13)  mov FR_Result = f0
//
//     Produce x ** +/- Inf results
//
(p6)   br.cond.spnt L(POWL_64_RETURN) ;;
}
{ .mib
	nop.m 999
	nop.i 999
(p7)   br.cond.spnt L(POWL_64_RETURN) ;;
}
{ .mib
	nop.m 999
	nop.i 999
(p12)  br.cond.spnt L(POWL_64_RETURN) ;;
}
{ .mib
	nop.m 999
	nop.i 999
(p13)  br.cond.spnt L(POWL_64_RETURN) ;;
}
{ .mfb
	nop.m 999
//
//     +/-1 ** +/-Inf, result is +1
//
(p0)   fmpy.s0 FR_Result = f1,f1
(p0)   br.cond.sptk L(POWL_64_RETURN) ;;
}
L(POWL_64_UNSUPPORT): 
{ .mfb
	nop.m 999
//
//     Return NaN and raise invalid    
//
(p0)   fmpy.s0 FR_Result = FR_Input_X,f0
// 
//     Raise exceptions for specific
//     values - pseudo NaN and
//     infinities.  
// 
(p0)   br.cond.sptk L(POWL_64_RETURN) ;; 
}
L(POWL_64_XNEG): 
{ .mfi
	nop.m 999
(p0)   frcpa.s0 FR_Result, p8 =  f0, f0
// 
//     Raise invalid for x < 0  and
//     y not an integer and
// 
(p0)   mov GR_Parameter_TAG = 22
}
{ .mib
	nop.m 999
	nop.i 999
(p0)   br.cond.sptk __libm_error_region ;; 
}
L(POWL_64_SQRT): 
{ .mfi
	nop.m 999
(p0)   frsqrta.s0 FR_Result,p10 = FR_Input_X
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p10)  fma.s1   f62=FR_Half,FR_Input_X,f0
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Step (2)
//     h = 1/2 * a in f9
//
(p10)  fma.s1   f63=FR_Result,FR_Result,f0
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Step (3)
//     t1 = y0 * y0 in f10
//
(p10)  fnma.s1  f32=f63,f62,f11
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Step (4)
//     t2 = 1/2 - t1 * h in f10
//
(p10)  fma.s1   f33=f32,FR_Result,FR_Result
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Step (5)
//     y1 = y0 + t2 * y0 in f13
//
(p10)  fma.s1   f34=f33,f62,f0
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Step (6)
//     t3 = y1 * h in f10
//
(p10)  fnma.s1  f35=f34,f33,f11
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Step (7)
//     t4 = 1/2 - t3 * y1 in f10
//
(p10)  fma.s1   f63=f35,f33,f33
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Step (8)
//     y2 = y1 + t4 * y1 in f13
//
(p10)  fma.s1   f32=FR_Input_X,f63,f0
	nop.i 999
}
{ .mfi
	nop.m 999
//
//     Step (9)
//     S = a * y2 in f10
//
(p10)  fma.s1   FR_Result=f63,f62,f0
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Step (10)
//     t5 = y2 * h in f9
//
(p10)  fma.s1   f33=f11,f63,f0
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Step (11)
//     H = 1/2 * y2 in f11
//
(p10)  fnma.s1  f34=f32,f32,f8
	nop.i 999
}
{ .mfi
	nop.m 999
//
//     Step (12)
//     d = a - S * S in f12
//
(p10)  fnma.s1  f35=FR_Result,f63,f11
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Step (13)
//     t6 = 1/2 - t5 * y2 in f7
//
(p10)  fma.s1   f62=f33,f34,f32
	nop.i 999
}
{ .mfi
	nop.m 999
//
//     Step (14)
//     S1 = S + d * H in f13
//
(p10)  fma.s1   f63=f33,f35,f33
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Step (15)
//     H1 = H + t6 * h in f7
//
(p10)  fnma.s1  f32=f62,f62,FR_Input_X
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//
//     Step (16)
//     d1 = a - S1 * S1 
//
(p10)  fma.s0 FR_Result=f32,f63,f62
//
//     Step (17)
//     R = S1 + d1 * H1 
//
(p10)  br.cond.sptk L(POWL_64_RETURN) ;; 
}
{ .mib
	nop.m 999
	nop.i 999
//
//     Do the Newton-Raphson iteration from the EAS.
//
(p0)   br.cond.sptk L(POWL_64_RETURN) ;; 
}
//
//     Take care of the degenerate cases.
//

L(POWL_64_RETURN):
{ .mfb
       nop.m 999
(p0)   mov   FR_Output = FR_Result
(p0)   br.ret.sptk   b0 ;;
}
.endp powl
ASM_SIZE_DIRECTIVE(powl)

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
        stfe [GR_Parameter_Y] = FR_Y,16         // Save Parameter 2 on stack
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
        stfe [GR_Parameter_Y] = FR_RESULT      // Store Parameter 3 on stack
        add   GR_Parameter_Y = -16,GR_Parameter_Y
        br.call.sptk b0=__libm_error_support#  // Call error handling function
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
