.file "log1pl.s" 

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
// History: 
// 2/02/00  hand-optimized
// 4/04/00  Unwind support added
// 8/15/00  Bundle added after call to __libm_error_support to properly
//          set [the previously overwritten] GR_Parameter_RESULT.
//
// *********************************************************************
//
// *********************************************************************
//
// Function:   Combined logl(x), log1pl(x), and log10l(x) where
//             logl(x)   = ln(x), for double-extended precision x values
//             log1pl(x) = ln(x+1), for double-extended precision x values
//             log10l(x) = log (x), for double-extended precision x values
//                           10
//
// *********************************************************************
//
// Resources Used:
//
//    Floating-Point Registers: f8 (Input and Return Value)
//                              f9,f33-f55,f99 
//
//    General Purpose Registers:
//      r32-r53
//      r54-r57 (Used to pass arguments to error handling routine)
//
//    Predicate Registers:      p6-p15
//
// *********************************************************************
//
// IEEE Special Conditions:
//
//    Denormal  fault raised on denormal inputs
//    Overflow exceptions cannot occur  
//    Underflow exceptions raised when appropriate for log1p 
//    (Error Handling Routine called for underflow)
//    Inexact raised when appropriate by algorithm
//
//    logl(inf) = inf
//    logl(-inf) = QNaN 
//    logl(+/-0) = -inf 
//    logl(SNaN) = QNaN
//    logl(QNaN) = QNaN
//    logl(EM_special Values) = QNaN
//    log1pl(inf) = inf
//    log1pl(-inf) = QNaN 
//    log1pl(+/-0) = +/-0 
//    log1pl(-1) =  -inf 
//    log1pl(SNaN) = QNaN
//    log1pl(QNaN) = QNaN
//    log1pl(EM_special Values) = QNaN
//    log10l(inf) = inf
//    log10l(-inf) = QNaN 
//    log10l(+/-0) = -inf 
//    log10l(SNaN) = QNaN
//    log10l(QNaN) = QNaN
//    log10l(EM_special Values) = QNaN
//
// *********************************************************************
//
// Computation is based on the following kernel.
//
// ker_log_64( in_FR    :  X,
// 	    in_FR    :  E,
// 	    in_FR    :  Em1,
// 	    in_GR    :  Expo_Range,
// 	    out_FR   :  Y_hi,
// 	    out_FR   :  Y_lo,
// 	    out_FR   :  Scale,
// 	    out_PR   :  Safe  )
// 
// Overview
//
// The method consists of three cases.
//
// If	|X+Em1| < 2^(-80)	use case log1pl_small;
// elseif	|X+Em1| < 2^(-7)	use case log_near1;
// else				use case log_regular;
//
// Case log1pl_small:
//
// logl( 1 + (X+Em1) ) can be approximated by (X+Em1).
//
// Case log_near1:
//
//   logl( 1 + (X+Em1) ) can be approximated by a simple polynomial
//   in W = X+Em1. This polynomial resembles the truncated Taylor
//   series W - W^/2 + W^3/3 - ...
// 
// Case log_regular:
//
//   Here we use a table lookup method. The basic idea is that in
//   order to compute logl(Arg) for an argument Arg in [1,2), we 
//   construct a value G such that G*Arg is close to 1 and that
//   logl(1/G) is obtainable easily from a table of values calculated
//   beforehand. Thus
//
//	logl(Arg) = logl(1/G) + logl(G*Arg)
//		 = logl(1/G) + logl(1 + (G*Arg - 1))
//
//   Because |G*Arg - 1| is small, the second term on the right hand
//   side can be approximated by a short polynomial. We elaborate
//   this method in four steps.
//
//   Step 0: Initialization
//
//   We need to calculate logl( E + X ). Obtain N, S_hi, S_lo such that
//
//	E + X = 2^N * ( S_hi + S_lo )	exactly
//
//   where S_hi in [1,2) and S_lo is a correction to S_hi in the sense
//   that |S_lo| <= ulp(S_hi).
//
//   Step 1: Argument Reduction
//
//   Based on S_hi, obtain G_1, G_2, G_3 from a table and calculate
//
//	G := G_1 * G_2 * G_3
//	r := (G * S_hi - 1)  + G * S_lo
//
//   These G_j's have the property that the product is exactly 
//   representable and that |r| < 2^(-12) as a result.
//
//   Step 2: Approximation
//
//
//   logl(1 + r) is approximated by a short polynomial poly(r).
//
//   Step 3: Reconstruction
//
//
//   Finally, logl( E + X ) is given by
//
//   logl( E + X )   =   logl( 2^N * (S_hi + S_lo) )
//                 ~=~  N*logl(2) + logl(1/G) + logl(1 + r)
//                 ~=~  N*logl(2) + logl(1/G) + poly(r).
//
// **** Algorithm ****
//
// Case log1pl_small:
//
// Although logl(1 + (X+Em1)) is basically X+Em1, we would like to 
// preserve the inexactness nature as well as consistent behavior
// under different rounding modes. Note that this case can only be
// taken if E is set to be 1.0. In this case, Em1 is zero, and that
// X can be very tiny and thus the final result can possibly underflow.
// Thus, we compare X against a threshold that is dependent on the
// input Expo_Range. If |X| is smaller than this threshold, we set
// SAFE to be FALSE. 
//
// The result is returned as Y_hi, Y_lo, and in the case of SAFE 
// is FALSE, an additional value Scale is also returned. 
//
//	W    := X + Em1
//      Threshold := Threshold_Table( Expo_Range )
//      Tiny      := Tiny_Table( Expo_Range )
//
//      If ( |W| > Threshold ) then
//         Y_hi  := W
//         Y_lo  := -W*W
//      Else
//         Y_hi  := W
//         Y_lo  := -Tiny
//         Scale := 2^(-100)
//         Safe  := FALSE
//      EndIf
//
//
// One may think that Y_lo should be -W*W/2; however, it does not matter
// as Y_lo will be rounded off completely except for the correct effect in 
// directed rounding. Clearly -W*W is simplier to compute. Moreover,
// because of the difference in exponent value, Y_hi + Y_lo or 
// Y_hi + Scale*Y_lo is always inexact.
//
// Case log_near1:
//
// Here we compute a simple polynomial. To exploit parallelism, we split
// the polynomial into two portions.
// 
// 	W := X + Em1
// 	Wsq := W * W
// 	W4  := Wsq*Wsq
// 	W6  := W4*Wsq
// 	Y_hi := W + Wsq*(P_1 + W*(P_2 + W*(P_3 + W*P_4))
// 	Y_lo := W6*(P_5 + W*(P_6 + W*(P_7 + W*P_8)))
//      set lsb(Y_lo) to be 1
//
// Case log_regular:
//
// We present the algorithm in four steps.
//
//   Step 0. Initialization
//   ----------------------
//
//   Z := X + E
//   N := unbaised exponent of Z
//   S_hi := 2^(-N) * Z
//   S_lo := 2^(-N) * { (max(X,E)-Z) + min(X,E) }
//
//   Note that S_lo is always 0 for the case E = 0.
//
//   Step 1. Argument Reduction
//   --------------------------
//
//   Let
//
//	Z = 2^N * S_hi = 2^N * 1.d_1 d_2 d_3 ... d_63
//
//   We obtain G_1, G_2, G_3 by the following steps.
//
//
//	Define		X_0 := 1.d_1 d_2 ... d_14. This is extracted
//			from S_hi.
//
//	Define		A_1 := 1.d_1 d_2 d_3 d_4. This is X_0 truncated
//			to lsb = 2^(-4).
//
//	Define		index_1 := [ d_1 d_2 d_3 d_4 ].
//
//	Fetch 		Z_1 := (1/A_1) rounded UP in fixed point with
//	fixed point	lsb = 2^(-15).
//			Z_1 looks like z_0.z_1 z_2 ... z_15
//		        Note that the fetching is done using index_1.
//			A_1 is actually not needed in the implementation
//			and is used here only to explain how is the value
//			Z_1 defined.
//
//	Fetch		G_1 := (1/A_1) truncated to 21 sig. bits.
//	floating pt.	Again, fetching is done using index_1. A_1
//			explains how G_1 is defined.
//
//	Calculate	X_1 := X_0 * Z_1 truncated to lsb = 2^(-14)
//			     = 1.0 0 0 0 d_5 ... d_14
//			This is accomplised by integer multiplication.
//			It is proved that X_1 indeed always begin
//			with 1.0000 in fixed point.
//
//
//	Define		A_2 := 1.0 0 0 0 d_5 d_6 d_7 d_8. This is X_1 
//			truncated to lsb = 2^(-8). Similar to A_1,
//			A_2 is not needed in actual implementation. It
//			helps explain how some of the values are defined.
//
//	Define		index_2 := [ d_5 d_6 d_7 d_8 ].
//
//	Fetch 		Z_2 := (1/A_2) rounded UP in fixed point with
//	fixed point	lsb = 2^(-15). Fetch done using index_2.
//			Z_2 looks like z_0.z_1 z_2 ... z_15
//
//	Fetch		G_2 := (1/A_2) truncated to 21 sig. bits.
//	floating pt.
//
//	Calculate	X_2 := X_1 * Z_2 truncated to lsb = 2^(-14)
//			     = 1.0 0 0 0 0 0 0 0 d_9 d_10 ... d_14
//			This is accomplised by integer multiplication.
//			It is proved that X_2 indeed always begin
//			with 1.00000000 in fixed point.
//
//
//	Define		A_3 := 1.0 0 0 0 0 0 0 0 d_9 d_10 d_11 d_12 d_13 1.
//			This is 2^(-14) + X_2 truncated to lsb = 2^(-13).
//
//	Define		index_3 := [ d_9 d_10 d_11 d_12 d_13 ].
//
//	Fetch		G_3 := (1/A_3) truncated to 21 sig. bits.
//	floating pt.	Fetch is done using index_3.
//
//	Compute		G := G_1 * G_2 * G_3. 
//
//	This is done exactly since each of G_j only has 21 sig. bits.
//
//	Compute   
//
//		r := (G*S_hi - 1) + G*S_lo   using 2 FMA operations.
//
//	thus, r approximates G*(S_hi+S_lo) - 1 to within a couple of 
//	rounding errors.
//
//
//  Step 2. Approximation
//  ---------------------
//
//   This step computes an approximation to logl( 1 + r ) where r is the
//   reduced argument just obtained. It is proved that |r| <= 1.9*2^(-13);
//   thus logl(1+r) can be approximated by a short polynomial:
//
//	logl(1+r) ~=~ poly = r + Q1 r^2 + ... + Q4 r^5
//
//
//  Step 3. Reconstruction
//  ----------------------
//
//   This step computes the desired result of logl(X+E):
//
//	logl(X+E)  =   logl( 2^N * (S_hi + S_lo) )
//		  =   N*logl(2) + logl( S_hi + S_lo )
//		  =   N*logl(2) + logl(1/G) +
//		      logl(1 + C*(S_hi+S_lo) - 1 )
//
//   logl(2), logl(1/G_j) are stored as pairs of (single,double) numbers:
//   log2_hi, log2_lo, log1byGj_hi, log1byGj_lo. The high parts are
//   single-precision numbers and the low parts are double precision
//   numbers. These have the property that
//
//	N*log2_hi + SUM ( log1byGj_hi )
//
//   is computable exactly in double-extended precision (64 sig. bits).
//   Finally
//
//	Y_hi := N*log2_hi + SUM ( log1byGj_hi )
//	Y_lo := poly_hi + [ poly_lo + 
//	        ( SUM ( log1byGj_lo ) + N*log2_lo ) ]
//      set lsb(Y_lo) to be 1
//

#include "libm_support.h"

#ifdef _LIBC
.rodata
#else
.data
#endif

// P_7, P_6, P_5, P_4, P_3, P_2, and P_1 

.align 64
Constants_P:
ASM_TYPE_DIRECTIVE(Constants_P,@object)
data4  0xEFD62B15,0xE3936754,0x00003FFB,0x00000000
data4  0xA5E56381,0x8003B271,0x0000BFFC,0x00000000
data4  0x73282DB0,0x9249248C,0x00003FFC,0x00000000
data4  0x47305052,0xAAAAAA9F,0x0000BFFC,0x00000000
data4  0xCCD17FC9,0xCCCCCCCC,0x00003FFC,0x00000000
data4  0x00067ED5,0x80000000,0x0000BFFD,0x00000000
data4  0xAAAAAAAA,0xAAAAAAAA,0x00003FFD,0x00000000
data4  0xFFFFFFFE,0xFFFFFFFF,0x0000BFFD,0x00000000
ASM_SIZE_DIRECTIVE(Constants_P)
 
// log2_hi, log2_lo, Q_4, Q_3, Q_2, and Q_1 

.align 64
Constants_Q:
ASM_TYPE_DIRECTIVE(Constants_Q,@object)
data4  0x00000000,0xB1721800,0x00003FFE,0x00000000 
data4  0x4361C4C6,0x82E30865,0x0000BFE2,0x00000000
data4  0x328833CB,0xCCCCCAF2,0x00003FFC,0x00000000
data4  0xA9D4BAFB,0x80000077,0x0000BFFD,0x00000000
data4  0xAAABE3D2,0xAAAAAAAA,0x00003FFD,0x00000000
data4  0xFFFFDAB7,0xFFFFFFFF,0x0000BFFD,0x00000000
ASM_SIZE_DIRECTIVE(Constants_Q)
 
// Z1 - 16 bit fixed, G1 and H1 - IEEE single 
 
.align 64
Constants_Z_G_H_h1:
ASM_TYPE_DIRECTIVE(Constants_Z_G_H_h1,@object)
data4  0x00008000,0x3F800000,0x00000000,0x00000000,0x00000000,0x00000000
data4  0x00007879,0x3F70F0F0,0x3D785196,0x00000000,0x617D741C,0x3DA163A6
data4  0x000071C8,0x3F638E38,0x3DF13843,0x00000000,0xCBD3D5BB,0x3E2C55E6
data4  0x00006BCB,0x3F579430,0x3E2FF9A0,0x00000000,0xD86EA5E7,0xBE3EB0BF
data4  0x00006667,0x3F4CCCC8,0x3E647FD6,0x00000000,0x86B12760,0x3E2E6A8C
data4  0x00006187,0x3F430C30,0x3E8B3AE7,0x00000000,0x5C0739BA,0x3E47574C
data4  0x00005D18,0x3F3A2E88,0x3EA30C68,0x00000000,0x13E8AF2F,0x3E20E30F
data4  0x0000590C,0x3F321640,0x3EB9CEC8,0x00000000,0xF2C630BD,0xBE42885B
data4  0x00005556,0x3F2AAAA8,0x3ECF9927,0x00000000,0x97E577C6,0x3E497F34
data4  0x000051EC,0x3F23D708,0x3EE47FC5,0x00000000,0xA6B0A5AB,0x3E3E6A6E
data4  0x00004EC5,0x3F1D89D8,0x3EF8947D,0x00000000,0xD328D9BE,0xBDF43E3C
data4  0x00004BDB,0x3F17B420,0x3F05F3A1,0x00000000,0x0ADB090A,0x3E4094C3
data4  0x00004925,0x3F124920,0x3F0F4303,0x00000000,0xFC1FE510,0xBE28FBB2
data4  0x0000469F,0x3F0D3DC8,0x3F183EBF,0x00000000,0x10FDE3FA,0x3E3A7895
data4  0x00004445,0x3F088888,0x3F20EC80,0x00000000,0x7CC8C98F,0x3E508CE5
data4  0x00004211,0x3F042108,0x3F29516A,0x00000000,0xA223106C,0xBE534874
ASM_SIZE_DIRECTIVE(Constants_Z_G_H_h1)
 
// Z2 - 16 bit fixed, G2 and H2 - IEEE single 

.align 64 
Constants_Z_G_H_h2:
ASM_TYPE_DIRECTIVE(Constants_Z_G_H_h2,@object)
data4  0x00008000,0x3F800000,0x00000000,0x00000000,0x00000000,0x00000000
data4  0x00007F81,0x3F7F00F8,0x3B7F875D,0x00000000,0x22C42273,0x3DB5A116
data4  0x00007F02,0x3F7E03F8,0x3BFF015B,0x00000000,0x21F86ED3,0x3DE620CF
data4  0x00007E85,0x3F7D08E0,0x3C3EE393,0x00000000,0x484F34ED,0xBDAFA07E
data4  0x00007E08,0x3F7C0FC0,0x3C7E0586,0x00000000,0x3860BCF6,0xBDFE07F0
data4  0x00007D8D,0x3F7B1880,0x3C9E75D2,0x00000000,0xA78093D6,0x3DEA370F
data4  0x00007D12,0x3F7A2328,0x3CBDC97A,0x00000000,0x72A753D0,0x3DFF5791
data4  0x00007C98,0x3F792FB0,0x3CDCFE47,0x00000000,0xA7EF896B,0x3DFEBE6C
data4  0x00007C20,0x3F783E08,0x3CFC15D0,0x00000000,0x409ECB43,0x3E0CF156
data4  0x00007BA8,0x3F774E38,0x3D0D874D,0x00000000,0xFFEF71DF,0xBE0B6F97
data4  0x00007B31,0x3F766038,0x3D1CF49B,0x00000000,0x5D59EEE8,0xBE080483
data4  0x00007ABB,0x3F757400,0x3D2C531D,0x00000000,0xA9192A74,0x3E1F91E9
data4  0x00007A45,0x3F748988,0x3D3BA322,0x00000000,0xBF72A8CD,0xBE139A06
data4  0x000079D1,0x3F73A0D0,0x3D4AE46F,0x00000000,0xF8FBA6CF,0x3E1D9202
data4  0x0000795D,0x3F72B9D0,0x3D5A1756,0x00000000,0xBA796223,0xBE1DCCC4
data4  0x000078EB,0x3F71D488,0x3D693B9D,0x00000000,0xB6B7C239,0xBE049391
ASM_SIZE_DIRECTIVE(Constants_Z_G_H_h2)
 
// G3 and H3 - IEEE single and h3 -IEEE double 

.align 64 
Constants_Z_G_H_h3:
ASM_TYPE_DIRECTIVE(Constants_Z_G_H_h3,@object)
data4  0x3F7FFC00,0x38800100,0x562224CD,0x3D355595
data4  0x3F7FF400,0x39400480,0x06136FF6,0x3D8200A2
data4  0x3F7FEC00,0x39A00640,0xE8DE9AF0,0x3DA4D68D
data4  0x3F7FE400,0x39E00C41,0xB10238DC,0xBD8B4291
data4  0x3F7FDC00,0x3A100A21,0x3B1952CA,0xBD89CCB8
data4  0x3F7FD400,0x3A300F22,0x1DC46826,0xBDB10707
data4  0x3F7FCC08,0x3A4FF51C,0xF43307DB,0x3DB6FCB9
data4  0x3F7FC408,0x3A6FFC1D,0x62DC7872,0xBD9B7C47
data4  0x3F7FBC10,0x3A87F20B,0x3F89154A,0xBDC3725E
data4  0x3F7FB410,0x3A97F68B,0x62B9D392,0xBD93519D
data4  0x3F7FAC18,0x3AA7EB86,0x0F21BD9D,0x3DC18441
data4  0x3F7FA420,0x3AB7E101,0x2245E0A6,0xBDA64B95
data4  0x3F7F9C20,0x3AC7E701,0xAABB34B8,0x3DB4B0EC
data4  0x3F7F9428,0x3AD7DD7B,0x6DC40A7E,0x3D992337
data4  0x3F7F8C30,0x3AE7D474,0x4F2083D3,0x3DC6E17B
data4  0x3F7F8438,0x3AF7CBED,0x811D4394,0x3DAE314B
data4  0x3F7F7C40,0x3B03E1F3,0xB08F2DB1,0xBDD46F21
data4  0x3F7F7448,0x3B0BDE2F,0x6D34522B,0xBDDC30A4
data4  0x3F7F6C50,0x3B13DAAA,0xB1F473DB,0x3DCB0070
data4  0x3F7F6458,0x3B1BD766,0x6AD282FD,0xBDD65DDC
data4  0x3F7F5C68,0x3B23CC5C,0xF153761A,0xBDCDAB83
data4  0x3F7F5470,0x3B2BC997,0x341D0F8F,0xBDDADA40
data4  0x3F7F4C78,0x3B33C711,0xEBC394E8,0x3DCD1BD7
data4  0x3F7F4488,0x3B3BBCC6,0x52E3E695,0xBDC3532B
data4  0x3F7F3C90,0x3B43BAC0,0xE846B3DE,0xBDA3961E
data4  0x3F7F34A0,0x3B4BB0F4,0x785778D4,0xBDDADF06
data4  0x3F7F2CA8,0x3B53AF6D,0xE55CE212,0x3DCC3ED1
data4  0x3F7F24B8,0x3B5BA620,0x9E382C15,0xBDBA3103
data4  0x3F7F1CC8,0x3B639D12,0x5C5AF197,0x3D635A0B
data4  0x3F7F14D8,0x3B6B9444,0x71D34EFC,0xBDDCCB19
data4  0x3F7F0CE0,0x3B7393BC,0x52CD7ADA,0x3DC74502
data4  0x3F7F04F0,0x3B7B8B6D,0x7D7F2A42,0xBDB68F17
ASM_SIZE_DIRECTIVE(Constants_Z_G_H_h3)
 
// 
//  Exponent Thresholds and Tiny Thresholds
//  for 8, 11, 15, and 17 bit exponents
// 
//  Expo_Range             Value
// 
//  0 (8  bits)            2^(-126)
//  1 (11 bits)            2^(-1022)
//  2 (15 bits)            2^(-16382)
//  3 (17 bits)            2^(-16382)
// 
//  Tiny_Table
//  ----------
//  Expo_Range             Value
// 
//  0 (8  bits)            2^(-16382)
//  1 (11 bits)            2^(-16382)
//  2 (15 bits)            2^(-16382)
//  3 (17 bits)            2^(-16382)
// 

.align 64 
Constants_Threshold:
ASM_TYPE_DIRECTIVE(Constants_Threshold,@object)
data4  0x00000000,0x80000000,0x00003F81,0x00000000
data4  0x00000000,0x80000000,0x00000001,0x00000000
data4  0x00000000,0x80000000,0x00003C01,0x00000000
data4  0x00000000,0x80000000,0x00000001,0x00000000
data4  0x00000000,0x80000000,0x00000001,0x00000000
data4  0x00000000,0x80000000,0x00000001,0x00000000
data4  0x00000000,0x80000000,0x00000001,0x00000000
data4  0x00000000,0x80000000,0x00000001,0x00000000
ASM_SIZE_DIRECTIVE(Constants_Threshold)

.align 64
Constants_1_by_LN10:
ASM_TYPE_DIRECTIVE(Constants_1_by_LN10,@object)
data4  0x37287195,0xDE5BD8A9,0x00003FFD,0x00000000
data4  0xACCF70C8,0xD56EAABE,0x00003FBB,0x00000000
ASM_SIZE_DIRECTIVE(Constants_1_by_LN10)

FR_Input_X = f8 
FR_Neg_One = f9
FR_E       = f33
FR_Em1     = f34
FR_Y_hi    = f34  
// Shared with Em1
FR_Y_lo    = f35
FR_Scale   = f36
FR_X_Prime = f37 
FR_Z       = f38 
FR_S_hi    = f38  
// Shared with Z  
FR_W       = f39
FR_G       = f40
FR_wsq     = f40 
// Shared with G 
FR_H       = f41
FR_w4      = f41
// Shared with H  
FR_h       = f42
FR_w6      = f42  
// Shared with h     
FR_G_tmp   = f43
FR_poly_lo = f43
// Shared with G_tmp 
FR_P8      = f43  
// Shared with G_tmp 
FR_H_tmp   = f44
FR_poly_hi = f44
  // Shared with H_tmp
FR_P7      = f44  
// Shared with H_tmp
FR_h_tmp   = f45 
FR_rsq     = f45  
// Shared with h_tmp
FR_P6      = f45
// Shared with h_tmp
FR_abs_W   = f46
FR_r       = f46  
// Shared with abs_W  
FR_AA      = f47 
FR_log2_hi = f47  
// Shared with AA  
FR_BB          = f48
FR_log2_lo     = f48  
// Shared with BB  
FR_S_lo        = f49 
FR_two_negN    = f50  
FR_float_N     = f51 
FR_Q4          = f52 
FR_dummy       = f52  
// Shared with Q4
FR_P4          = f52  
// Shared with Q4
FR_Threshold    = f52
// Shared with Q4
FR_Q3          = f53  
FR_P3          = f53  
// Shared with Q3
FR_Tiny        = f53  
// Shared with Q3
FR_Q2          = f54 
FR_P2          = f54  
// Shared with Q2
FR_1LN10_hi     = f54 
// Shared with Q2
FR_Q1           = f55 
FR_P1           = f55 
// Shared with Q1 
FR_1LN10_lo     = f55 
// Shared with Q1 
FR_P5           = f98 
FR_SCALE        = f98 
FR_Output_X_tmp = f99 

GR_Expo_Range   = r32
GR_Table_Base   = r34
GR_Table_Base1  = r35
GR_Table_ptr    = r36 
GR_Index2       = r37 
GR_signif       = r38 
GR_X_0          = r39 
GR_X_1          = r40 
GR_X_2          = r41 
GR_Z_1          = r42 
GR_Z_2          = r43 
GR_N            = r44 
GR_Bias         = r45 
GR_M            = r46 
GR_ScaleN       = r47  
GR_Index3       = r48 
GR_Perturb      = r49 
GR_Table_Scale  = r50 

//
// Added for unwind support
//

GR_SAVE_PFS         = r51
GR_SAVE_B0          = r52
GR_SAVE_GP          = r53
GR_Parameter_X      = r54
GR_Parameter_Y      = r55
GR_Parameter_RESULT = r56
GR_Parameter_TAG    = r57

FR_X                = f8
FR_Y                = f0
FR_RESULT           = f99

.section .text
.proc logl#
.global logl#
.align 64 
logl:
#ifdef _LIBC
.global __ieee754_logl
__ieee754_logl:
#endif 
{ .mfi
alloc r32 = ar.pfs,0,22,4,0
(p0)  fnorm.s1 FR_X_Prime = FR_Input_X 
(p0)  cmp.eq.unc  p7, p0 = r0, r0 
}
{ .mfi
(p0)  cmp.ne.unc  p14, p0 = r0, r0 
(p0)  fclass.m.unc p6, p0 =  FR_Input_X, 0x1E3 
(p0)  cmp.ne.unc  p15, p0 = r0, r0 ;; 
}
{ .mfi
 nop.m 0
(p0)  fclass.nm.unc p10, p0 =  FR_Input_X, 0x1FF 
 nop.i 0
}
{ .mfi
nop.m 999
(p0)  fcmp.eq.unc.s1 p8, p0 =  FR_Input_X, f0 
 nop.i 0
}
{ .mfi
	nop.m 999
(p0)  fcmp.lt.unc.s1 p13, p0 =  FR_Input_X, f0 
 nop.i 0
}
{ .mfi
	nop.m 999
(p0)  fcmp.eq.unc.s1 p9, p0 =  FR_Input_X, f1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fsub.s1 FR_Em1 = f0,f1 
	nop.i 999
}
{ .mfb
	nop.m 999
(p0)  fadd FR_E = f0,f0 
//     
//    Create E = 0 and Em1 = -1 
//    Check for X == 1, meaning logl(1)
//    Check for X < 0, meaning logl(negative)
//    Check for X == 0, meaning logl(0)
//    Identify NatVals, NaNs, Infs. 
//    Identify EM unsupporteds. 
//    Identify Negative values - us S1 so as
//    not to raise denormal operand exception 
//    Set p15 to false for log
//    Set p14 to false for log
//    Set p7 true for log and log1p
//    
(p0)  br.cond.sptk L(LOGL_BEGIN) ;; 
}

.endp logl
ASM_SIZE_DIRECTIVE(logl)

.section .text
.proc log10l#
.global log10l#
.align 64 
log10l:
#ifdef _LIBC
.global __ieee754_log10l
__ieee754_log10l:
#endif
{ .mfi
alloc r32 = ar.pfs,0,22,4,0
(p0)  fadd FR_E = f0,f0 
      nop.i 0
}
{ .mfi
      nop.m 0
(p0)  fsub.s1 FR_Em1 = f0,f1 
      nop.i 0
}
{ .mfi
(p0)  cmp.ne.unc  p15, p0 = r0, r0 
(p0)  fcmp.eq.unc.s1 p9, p0 =  FR_Input_X, f1 
      nop.i 0
}
{ .mfi
(p0)  cmp.eq.unc  p14, p0 = r0, r0 
(p0)  fcmp.lt.unc.s1 p13, p0 =  FR_Input_X, f0 
(p0)  cmp.ne.unc  p7, p0 = r0, r0 ;; 
}
{ .mfi
	nop.m 999
(p0)  fcmp.eq.unc.s1 p8, p0 =  FR_Input_X, f0 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)  fclass.nm.unc p10, p0 =  FR_Input_X, 0x1FF 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fclass.m.unc p6, p0 =  FR_Input_X, 0x1E3 
	nop.i 999
}
{ .mfb
	nop.m 999
(p0)  fnorm.s1 FR_X_Prime = FR_Input_X 
//     
//    Create E = 0 and Em1 = -1 
//    Check for X == 1, meaning logl(1)
//    Check for X < 0, meaning logl(negative)
//    Check for X == 0, meaning logl(0)
//    Identify NatVals, NaNs, Infs. 
//    Identify EM unsupporteds. 
//    Identify Negative values - us S1 so as
//    Identify Negative values - us S1 so as
//    not to raise denormal operand exception 
//    Set p15 to false for log10
//    Set p14 to true for log10
//    Set p7 to false for log10
//    
(p0)  br.cond.sptk L(LOGL_BEGIN) ;; 
}

.endp log10l
ASM_SIZE_DIRECTIVE(log10l)

.section .text
.proc log1pl#
.global log1pl#
.align 64 
log1pl:
#ifdef _LIBC
.global __log1pl
__log1pl:
#endif
{ .mfi
alloc r32 = ar.pfs,0,22,4,0
(p0)  fsub.s1 FR_Neg_One = f0,f1 
(p0)  cmp.eq.unc  p7, p0 = r0, r0 
}
{ .mfi
(p0)  cmp.ne.unc  p14, p0 = r0, r0 
(p0)  fnorm.s1 FR_X_Prime = FR_Input_X 
(p0)  cmp.eq.unc  p15, p0 = r0, r0 ;; 
}
{ .mfi
      nop.m 0
(p0)  fclass.m.unc p6, p0 =  FR_Input_X, 0x1E3 
      nop.i 0
}
{ .mfi
      nop.m 999
(p0)  fclass.nm.unc p10, p0 =  FR_Input_X, 0x1FF 
      nop.i 0
}
{ .mfi
      nop.m 999
(p0)  fcmp.eq.unc.s1 p9, p0 =  FR_Input_X, f0 
      nop.i 0 
}
{ .mfi
      nop.m 999
(p0)  fadd FR_Em1 = f0,f0 
      nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fadd FR_E = f0,f1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fcmp.eq.unc.s1 p8, p0 =  FR_Input_X, FR_Neg_One 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)  fcmp.lt.unc.s1 p13, p0 =  FR_Input_X, FR_Neg_One 
	nop.i 999
}
L(LOGL_BEGIN): 
{ .mfi
	nop.m 999
(p0)  fadd.s1 FR_Z = FR_X_Prime, FR_E 
	nop.i 999
}
{ .mlx
	nop.m 999
(p0)  movl GR_Table_Scale = 0x0000000000000018 ;; 
}
{ .mmi
	nop.m 999
	nop.m 999
//     
//    Create E = 1 and Em1 = 0 
//    Check for X == 0, meaning logl(1+0)
//    Check for X < -1, meaning logl(negative)
//    Check for X == -1, meaning logl(0)
//    Normalize x 
//    Identify NatVals, NaNs, Infs. 
//    Identify EM unsupporteds. 
//    Identify Negative values - us S1 so as
//    not to raise denormal operand exception 
//    Set p15 to true for log1p
//    Set p14 to false for log1p
//    Set p7 true for log and log1p
//    
(p0)  addl GR_Table_Base = @ltoff(Constants_Z_G_H_h1#),gp
}
{ .mfi
      nop.m 999
(p0)  fmax.s1 FR_AA = FR_X_Prime, FR_E 
      nop.i 999 ;;
}
{ .mfi
      ld8    GR_Table_Base = [GR_Table_Base]
(p0)  fmin.s1 FR_BB = FR_X_Prime, FR_E 
      nop.i 999
}
{ .mfb
      nop.m 999
(p0)  fadd.s1 FR_W = FR_X_Prime, FR_Em1 
//     
//    Begin load of constants base
//    FR_Z = Z = |x| + E 
//    FR_W = W = |x| + Em1
//    AA = fmax(|x|,E)
//    BB = fmin(|x|,E)
//
(p6)  br.cond.spnt L(LOGL_64_special) ;; 
}
{ .mib
	nop.m 999
	nop.i 999
(p10) br.cond.spnt L(LOGL_64_unsupported) ;; 
}
{ .mib
	nop.m 999
	nop.i 999
(p13) br.cond.spnt L(LOGL_64_negative) ;; 
}
{ .mib
(p0)  getf.sig GR_signif = FR_Z 
	nop.i 999
(p9)  br.cond.spnt L(LOGL_64_one) ;; 
}
{ .mib
	nop.m 999
	nop.i 999
(p8)  br.cond.spnt L(LOGL_64_zero) ;; 
}
{ .mfi
(p0)  getf.exp GR_N =  FR_Z 
//   
//    Raise possible denormal operand exception 
//    Create Bias
// 
//    This function computes ln( x + e ) 
//    Input  FR 1: FR_X   = FR_Input_X          
//    Input  FR 2: FR_E   = FR_E
//    Input  FR 3: FR_Em1 = FR_Em1 
//    Input  GR 1: GR_Expo_Range = GR_Expo_Range = 1
//    Output FR 4: FR_Y_hi  
//    Output FR 5: FR_Y_lo  
//    Output FR 6: FR_Scale  
//    Output PR 7: PR_Safe  
//
(p0)  fsub.s1 FR_S_lo = FR_AA, FR_Z 
//
//    signif = getf.sig(Z)
//    abs_W = fabs(w)
//
(p0)  extr.u GR_Table_ptr = GR_signif, 59, 4 ;; 
}
{ .mfi
	nop.m 999
(p0)  fmerge.se FR_S_hi =  f1,FR_Z 
(p0)  extr.u GR_X_0 = GR_signif, 49, 15  
}
{ .mmi
       nop.m 999
       nop.m 999
(p0)  addl GR_Table_Base1 = @ltoff(Constants_Z_G_H_h2#),gp ;; 
}
{ .mlx
      ld8    GR_Table_Base1 = [GR_Table_Base1]
(p0)  movl GR_Bias = 0x000000000000FFFF ;; 
}
{ .mfi
	nop.m 999
(p0)  fabs FR_abs_W =  FR_W 
(p0)  pmpyshr2.u GR_Table_ptr = GR_Table_ptr,GR_Table_Scale,0 
}
{ .mfi
	nop.m 999
//    
//    Branch out for special input values 
//    
(p0)  fcmp.lt.unc.s0 p8, p0 =  FR_Input_X, f0 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//    X_0 = extr.u(signif,49,15)
//    Index1 = extr.u(signif,59,4)
//
(p0)  fadd.s1 FR_S_lo = FR_S_lo, FR_BB 
	nop.i 999 ;;
}
{ .mii
	nop.m 999
	nop.i 999 ;;
//
//    Offset_to_Z1 = 24 * Index1
//    For performance, don't use result
//    for 3 or 4 cycles.
//
(p0)  add GR_Table_ptr = GR_Table_ptr, GR_Table_Base ;; 
}
//
//    Add Base to Offset for Z1
//    Create Bias
{ .mmi
(p0)  ld4 GR_Z_1 = [GR_Table_ptr],4 ;; 
(p0)  ldfs  FR_G = [GR_Table_ptr],4 
	nop.i 999 ;;
}
{ .mmi
(p0)  ldfs  FR_H = [GR_Table_ptr],8 ;; 
(p0)  ldfd  FR_h = [GR_Table_ptr],0 
(p0)  pmpyshr2.u GR_X_1 = GR_X_0,GR_Z_1,15 
}
//
//    Load Z_1 
//    Get Base of Table2 
//
{ .mfi
(p0)  getf.exp GR_M = FR_abs_W 
	nop.f 999
	nop.i 999 ;;
}
{ .mii
	nop.m 999
	nop.i 999 ;;
//
//    M = getf.exp(abs_W)
//    S_lo = AA - Z
//    X_1 = pmpyshr2(X_0,Z_1,15)
//
(p0)  sub GR_M = GR_M, GR_Bias ;; 
}
//     
//    M = M - Bias
//    Load G1
//    N = getf.exp(Z)
//
{ .mii
(p0)  cmp.gt.unc  p11, p0 =  -80, GR_M 
(p0)  cmp.gt.unc  p12, p0 =  -7, GR_M ;; 
(p0)  extr.u GR_Index2 = GR_X_1, 6, 4 ;; 
}
{ .mib
	nop.m 999
//
//    if -80 > M, set p11
//    Index2 = extr.u(X_1,6,4)
//    if -7  > M, set p12
//    Load H1
//
(p0)  pmpyshr2.u GR_Index2 = GR_Index2,GR_Table_Scale,0 
(p11) br.cond.spnt L(log1pl_small) ;; 
}
{ .mib
      nop.m 999
	nop.i 999
(p12) br.cond.spnt L(log1pl_near) ;; 
}
{ .mii
(p0)  sub GR_N = GR_N, GR_Bias 
//
//    poly_lo = r * poly_lo 
//
(p0)  add GR_Perturb = 0x1, r0 ;; 
(p0)  sub GR_ScaleN = GR_Bias, GR_N  
}
{ .mii
(p0)  setf.sig FR_float_N = GR_N 
	nop.i 999 ;;
//
//    Prepare Index2 - pmpyshr2.u(X_1,Z_2,15)
//    Load h1
//    S_lo = S_lo + BB 
//    Branch for -80 > M
//   
(p0)  add GR_Index2 = GR_Index2, GR_Table_Base1
}
{ .mmi
(p0)  setf.exp FR_two_negN = GR_ScaleN 
      nop.m 999
(p0)  addl GR_Table_Base = @ltoff(Constants_Z_G_H_h3#),gp ;; 
}
//
//    Index2 points to Z2
//    Branch for -7 > M
//
{ .mmb
(p0)  ld4 GR_Z_2 = [GR_Index2],4 
(p0)  ld8 GR_Table_Base = [GR_Table_Base] 
	nop.b 999 ;;
}
(p0)  nop.i 999
//
//    Load Z_2
//    N = N - Bias
//    Tablebase points to Table3
//
{ .mmi
(p0)  ldfs  FR_G_tmp = [GR_Index2],4 ;; 
//
//    Load G_2
//    pmpyshr2  X_2= (X_1,Z_2,15)
//    float_N = setf.sig(N)
//    ScaleN = Bias - N
//
(p0)  ldfs  FR_H_tmp = [GR_Index2],8 
	nop.i 999 ;;
}
//
//    Load H_2
//    two_negN = setf.exp(scaleN)
//    G = G_1 * G_2
//
{ .mfi
(p0)  ldfd  FR_h_tmp = [GR_Index2],0 
	nop.f 999
(p0)  pmpyshr2.u GR_X_2 = GR_X_1,GR_Z_2,15 ;; 
}
{ .mii
	nop.m 999
(p0)  extr.u GR_Index3 = GR_X_2, 1, 5 ;; 
//
//    Load h_2
//    H = H_1 + H_2 
//    h = h_1 + h_2 
//    Index3 = extr.u(X_2,1,5)
//
(p0)  shladd GR_Index3 = GR_Index3,4,GR_Table_Base 
}
{ .mmi
	nop.m 999
	nop.m 999
//
//    float_N = fcvt.xf(float_N)
//    load G3
//
(p0)  addl GR_Table_Base = @ltoff(Constants_Q#),gp ;; 
}
{ .mmi
      nop.m 999
      ld8    GR_Table_Base = [GR_Table_Base]
      nop.i 999
};;

{ .mfi
(p0)  ldfe FR_log2_hi = [GR_Table_Base],16 
(p0)  fmpy.s1 FR_S_lo = FR_S_lo, FR_two_negN 
	nop.i 999 ;;
}
{ .mmf
	nop.m 999
//
//    G = G3 * G
//    Load h3
//    Load log2_hi
//    H = H + H3
//
(p0)  ldfe FR_log2_lo = [GR_Table_Base],16 
(p0)  fmpy.s1 FR_G = FR_G, FR_G_tmp ;; 
}
{ .mmf
(p0)  ldfs  FR_G_tmp = [GR_Index3],4 
//
//    h = h + h3
//    r = G * S_hi + 1 
//    Load log2_lo
//
(p0)  ldfe FR_Q4 = [GR_Table_Base],16 
(p0)  fadd.s1 FR_h = FR_h, FR_h_tmp ;; 
}
{ .mfi
(p0)  ldfe FR_Q3 = [GR_Table_Base],16 
(p0)  fadd.s1 FR_H = FR_H, FR_H_tmp 
	nop.i 999 ;;
}
{ .mmf
(p0)  ldfs  FR_H_tmp = [GR_Index3],4 
(p0)  ldfe FR_Q2 = [GR_Table_Base],16 
//
//    Comput Index for Table3
//    S_lo = S_lo * two_negN
//
(p0)  fcvt.xf FR_float_N = FR_float_N ;; 
}
//
//    If S_lo == 0, set p8 false
//    Load H3
//    Load ptr to table of polynomial coeff.
//
{ .mmf
(p0)  ldfd  FR_h_tmp = [GR_Index3],0 
(p0)  ldfe FR_Q1 = [GR_Table_Base],0 
(p0)  fcmp.eq.unc.s1 p0, p8 =  FR_S_lo, f0 ;; 
}
{ .mfi
	nop.m 999
(p0)  fmpy.s1 FR_G = FR_G, FR_G_tmp 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fadd.s1 FR_H = FR_H, FR_H_tmp 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fms.s1 FR_r = FR_G, FR_S_hi, f1 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)  fadd.s1 FR_h = FR_h, FR_h_tmp 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fma.s1 FR_Y_hi = FR_float_N, FR_log2_hi, FR_H 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//    Load Q4 
//    Load Q3 
//    Load Q2 
//    Load Q1 
//
(p8) fma.s1 FR_r = FR_G, FR_S_lo, FR_r 
	nop.i 999
}
{ .mfi
	nop.m 999
//
//    poly_lo = r * Q4 + Q3
//    rsq = r* r
//
(p0)  fma.s1 FR_h = FR_float_N, FR_log2_lo, FR_h 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//    If (S_lo!=0) r = s_lo * G + r
//
(p0)  fma.s1 FR_poly_lo = FR_r, FR_Q4, FR_Q3 
	nop.i 999
}
//
//    Create a 0x00000....01
//    poly_lo = poly_lo * rsq + h
//
{ .mfi
(p0)  setf.sig FR_dummy = GR_Perturb 
(p0)  fmpy.s1 FR_rsq = FR_r, FR_r 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//    h = N * log2_lo + h 
//    Y_hi = n * log2_hi + H 
//
(p0)  fma.s1 FR_poly_lo = FR_poly_lo, FR_r, FR_Q2 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)  fma.s1 FR_poly_hi = FR_Q1, FR_rsq, FR_r 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//    poly_lo = r * poly_o + Q2 
//    poly_hi = Q1 * rsq + r 
//
(p0)  fmpy.s1 FR_poly_lo = FR_poly_lo, FR_r 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fma.s1 FR_poly_lo = FR_poly_lo, FR_rsq, FR_h 
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p0)  fadd.s1 FR_Y_lo = FR_poly_hi, FR_poly_lo 
//
//    Create the FR for a binary "or"
//    Y_lo = poly_hi + poly_lo
//
// (p0)  for FR_dummy = FR_Y_lo,FR_dummy ;;
//
//    Turn the lsb of Y_lo ON
//
// (p0)  fmerge.se FR_Y_lo =  FR_Y_lo,FR_dummy ;;
//
//    Merge the new lsb into Y_lo, for alone doesn't
//
(p0)  br.cond.sptk LOGL_main ;; 
}
L(log1pl_near): 
{ .mmi
	nop.m 999
	nop.m 999
//    /*******************************************************/
//    /*********** Branch log1pl_near  ************************/
//    /*******************************************************/
(p0)  addl GR_Table_Base = @ltoff(Constants_P#),gp ;; 
}
{ .mmi
      nop.m 999
      ld8    GR_Table_Base = [GR_Table_Base]
      nop.i 999
};;
//
//    Load base address of poly. coeff.
//
{ .mmb
(p0)  add GR_Table_ptr = 0x40,GR_Table_Base  
//
//    Address tables with separate pointers 
//
(p0)  ldfe FR_P8 = [GR_Table_Base],16 
	nop.b 999 ;;
}
{ .mmb
(p0)  ldfe FR_P4 = [GR_Table_ptr],16 
//
//    Load P4
//    Load P8
//
(p0)  ldfe FR_P7 = [GR_Table_Base],16 
	nop.b 999 ;;
}
{ .mmf
(p0)  ldfe FR_P3 = [GR_Table_ptr],16 
//
//    Load P3
//    Load P7
//
(p0)  ldfe FR_P6 = [GR_Table_Base],16 
(p0)  fmpy.s1 FR_wsq = FR_W, FR_W ;; 
}
{ .mfi
(p0)  ldfe FR_P2 = [GR_Table_ptr],16 
	nop.f 999
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fma.s1 FR_Y_hi = FR_W, FR_P4, FR_P3 
	nop.i 999
}
//
//    Load P2
//    Load P6
//    Wsq = w * w
//    Y_hi = p4 * w + p3
//
{ .mfi
(p0)  ldfe FR_P5 = [GR_Table_Base],16 
(p0)  fma.s1 FR_Y_lo = FR_W, FR_P8, FR_P7 
	nop.i 999 ;;
}
{ .mfi
(p0)  ldfe FR_P1 = [GR_Table_ptr],16 
//
//    Load P1
//    Load P5
//    Y_lo = p8 * w + P7
//
(p0)  fmpy.s1 FR_w4 = FR_wsq, FR_wsq 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fma.s1 FR_Y_hi = FR_W, FR_Y_hi, FR_P2 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)  fma.s1 FR_Y_lo = FR_W, FR_Y_lo, FR_P6 
(p0)  add GR_Perturb = 0x1, r0 ;; 
}
{ .mfi
	nop.m 999
//
//    w4 = w2 * w2 
//    Y_hi = y_hi * w + p2 
//    Y_lo = y_lo * w + p6 
//    Create perturbation bit
//
(p0)  fmpy.s1 FR_w6 = FR_w4, FR_wsq 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fma.s1 FR_Y_hi = FR_W, FR_Y_hi, FR_P1 
	nop.i 999
}
//
//    Y_hi = y_hi * w + p1 
//    w6 = w4 * w2 
//
{ .mfi
(p0)  setf.sig FR_Q4 = GR_Perturb 
(p0)  fma.s1 FR_Y_lo = FR_W, FR_Y_lo, FR_P5 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fma.s1 FR_dummy = FR_wsq,FR_Y_hi, f0 
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)  fma.s1 FR_Y_hi = FR_W,f1,f0 
	nop.i 999
};;
{ .mfb
	nop.m 999
//
//    Y_hi = w 
//    Y_lo = y_lo * w + p5 
//
(p0)  fma.s1 FR_Y_lo = FR_w6, FR_Y_lo,FR_dummy 
//
//    Y_lo = y_lo * w6   + y_high order part. 
//
//    performance
//
(p0)  br.cond.sptk LOGL_main ;; 
}
L(log1pl_small): 
{ .mmi
	nop.m 999
//  /*******************************************************/
//  /*********** Branch log1pl_small  ***********************/
//  /*******************************************************/
(p0)  addl GR_Table_Base = @ltoff(Constants_Threshold#),gp
}
{ .mfi
      nop.m 999
(p0)  mov FR_Em1 = FR_W 
(p0)  cmp.eq.unc  p7, p0 = r0, r0 ;; 
}
{ .mlx
      ld8    GR_Table_Base = [GR_Table_Base]
(p0)  movl GR_Expo_Range = 0x0000000000000004 ;; 
}
//
//    Set Safe to true
//    Set Expo_Range = 0 for single
//    Set Expo_Range = 2 for double 
//    Set Expo_Range = 4 for double-extended 
//
{ .mmi
(p0)  shladd GR_Table_Base = GR_Expo_Range,4,GR_Table_Base ;; 
(p0)  ldfe FR_Threshold = [GR_Table_Base],16 
	nop.i 999
}
{ .mlx
	nop.m 999
(p0)  movl GR_Bias = 0x000000000000FF9B ;; 
}
{ .mfi
(p0)  ldfe FR_Tiny = [GR_Table_Base],0 
	nop.f 999
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fcmp.gt.unc.s1 p13, p12 =  FR_abs_W, FR_Threshold 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p13) fnmpy.s1 FR_Y_lo = FR_W, FR_W 
	nop.i 999
}
{ .mfi
	nop.m 999
(p13) fadd FR_SCALE = f0, f1 
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p12) fsub.s1 FR_Y_lo = f0, FR_Tiny 
(p12) cmp.ne.unc  p7, p0 = r0, r0 
}
{ .mfi
(p12) setf.exp FR_SCALE = GR_Bias 
	nop.f 999
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//
//    Set p7 to SAFE = FALSE
//    Set Scale = 2^-100 
//
(p0)  fma.s0 f8 = FR_Y_lo,FR_SCALE,FR_Y_hi
(p0)  br.ret.sptk   b0 ;; 
}
L(LOGL_64_one): 
{ .mfb
	nop.m 999
(p0)  fmpy.s0 f8 = FR_Input_X, f0 
(p0)  br.ret.sptk   b0 ;; 
}
//    
//    Raise divide by zero for +/-0 input.
//    
L(LOGL_64_zero): 
{ .mfi
(p0)  mov   GR_Parameter_TAG = 0
//
//    If we have logl(1), log10l(1) or log1pl(0), return 0.
//  
(p0)  fsub.s0 FR_Output_X_tmp = f0, f1 
	nop.i 999 ;;
}
{ .mii
(p14) mov   GR_Parameter_TAG = 6 
	nop.i 999 ;;
(p15) mov   GR_Parameter_TAG = 138 ;; 
}
{ .mfb
	nop.m 999
(p0)  frcpa.s0 FR_Output_X_tmp, p8 =  FR_Output_X_tmp, f0 
(p0)  br.cond.sptk __libm_error_region ;; 
}
{ .mfb
	nop.m 999
//     
//    Report that logl(0) computed
//     { .mfb
(p0)  mov   FR_Input_X     = FR_Output_X_tmp
(p0)  br.ret.sptk   b0 ;;
}

L(LOGL_64_special): 
{ .mfi
	nop.m 999
//    
//    Return -Inf or value from handler.
//    
(p0)  fclass.m.unc p7, p0 =  FR_Input_X, 0x1E1 
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//     
//    Check for Natval, QNan, SNaN, +Inf   
//    
(p7)  fmpy.s0 f8 =  FR_Input_X, f1 
//     
//    For SNaN raise invalid and return QNaN.
//    For QNaN raise invalid and return QNaN.
//    For +Inf return +Inf.
//    
(p7)  br.ret.sptk   b0 ;;
}
//    
//    For -Inf raise invalid and return QNaN.
//    
{ .mii
(p0)  mov   GR_Parameter_TAG = 1
	nop.i 999 ;;
(p14) mov   GR_Parameter_TAG = 7 ;;
}
{ .mfi
(p15) mov   GR_Parameter_TAG = 139 
	nop.f 999
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p0)  fmpy.s0 FR_Output_X_tmp =  FR_Input_X, f0 
(p0)  br.cond.sptk __libm_error_region ;; 
}
//     
//    Report that logl(-Inf) computed
//    Report that log10l(-Inf) computed
//    Report that log1p(-Inf) computed
//     
{ .mfb
      nop.m 0
(p0)  mov   FR_Input_X     = FR_Output_X_tmp
(p0)  br.ret.sptk   b0 ;;
}
L(LOGL_64_unsupported): 
{ .mfb
	nop.m 999
//    
//    Return generated NaN or other value .
//    
(p0)  fmpy.s0 f8 = FR_Input_X, f0 
(p0)  br.ret.sptk   b0 ;;
}
L(LOGL_64_negative): 
{ .mfi
	nop.m 999
//     
//    Deal with x < 0 in a special way 
//    
(p0)  frcpa.s0 FR_Output_X_tmp, p8 =  f0, f0 
//     
//    Deal with x < 0 in a special way - raise
//    invalid and produce QNaN indefinite.
//    
(p0)  mov   GR_Parameter_TAG = 1 ;; 
}
{ .mii
(p14) mov   GR_Parameter_TAG = 7
	nop.i 999 ;;
(p15) mov   GR_Parameter_TAG = 139
}
.endp log1pl
ASM_SIZE_DIRECTIVE(log1pl) 

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

.proc LOGL_main 
LOGL_main: 
{ .mfi
	nop.m 999
//
//    kernel_log_64 computes ln(X + E)
//
(p7)  fadd.s0 FR_Input_X = FR_Y_lo,FR_Y_hi
      nop.i 0
}
{ .mmi
      nop.m 999
      nop.m 999
(p14) addl GR_Table_Base = @ltoff(Constants_1_by_LN10#),gp ;; 
}
{ .mmi
      nop.m 999
(p14) ld8    GR_Table_Base = [GR_Table_Base]
      nop.i 999
};;

{ .mmi
(p14) ldfe FR_1LN10_hi = [GR_Table_Base],16 ;; 
(p14) ldfe FR_1LN10_lo = [GR_Table_Base]
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p14) fmpy.s1 FR_Output_X_tmp = FR_Y_lo,FR_1LN10_hi
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p14) fma.s1  FR_Output_X_tmp = FR_Y_hi,FR_1LN10_lo,FR_Output_X_tmp
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p14) fma.s0 FR_Input_X = FR_Y_hi,FR_1LN10_hi,FR_Output_X_tmp
(p0)  br.ret.sptk   b0 ;; 
}
.endp LOGL_main
ASM_SIZE_DIRECTIVE(LOGL_main) 

.type   __libm_error_support#,@function
.global __libm_error_support#
