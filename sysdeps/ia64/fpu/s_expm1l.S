.file "exp_m1l.s"

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
// 4/04/00  Unwind support added
// 8/15/00  Bundle added after call to __libm_error_support to properly
//          set [the previously overwritten] GR_Parameter_RESULT.
//
// ********************************************************************* 
//
// Function:   Combined expl(x) and expm1l(x), where
//                        x 
//             expl(x) = e , for double-extended precision x values
//                          x
//             expm1l(x) = e  - 1  for double-extended precision x values
//
// ********************************************************************* 
//
// Resources Used:
//
//    Floating-Point Registers: f8  (Input and Return Value) 
//                              f9,f32-f61, f99-f102 
//
//    General Purpose Registers: 
//      r32-r61
//      r62-r65 (Used to pass arguments to error handling routine)
//                                     
//    Predicate Registers:      p6-p15
//
// ********************************************************************* 
//
// IEEE Special Conditions:
//
//    Denormal  fault raised on denormal inputs  
//    Overflow exceptions raised when appropriate for exp and expm1
//    Underflow exceptions raised when appropriate for exp and expm1
//    (Error Handling Routine called for overflow and Underflow)
//    Inexact raised when appropriate by algorithm 
//
//    expl(inf) = inf
//    expl(-inf) = +0
//    expl(SNaN) = QNaN
//    expl(QNaN) = QNaN
//    expl(0) = 1
//    expl(EM_special Values) = QNaN
//    expl(inf) = inf
//    expm1l(-inf) = -1 
//    expm1l(SNaN) = QNaN
//    expm1l(QNaN) = QNaN
//    expm1l(0) = 0
//    expm1l(EM_special Values) = QNaN
//    
// ********************************************************************* 
//
// Implementation and Algorithm Notes:
//
//  ker_exp_64( in_FR  : X,
//            in_GR  : Flag,
//            in_GR  : Expo_Range
//            out_FR : Y_hi,
//            out_FR : Y_lo,
//            out_FR : scale,
//            out_PR : Safe )
//
// On input, X is in register format and 
// Flag  = 0 for exp,
// Flag  = 1 for expm1,
//
// On output, provided X and X_cor are real numbers, then
//
//   scale*(Y_hi + Y_lo)  approximates  expl(X)       if Flag is 0
//   scale*(Y_hi + Y_lo)  approximates  expl(X)-1     if Flag is 1
//
// The accuracy is sufficient for a highly accurate 64 sig.
// bit implementation.  Safe is set if there is no danger of 
// overflow/underflow when the result is composed from scale, 
// Y_hi and Y_lo. Thus, we can have a fast return if Safe is set. 
// Otherwise, one must prepare to handle the possible exception 
// appropriately.  Note that SAFE not set (false) does not mean 
// that overflow/underflow will occur; only the setting of SAFE
// guarantees the opposite.
//
// **** High Level Overview **** 
//
// The method consists of three cases.
// 
// If           |X| < Tiny	use case exp_tiny;
// else if	|X| < 2^(-6)	use case exp_small;
// else		use case exp_regular;
//
// Case exp_tiny:
//
//   1 + X     can be used to approximate expl(X) or expl(X+X_cor);
//   X + X^2/2 can be used to approximate expl(X) - 1
//
// Case exp_small:
//
//   Here, expl(X), expl(X+X_cor), and expl(X) - 1 can all be 
//   appproximated by a relatively simple polynomial.
//
//   This polynomial resembles the truncated Taylor series
//
//	expl(w) = 1 + w + w^2/2! + w^3/3! + ... + w^n/n!
//
// Case exp_regular:
//
//   Here we use a table lookup method. The basic idea is that in
//   order to compute expl(X), we accurately decompose X into
//
//   X = N * log(2)/(2^12)  + r,	|r| <= log(2)/2^13.
//
//   Hence
//
//   expl(X) = 2^( N / 2^12 ) * expl(r).
//
//   The value 2^( N / 2^12 ) is obtained by simple combinations
//   of values calculated beforehand and stored in table; expl(r)
//   is approximated by a short polynomial because |r| is small.
//
//   We elaborate this method in 4 steps.
//
//   Step 1: Reduction
//
//   The value 2^12/log(2) is stored as a double-extended number
//   L_Inv.
//
//   N := round_to_nearest_integer( X * L_Inv )
//
//   The value log(2)/2^12 is stored as two numbers L_hi and L_lo so
//   that r can be computed accurately via
//
//   r := (X - N*L_hi) - N*L_lo
//
//   We pick L_hi such that N*L_hi is representable in 64 sig. bits
//   and thus the FMA   X - N*L_hi   is error free. So r is the 
//   1 rounding error from an exact reduction with respect to 
//   
//   L_hi + L_lo.
//
//   In particular, L_hi has 30 significant bit and can be stored
//   as a double-precision number; L_lo has 64 significant bits and
//   stored as a double-extended number.
//
//   In the case Flag = 2, we further modify r by
//
//   r := r + X_cor.
//
//   Step 2: Approximation
//
//   expl(r) - 1 is approximated by a short polynomial of the form
//   
//   r + A_1 r^2 + A_2 r^3 + A_3 r^4 .
//
//   Step 3: Composition from Table Values 
//
//   The value 2^( N / 2^12 ) can be composed from a couple of tables
//   of precalculated values. First, express N as three integers
//   K, M_1, and M_2 as
//
//     N  =  K * 2^12  + M_1 * 2^6 + M_2
//
//   Where 0 <= M_1, M_2 < 2^6; and K can be positive or negative.
//   When N is represented in 2's complement, M_2 is simply the 6
//   lsb's, M_1 is the next 6, and K is simply N shifted right
//   arithmetically (sign extended) by 12 bits.
//
//   Now, 2^( N / 2^12 ) is simply  
//	
//      2^K * 2^( M_1 / 2^6 ) * 2^( M_2 / 2^12 )
//
//   Clearly, 2^K needs no tabulation. The other two values are less
//   trivial because if we store each accurately to more than working
//   precision, than its product is too expensive to calculate. We
//   use the following method.
//
//   Define two mathematical values, delta_1 and delta_2, implicitly
//   such that
//
//     T_1 = expl( [M_1 log(2)/2^6]  -  delta_1 ) 
//     T_2 = expl( [M_2 log(2)/2^12] -  delta_2 )
//
//   are representable as 24 significant bits. To illustrate the idea,
//   we show how we define delta_1: 
//
//     T_1     := round_to_24_bits( expl( M_1 log(2)/2^6 ) )
//     delta_1  = (M_1 log(2)/2^6) - log( T_1 )  
//
//   The last equality means mathematical equality. We then tabulate
//
//     W_1 := expl(delta_1) - 1
//     W_2 := expl(delta_2) - 1
//
//   Both in double precision.
//
//   From the tabulated values T_1, T_2, W_1, W_2, we compose the values
//   T and W via
//
//     T := T_1 * T_2			...exactly
//     W := W_1 + (1 + W_1)*W_2	
//
//   W approximates expl( delta ) - 1  where delta = delta_1 + delta_2.
//   The mathematical product of T and (W+1) is an accurate representation
//   of 2^(M_1/2^6) * 2^(M_2/2^12).
//
//   Step 4. Reconstruction
//
//   Finally, we can reconstruct expl(X), expl(X) - 1. 
//   Because
//
//	X = K * log(2) + (M_1*log(2)/2^6  - delta_1) 
//		       + (M_2*log(2)/2^12 - delta_2)
//		       + delta_1 + delta_2 + r 		...accurately
//   We have
//
//	expl(X) ~=~ 2^K * ( T + T*[expl(delta_1+delta_2+r) - 1] )
//	       ~=~ 2^K * ( T + T*[expl(delta + r) - 1]         )
//	       ~=~ 2^K * ( T + T*[(expl(delta)-1)  
//				 + expl(delta)*(expl(r)-1)]   )
//             ~=~ 2^K * ( T + T*( W + (1+W)*poly(r) ) )
//             ~=~ 2^K * ( Y_hi  +  Y_lo )
//
//   where Y_hi = T  and Y_lo = T*(W + (1+W)*poly(r))
//
//   For expl(X)-1, we have
//
//	expl(X)-1 ~=~ 2^K * ( Y_hi + Y_lo ) - 1
//		 ~=~ 2^K * ( Y_hi + Y_lo - 2^(-K) )
//
//   and we combine Y_hi + Y_lo - 2^(-N)  into the form of two 
//   numbers  Y_hi + Y_lo carefully.
//
//   **** Algorithm Details ****
//
//   A careful algorithm must be used to realize the mathematical ideas
//   accurately. We describe each of the three cases. We assume SAFE
//   is preset to be TRUE.
//
//   Case exp_tiny:
//
//   The important points are to ensure an accurate result under 
//   different rounding directions and a correct setting of the SAFE 
//   flag.
//
//   If Flag is 1, then
//      SAFE  := False	...possibility of underflow
//      Scale := 1.0
//      Y_hi  := X
//      Y_lo  := 2^(-17000)
//   Else
//      Scale := 1.0
//      Y_hi  := 1.0
//      Y_lo  := X	...for different rounding modes
//   Endif
//
//   Case exp_small:
//
//   Here we compute a simple polynomial. To exploit parallelism, we split
//   the polynomial into several portions.
//
//   Let r = X 
//
//   If Flag is not 1	...i.e. expl( argument )
//
//      rsq := r * r; 
//      r4  := rsq*rsq
//      poly_lo := P_3 + r*(P_4 + r*(P_5 + r*P_6))
//      poly_hi := r + rsq*(P_1 + r*P_2)
//      Y_lo    := poly_hi + r4 * poly_lo
//      set lsb(Y_lo) to 1
//      Y_hi    := 1.0
//      Scale   := 1.0
//
//   Else			...i.e. expl( argument ) - 1
//
//      rsq := r * r
//      r4  := rsq * rsq
//      r6  := rsq * r4
//      poly_lo := r6*(Q_5 + r*(Q_6 + r*Q_7))
//      poly_hi := Q_1 + r*(Q_2 + r*(Q_3 + r*Q_4))
//      Y_lo    := rsq*poly_hi +  poly_lo
//      set lsb(Y_lo) to 1
//      Y_hi    := X
//      Scale   := 1.0
//
//   Endif
//
//  Case exp_regular:
//
//  The previous description contain enough information except the
//  computation of poly and the final Y_hi and Y_lo in the case for
//  expl(X)-1.
//
//  The computation of poly for Step 2:
//
//   rsq := r*r
//   poly := r + rsq*(A_1 + r*(A_2 + r*A_3))
//
//  For the case expl(X) - 1, we need to incorporate 2^(-K) into
//  Y_hi and Y_lo at the end of Step 4.
//
//   If K > 10 then
//      Y_lo := Y_lo - 2^(-K)
//   Else
//      If K < -10 then
//	 Y_lo := Y_hi + Y_lo
//	 Y_hi := -2^(-K)
//      Else
//	 Y_hi := Y_hi - 2^(-K)
//      End If
//   End If
//

#include "libm_support.h"

#ifdef _LIBC
.rodata
#else
.data
#endif

.align 64 
Constants_exp_64_Arg:
ASM_TYPE_DIRECTIVE(Constants_exp_64_Arg,@object)
data4 0x5C17F0BC,0xB8AA3B29,0x0000400B,0x00000000 
data4 0x00000000,0xB17217F4,0x00003FF2,0x00000000
data4 0xF278ECE6,0xF473DE6A,0x00003FD4,0x00000000
// /* Inv_L, L_hi, L_lo */
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
data4 0xB1B736A0,0xAAAAAAAB,0x00003FFA,0x00000000
data4 0x90CD6327,0xAAAAAAAB,0x00003FFC,0x00000000
data4 0xFFFFFFFF,0xFFFFFFFF,0x00003FFD,0x00000000
// /* Reversed */
ASM_SIZE_DIRECTIVE(Constants_exp_64_A)

.align 64 
Constants_exp_64_P:
ASM_TYPE_DIRECTIVE(Constants_exp_64_P,@object)
data4 0x43914A8A,0xD00D6C81,0x00003FF2,0x00000000
data4 0x30304B30,0xB60BC4AC,0x00003FF5,0x00000000
data4 0x7474C518,0x88888888,0x00003FF8,0x00000000
data4 0x8DAE729D,0xAAAAAAAA,0x00003FFA,0x00000000
data4 0xAAAAAF61,0xAAAAAAAA,0x00003FFC,0x00000000
data4 0x000004C7,0x80000000,0x00003FFE,0x00000000 
// /* Reversed */
ASM_SIZE_DIRECTIVE(Constants_exp_64_P)

.align 64 
Constants_exp_64_Q:
ASM_TYPE_DIRECTIVE(Constants_exp_64_Q,@object)
data4 0xA49EF6CA,0xD00D56F7,0x00003FEF,0x00000000
data4 0x1C63493D,0xD00D59AB,0x00003FF2,0x00000000
data4 0xFB50CDD2,0xB60B60B5,0x00003FF5,0x00000000
data4 0x7BA68DC8,0x88888888,0x00003FF8,0x00000000
data4 0xAAAAAC8D,0xAAAAAAAA,0x00003FFA,0x00000000
data4 0xAAAAACCA,0xAAAAAAAA,0x00003FFC,0x00000000
data4 0x00000000,0x80000000,0x00003FFE,0x00000000 
// /* Reversed */
ASM_SIZE_DIRECTIVE(Constants_exp_64_Q)

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

GR_SAVE_PFS         = r59
GR_SAVE_B0          = r60
GR_SAVE_GP          = r61
GR_Parameter_X      = r62
GR_Parameter_Y      = r63
GR_Parameter_RESULT = r64
GR_Parameter_TAG    = r65 

FR_X                = f9
FR_Y                = f9
FR_RESULT           = f99

.section .text
.proc expm1l#
.global expm1l#
.align 64 
expm1l: 
#ifdef _LIBC
.global __expm1l#
__expm1l:
#endif
{ .mii
alloc r32 = ar.pfs,0,30,4,0
(p0)  add r33 = 1, r0  
(p0)  cmp.eq.unc  p7, p0 =  r0, r0 
}
{ .mbb
	nop.m 999
(p0)  br.cond.sptk exp_continue 
	nop.b 999 ;;
}

//
//    Set p7 true for expm1
//    Set Flag = r33 = 1 for expm1
//    

.endp expm1l
ASM_SIZE_DIRECTIVE(expm1l)

.section .text
.proc expl#
.global expl#
.align 64 
expl: 
#ifdef _LIBC
.global __ieee754_expl#
__ieee754_expl:
#endif
{ .mii
alloc r32 = ar.pfs,0,30,4,0
(p0)  add r33 = r0, r0  
(p0)  cmp.eq.unc  p0, p7 =  r0, r0 ;; 
}
exp_continue: 
{ .mfi
(p0)  add r32 = 2,r0  
(p0)  fnorm.s1 f9 = f8 
      nop.i 0
}
{ .mfi
(p0)  nop.m 0 
//
//    Set p7 false for exp
//    Set Flag = r33 = 0 for exp
//    
(p0)  fclass.m.unc p6, p8 =  f8, 0x1E7 
      nop.i 0;;
}
{ .mfi
	nop.m 999
(p0)  fclass.nm.unc p9, p0 =  f8, 0x1FF 
      nop.i 0
}
{ .mfi
	nop.m 999
(p0)  mov f36 = f1 
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//     
//    Identify NatVals, NaNs, Infs, and Zeros. 
//    Identify EM unsupporteds. 
//    Save special input registers 
(p0)  mov f32 = f0 
//
//    Create FR_X_cor      = 0.0 
//           GR_Flag       = 0 
//           GR_Expo_Range = 2 (r32) for double-extended precision 
//           FR_Scale      = 1.0
//
(p6)  br.cond.spnt EXPL_64_SPECIAL ;; 
}
{ .mib
	nop.m 999
	nop.i 999
(p9)  br.cond.spnt EXPL_64_UNSUPPORTED ;; 
}
{ .mfi
(p0)  cmp.ne.unc p12, p13 = 0x01, r33
//     
//    Branch out for special input values 
//     
(p0)  fcmp.lt.unc.s0 p9,p0 =  f8, f0 
(p0)  cmp.eq.unc  p15, p0 =  r0, r0 
}
{ .mmi
	nop.m 999
//     
//    Raise possible denormal operand exception 
//    Normalize x 
//     
//    This function computes expl( x  + x_cor) 
//    Input  FR 1: FR_X            
//    Input  FR 2: FR_X_cor  
//    Input  GR 1: GR_Flag  
//    Input  GR 2: GR_Expo_Range  
//    Output FR 3: FR_Y_hi  
//    Output FR 4: FR_Y_lo  
//    Output FR 5: FR_Scale  
//    Output PR 1: PR_Safe  
(p0)  addl r34 = @ltoff(Constants_exp_64_Arg#),gp  
(p0)  addl r40 = @ltoff(Constants_exp_64_W1#),gp 
};;
//
//    Prepare to load constants
//    Set Safe = True
//

{ .mmi
      ld8  r34 = [r34]
      ld8  r40 = [r40]
(p0)  addl r41 = @ltoff(Constants_exp_64_W2#),gp  
};;

{ .mmi
(p0)  ldfe f37 = [r34],16 
(p0)  ld8 r41 = [r41] ;; 
}

//
//    N = fcvt.fx(float_N)
//    Set p14 if -6 > expo_X 
//
//
//    Bias = 0x0FFFF
//    expo_X = expo_X and Mask  
//

{ .mmi
(p0)  ldfe f40 = [r34],16 
      nop.m 999
//
//    Load L_lo
//    Set p10 if 14 < expo_X 
//
(p0)  addl r50 = @ltoff(Constants_exp_64_T1#),gp 
}
{ .mmi
	nop.m 999
	nop.m 999
(p0)  addl r51 = @ltoff(Constants_exp_64_T2#),gp ;; 
}
//
//    Load W2_ptr
//    Branch to SMALL is expo_X < -6
//

{.mmi
(p0)  ld8 r50 = [r50]  
(p0)  ld8 r51 = [r51]  
};;

{ .mlx
(p0)  ldfe f41 = [r34],16 
//
//    float_N = X * L_Inv
//    expo_X = exponent of X
//    Mask = 0x1FFFF
//
(p0)  movl r58 = 0x0FFFF 
}
{ .mlx
	nop.m 999
(p0)  movl r39 = 0x1FFFF ;; 
}
{ .mmi
(p0)  getf.exp r37 = f9 
	nop.m 999
(p0)  addl r34 = @ltoff(Constants_exp_64_Exponents#),gp ;; 
}
{ .mii
(p0)  ld8 r34 = [r34]  
      nop.i 999 
(p0)  and  r37 = r37, r39 ;;  
}
{ .mmi
(p0)  sub r37 = r37, r58 ;;  
(p0)  cmp.gt.unc  p14, p0 =  -6, r37 
(p0)  cmp.lt.unc  p10, p0 =  14, r37 ;; 
}
{ .mfi
(p0)  nop.m 0  
//
//    Load L_inv 
//    Set p12 true for Flag = 0 (exp)
//    Set p13 true for Flag = 1 (expm1)
//
(p0)  fmpy.s1 f38 = f9, f37 
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//
//    Load L_hi
//    expo_X = expo_X - Bias
//    get W1_ptr      
//
(p0)  fcvt.fx.s1 f39 = f38
(p14) br.cond.spnt EXPL_SMALL ;; 
}
{ .mib
	nop.m 999
	nop.i 999
(p10) br.cond.spnt EXPL_HUGE ;; 
}
{ .mmi
(p0)  shladd r34 = r32,4,r34 
      nop.m 999
(p0)  addl r35 = @ltoff(Constants_exp_64_A#),gp ;; 
}
//
//    Load T_1,T_2
//
{ .mmi
   nop.m 999
   ld8   r35 =[r35]
   nop.i 99
};;
{ .mmb
(p0)  ldfe f51 = [r35],16 
(p0)  ld8 r45 = [r34],8
	nop.b 999 ;;
}
//    
//    Set Safe = True  if k >= big_expo_neg  
//    Set Safe = False if k < big_expo_neg  
//    
{ .mmb
(p0)  ldfe f49 = [r35],16 
(p0)  ld8 r48 = [r34],0
	nop.b 999 ;;
}
{ .mfi
	nop.m 999
//
//    Branch to HUGE is expo_X > 14 
//
(p0)  fcvt.xf f38 = f39 
	nop.i 999 ;;
}
{ .mfi
(p0)  getf.sig r52 = f39 
	nop.f 999
	nop.i 999 ;;
}
{ .mii
	nop.m 999
(p0)  extr.u r43 = r52, 6, 6 ;;  
//
//    r = r - float_N * L_lo
//    K = extr(N_fix,12,52)
//
(p0)  shladd r40 = r43,3,r40 ;; 
}
{ .mfi
(p0)  shladd r50 = r43,2,r50 
(p0)  fnma.s1 f42 = f40, f38, f9 
//
//    float_N = float(N)
//    N_fix = signficand N 
//
(p0)  extr.u r42 = r52, 0, 6  
}
{ .mmi
(p0)  ldfd  f43 = [r40],0 ;; 
(p0)  shladd r41 = r42,3,r41 
(p0)  shladd r51 = r42,2,r51 
}
//
//    W_1_p1 = 1 + W_1
//
{ .mmi
(p0)  ldfs  f44 = [r50],0 ;; 
(p0)  ldfd  f45 = [r41],0 
//
//    M_2 = extr(N_fix,0,6)
//    M_1 = extr(N_fix,6,6)
//    r = X - float_N * L_hi
//
(p0)  extr r44 = r52, 12, 52  
}
{ .mmi
(p0)  ldfs  f46 = [r51],0 ;; 
(p0)  sub r46 = r58, r44  
(p0)  cmp.gt.unc  p8, p15 =  r44, r45 
}
//    
//    W = W_1 + W_1_p1*W_2 
//    Load  A_2 
//    Bias_m_K = Bias - K
//
{ .mii
(p0)  ldfe f40 = [r35],16 
//
//    load A_1
//    poly = A_2 + r*A_3 
//    rsq = r * r  
//    neg_2_mK = exponent of Bias_m_k
//
(p0)  add r47 = r58, r44 ;;  
//    
//    Set Safe = True  if k <= big_expo_pos  
//    Set Safe = False  if k >  big_expo_pos  
//    Load A_3
//    
(p15) cmp.lt p8,p15 = r44,r48 ;;
}
{ .mmf
(p0)  setf.exp f61 = r46 
//    
//    Bias_p + K = Bias + K
//    T = T_1 * T_2
//    
(p0)  setf.exp f36 = r47 
(p0)  fnma.s1 f42 = f41, f38, f42 ;; 
}
{ .mfi
	nop.m 999
//
//    Load W_1,W_2
//    Load big_exp_pos, load big_exp_neg
//
(p0)  fadd.s1 f47 = f43, f1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fma.s1 f52 = f42, f51, f49 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)  fmpy.s1 f48 = f42, f42 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fmpy.s1 f53 = f44, f46 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fma.s1 f54 = f45, f47, f43 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)  fneg f61 =  f61 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fma.s1 f52 = f42, f52, f40 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fadd.s1 f55 = f54, f1 
	nop.i 999
}
{ .mfi
	nop.m 999
//
//    W + Wp1 * poly     
// 
(p0)  mov f34 = f53 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//    A_1 + r * poly 
//    Scale = setf_expl(Bias_p_k) 
//
(p0)  fma.s1 f52 = f48, f52, f42 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//    poly = r + rsq(A_1 + r*poly) 
//    Wp1 = 1 + W
//    neg_2_mK = -neg_2_mK
//
(p0)  fma.s1 f35 = f55, f52, f54
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p0)  fmpy.s1 f35 = f35, f53 
//   
//    Y_hi = T
//    Y_lo = T * (W + Wp1*poly)
//
(p12) br.cond.sptk EXPL_MAIN ;; 
}
//
//    Branch if expl(x)  
//    Continue for expl(x-1)
//
{ .mii
(p0)  cmp.lt.unc  p12, p13 =  10, r44 
	nop.i 999 ;;
//
//    Set p12 if 10 < K, Else p13 
//
(p13) cmp.gt.unc  p13, p14 =  -10, r44 ;; 
}
//
//    K > 10:  Y_lo = Y_lo + neg_2_mK
//    K <=10:  Set p13 if -10 > K, Else set p14 
//
{ .mfi
(p13) cmp.eq  p15, p0 =  r0, r0 
(p14) fadd.s1 f34 = f61, f34 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p12) fadd.s1 f35 = f35, f61 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p13) fadd.s1 f35 = f35, f34 
	nop.i 999
}
{ .mfb
	nop.m 999
//
//    K <= 10 and K < -10, Set Safe = True
//    K <= 10 and K < 10,   Y_lo = Y_hi + Y_lo 
//    K <= 10 and K > =-10, Y_hi = Y_hi + neg_2_mk 
// 
(p13) mov f34 = f61 
(p0)  br.cond.sptk EXPL_MAIN ;; 
}
EXPL_SMALL: 
{ .mmi
      nop.m 999
(p0)  addl r34 = @ltoff(Constants_exp_64_Exponents#),gp  
(p12) addl r35 = @ltoff(Constants_exp_64_P#),gp ;; 
}
.pred.rel "mutex",p12,p13
{ .mmi
(p12) ld8  r35=[r35]      
nop.m 999
(p13) addl r35 = @ltoff(Constants_exp_64_Q#),gp 
};;
{ .mmi
(p13) ld8  r35=[r35]      
(p0) ld8  r34=[r34]      
nop.i 999
};;
{ .mfi
(p0)  add r34 = 0x48,r34  
// 
//    Return
//    K <= 10 and K < 10,   Y_hi = neg_2_mk 
// 
//    /*******************************************************/
//    /*********** Branch EXPL_SMALL  ************************/
//    /*******************************************************/
(p0)  mov f42 = f9 
	nop.i 999 ;;
}
//
//    Flag = 0
//    r4 = rsq * rsq
//
{ .mfi
(p0)  ld8 r49 =[r34],0
	nop.f 999
	nop.i 999 ;;
}
{ .mii
	nop.m 999
	nop.i 999 ;;
//
//    Flag = 1
//
(p0)  cmp.lt.unc  p14, p0 =  r37, r49 ;; 
}
{ .mfi
	nop.m 999
//
//    r = X
//
(p0)  fmpy.s1 f48 = f42, f42 
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//
//    rsq = r * r
//
(p0)  fmpy.s1 f50 = f48, f48 
//
//    Is input very small?
//
(p14) br.cond.spnt EXPL_VERY_SMALL ;; 
}
//
//    Flag_not1: Y_hi = 1.0
//    Flag is 1: r6 = rsq * r4
//
{ .mfi
(p12) ldfe f52 = [r35],16 
(p12) mov f34 = f1 
(p0)  add r53 = 0x1,r0 ;;  
}
{ .mfi
(p13) ldfe f51 = [r35],16 
//
//    Flag_not_1: Y_lo = poly_hi + r4 * poly_lo
//
(p13) mov f34 = f9 
	nop.i 999 ;;
}
{ .mmf
(p12) ldfe f53 = [r35],16 
//
//    For Flag_not_1, Y_hi = X
//    Scale = 1
//    Create 0x000...01
//
(p0)  setf.sig f37 = r53 
(p0)  mov f36 = f1 ;; 
}
{ .mmi
(p13) ldfe f52 = [r35],16 ;; 
(p12) ldfe f54 = [r35],16 
	nop.i 999 ;;
}
{ .mfi
(p13) ldfe f53 = [r35],16 
(p13) fmpy.s1 f58 = f48, f50 
	nop.i 999 ;;
}
//
//    Flag_not1: poly_lo = P_5 + r*P_6
//    Flag_1: poly_lo = Q_6 + r*Q_7
//
{ .mmi
(p13) ldfe f54 = [r35],16 ;; 
(p12) ldfe f55 = [r35],16 
	nop.i 999 ;;
}
{ .mmi
(p12) ldfe f56 = [r35],16 ;; 
(p13) ldfe f55 = [r35],16 
	nop.i 999 ;;
}
{ .mmi
(p12) ldfe f57 = [r35],0 ;; 
(p13) ldfe f56 = [r35],16 
	nop.i 999 ;;
}
{ .mfi
(p13) ldfe f57 = [r35],0 
	nop.f 999
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//    For  Flag_not_1, load p5,p6,p1,p2
//    Else load p5,p6,p1,p2
//
(p12) fma.s1 f60 = f52, f42, f53 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p13) fma.s1 f60 = f51, f42, f52 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p12) fma.s1 f60 = f60, f42, f54 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p12) fma.s1 f59 = f56, f42, f57 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p13) fma.s1 f60 = f42, f60, f53 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p12) fma.s1 f59 = f59, f48, f42 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//    Flag_1: poly_lo = Q_5 + r*(Q_6 + r*Q_7) 
//    Flag_not1: poly_lo = P_4 + r*(P_5 + r*P_6)
//    Flag_not1: poly_hi = (P_1 + r*P_2)
//
(p13) fmpy.s1 f60 = f60, f58 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p12) fma.s1 f60 = f60, f42, f55 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//    Flag_1: poly_lo = r6 *(Q_5 + ....)
//    Flag_not1: poly_hi =  r + rsq *(P_1 + r*P_2)
//
(p12) fma.s1 f35 = f60, f50, f59 
	nop.i 999
}
{ .mfi
	nop.m 999
(p13) fma.s1 f59 = f54, f42, f55 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//    Flag_not1: Y_lo = rsq* poly_hi + poly_lo 
//    Flag_1: poly_lo = rsq* poly_hi + poly_lo 
//
(p13) fma.s1 f59 = f59, f42, f56 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//    Flag_not_1: (P_1 + r*P_2) 
//
(p13) fma.s1 f59 = f59, f42, f57 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//    Flag_not_1: poly_hi = r + rsq * (P_1 + r*P_2) 
//
(p13) fma.s1 f35 = f59, f48, f60 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//    Create 0.000...01
//
(p0)  for f37 = f35, f37 
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//
//    Set lsb of Y_lo to 1
//
(p0)  fmerge.se f35 = f35,f37 
(p0)  br.cond.sptk EXPL_MAIN ;; 
}
EXPL_VERY_SMALL: 
{ .mmi
	nop.m 999
	nop.m 999
(p13) addl r34 = @ltoff(Constants_exp_64_Exponents#),gp 
}
{ .mfi
	nop.m 999
(p12) mov f35 = f9 
	nop.i 999 ;;
}
{ .mfb
(p13) ld8 r34 = [r34] 
(p12) mov f34 = f1 
(p12) br.cond.sptk EXPL_MAIN ;; 
}
{ .mlx
(p13) add  r34 = 8,r34 
(p13) movl r39 = 0x0FFFE ;; 
}
//
//    Load big_exp_neg 
//    Create 1/2's exponent
//
{ .mii
(p13) setf.exp f56 = r39 
(p13) shladd r34 = r32,4,r34 ;;  
	nop.i 999
}
//
//    Negative exponents are stored after positive
//
{ .mfi
(p13) ld8 r45 = [r34],0
//
//    Y_hi = x
//    Scale = 1
//
(p13) fmpy.s1 f35 = f9, f9 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//    Reset Safe if necessary 
//    Create 1/2
//
(p13) mov f34 = f9 
	nop.i 999 ;;
}
{ .mfi
(p13) cmp.lt.unc  p0, p15 =  r37, r45 
(p13) mov f36 = f1 
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//
//    Y_lo = x * x
//
(p13) fmpy.s1 f35 = f35, f56 
//
//    Y_lo = x*x/2 
//
(p13) br.cond.sptk EXPL_MAIN ;; 
}
EXPL_HUGE: 
{ .mfi
	nop.m 999
(p0)  fcmp.gt.unc.s1 p14, p0 =  f9, f0 
	nop.i 999
}
{ .mlx
	nop.m 999
(p0)  movl r39 = 0x15DC0 ;; 
}
{ .mfi
(p14) setf.exp f34 = r39 
(p14) mov f35 = f1 
(p14) cmp.eq  p0, p15 =  r0, r0 ;; 
}
{ .mfb
	nop.m 999
(p14) mov f36 = f34 
//
//    If x > 0, Set Safe = False
//    If x > 0, Y_hi = 2**(24,000)
//    If x > 0, Y_lo = 1.0
//    If x > 0, Scale = 2**(24,000)
//
(p14) br.cond.sptk EXPL_MAIN ;; 
}
{ .mlx
	nop.m 999
(p12) movl r39 = 0xA240 
}
{ .mlx
	nop.m 999
(p12) movl r38 = 0xA1DC ;; 
}
{ .mmb
(p13) cmp.eq  p15, p14 =  r0, r0 
(p12) setf.exp f34 = r39 
	nop.b 999 ;;
}
{ .mlx
(p12) setf.exp f35 = r38 
(p13) movl r39 = 0xFF9C 
}
{ .mfi
	nop.m 999
(p13) fsub.s1 f34 = f0, f1
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p12) mov f36 = f34 
(p12) cmp.eq  p0, p15 =  r0, r0 ;; 
}
{ .mfi
(p13) setf.exp f35 = r39 
(p13) mov f36 = f1 
	nop.i 999 ;;
}
EXPL_MAIN: 
{ .mfi
(p0)  cmp.ne.unc p12, p0 = 0x01, r33
(p0)  fmpy.s1 f101 = f36, f35 
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p0)  fma.s0 f99 = f34, f36, f101 
(p15) br.cond.sptk EXPL_64_RETURN ;;
}
{ .mfi
	nop.m 999
(p0)  fsetc.s3 0x7F,0x01
	nop.i 999
}
{ .mlx
	nop.m 999
(p0)  movl r50 = 0x00000000013FFF ;;
}
//    
//    S0 user supplied status
//    S2 user supplied status + WRE + TD  (Overflows) 
//    S3 user supplied status + RZ + TD   (Underflows) 
//    
//    
//    If (Safe) is true, then
//        Compute result using user supplied status field.
//        No overflow or underflow here, but perhaps inexact.
//        Return
//    Else
//       Determine if overflow or underflow  was raised.
//       Fetch +/- overflow threshold for IEEE single, double,
//       double extended   
//    
{ .mfi
(p0)  setf.exp f60 = r50
(p0)  fma.s3 f102 = f34, f36, f101 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)  fsetc.s3 0x7F,0x40 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//    For Safe, no need to check for over/under. 
//    For expm1, handle errors like exp. 
//
(p0)  fsetc.s2 0x7F,0x42
	nop.i 999;;
}
{ .mfi
	nop.m 999
(p0)  fma.s2 f100 = f34, f36, f101 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fsetc.s2 0x7F,0x40 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p7)  fclass.m.unc   p12, p0 =  f102, 0x00F
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)  fclass.m.unc   p11, p0 =  f102, 0x00F
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p7)  fcmp.ge.unc.s1 p10, p0 =  f100, f60
	nop.i 999
}
{ .mfi
	nop.m 999
//    
//    Create largest double exponent + 1.
//    Create smallest double exponent - 1.
//    
(p0)  fcmp.ge.unc.s1 p8, p0 =  f100, f60
	nop.i 999 ;;
}
//    
//    fcmp:   resultS2 >= + overflow threshold  -> set (a) if true
//    fcmp:   resultS2 <= - overflow threshold  -> set (b) if true
//    fclass: resultS3 is denorm/unorm/0        -> set (d) if true
//    
{ .mib
(p10) mov   GR_Parameter_TAG = 39
	nop.i 999
(p10) br.cond.sptk __libm_error_region ;;
}
{ .mib
(p8)  mov   GR_Parameter_TAG = 12
	nop.i 999
(p8)  br.cond.sptk __libm_error_region ;;
}
//    
//    Report that exp overflowed
//    
{ .mib
(p12) mov   GR_Parameter_TAG = 40
	nop.i 999
(p12) br.cond.sptk __libm_error_region ;;
}
{ .mib
(p11) mov   GR_Parameter_TAG = 13
	nop.i 999
(p11) br.cond.sptk __libm_error_region ;;
}
{ .mib
	nop.m 999
	nop.i 999
//    
//    Report that exp underflowed
//    
(p0)  br.cond.sptk EXPL_64_RETURN ;;
}
EXPL_64_SPECIAL: 
{ .mfi
	nop.m 999
(p0)  fclass.m.unc p6,  p0 =  f8, 0x0c3 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)  fclass.m.unc p13, p8 =  f8, 0x007 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p7)  fclass.m.unc p14, p0 =  f8, 0x007 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)  fclass.m.unc p12, p9 =  f8, 0x021 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fclass.m.unc p11, p0 =  f8, 0x022 
	nop.i 999
}
{ .mfi
	nop.m 999
(p7)  fclass.m.unc p10, p0 =  f8, 0x022 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//    
//    Identify +/- 0, Inf, or -Inf 
//    Generate the right kind of NaN.
//    
(p13) fadd.s0 f99 = f0, f1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p14) mov f99 = f8 
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p6)  fadd.s0 f99 = f8, f1 
//    
//    expl(+/-0) = 1 
//    expm1l(+/-0) = +/-0 
//    No exceptions raised
//    
(p6)  br.cond.sptk EXPL_64_RETURN ;; 
}
{ .mib
	nop.m 999
	nop.i 999
(p14) br.cond.sptk EXPL_64_RETURN ;; 
}
{ .mfi
	nop.m 999
(p11) mov f99 = f0 
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p10) fsub.s1 f99 = f0, f1 
//    
//    expl(-Inf) = 0 
//    expm1l(-Inf) = -1 
//    No exceptions raised.
//    
(p10) br.cond.sptk EXPL_64_RETURN ;; 
}
{ .mfb
	nop.m 999
(p12) fmpy.s1 f99 = f8, f1 
//    
//    expl(+Inf) = Inf 
//    No exceptions raised.
//    
(p0)  br.cond.sptk EXPL_64_RETURN ;; 
}
EXPL_64_UNSUPPORTED: 
{ .mfb
	nop.m 999
(p0)  fmpy.s0 f99 = f8, f0 
(p0)  br.cond.sptk EXPL_64_RETURN ;; 
}
EXPL_64_RETURN: 
{ .mfb
      nop.m 999
(p0)  mov   f8     = f99
(p0)  br.ret.sptk   b0
}
.endp
ASM_SIZE_DIRECTIVE(expl) 

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
