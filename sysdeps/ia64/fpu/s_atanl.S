.file "atanl.s"

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
//
// *********************************************************************
//
// History
// 2/02/00  (hand-optimized)
// 4/04/00  Unwind support added
// 8/15/00  Bundle added after call to __libm_error_support to properly
//          set [the previously overwritten] GR_Parameter_RESULT.
//
// *********************************************************************
//
// Function:   atanl(x) = inverse tangent(x), for double extended x values
// Function:   atan2l(y,x) = atan(y/x), for double extended x values
//
// *********************************************************************
//
// Resources Used:
//
//    Floating-Point Registers: f8 (Input and Return Value)
//                              f9-f15
//                              f32-f79
//
//    General Purpose Registers:
//      r32-r48
//      r49,r50,r51,r52 (Arguments to error support for 0,0 case)
//
//    Predicate Registers:      p6-p15
//
// *********************************************************************
//
// IEEE Special Conditions:
//
//    Denormal  fault raised on denormal inputs
//    Underflow exceptions may occur 
//    Special error handling for the y=0 and x=0 case
//    Inexact raised when appropriate by algorithm
//
//    atanl(SNaN) = QNaN
//    atanl(QNaN) = QNaN
//    atanl(+/-0) = +/- 0
//    atanl(+/-Inf) = +/-pi/2 
//
//    atan2l(Any NaN for x or y) = QNaN
//    atan2l(+/-0,x) = +/-0 for x > 0 
//    atan2l(+/-0,x) = +/-pi for x < 0 
//    atan2l(+/-0,+0) = +/-0 
//    atan2l(+/-0,-0) = +/-pi 
//    atan2l(y,+/-0) = pi/2 y > 0
//    atan2l(y,+/-0) = -pi/2 y < 0
//    atan2l(+/-y, Inf) = +/-0 for finite y > 0
//    atan2l(+/-Inf, x) = +/-pi/2 for finite x 
//    atan2l(+/-y, -Inf) = +/-pi for finite  y > 0 
//    atan2l(+/-Inf, Inf) = +/-pi/4
//    atan2l(+/-Inf, -Inf) = +/-3pi/4
//
// *********************************************************************
//
// Mathematical Description
// ---------------------------
//
// The function ATANL( Arg_Y, Arg_X ) returns the "argument"
// or the "phase" of the complex number
//
//           Arg_X + i Arg_Y
//
// or equivalently, the angle in radians from the positive
// x-axis to the line joining the origin and the point
// (Arg_X,Arg_Y)
//
//
//        (Arg_X, Arg_Y) x
//                        \ 
//                \ 
//                 \ 
//                  \ 
//                   \ angle between is ATANL(Arg_Y,Arg_X)




//                    \ 
//                     ------------------> X-axis

//                   Origin
//
// Moreover, this angle is reported in the range [-pi,pi] thus
//
//      -pi <= ATANL( Arg_Y, Arg_X ) <= pi.
//
// From the geometry, it is easy to define ATANL when one of
// Arg_X or Arg_Y is +-0 or +-inf:
//
//
//      \ Y |
//     X \  |  +0  | -0  |  +inf |  -inf  |  finite non-zero
//        \ |      |     |       |        |
//    ______________________________________________________
//          |            |       |        |
//     +-0  |   Invalid/ |  pi/2 | -pi/2  |  sign(Y)*pi/2
//          |    qNaN    |       |        |
//  --------------------------------------------------------
//          |      |     |       |        |
//     +inf |  +0  | -0  |  pi/4 | -pi/4  |  sign(Y)*0
//  --------------------------------------------------------
//          |      |     |       |        |
//     -inf |  +pi | -pi | 3pi/4 | -3pi/4 |  sign(Y)*pi
//  --------------------------------------------------------
//   finite |    X>0?    |  pi/2 | -pi/2  |  normal case
//  non-zero| sign(Y)*0: |       |        |
//       | sign(Y)*pi |       |        |
//
//
// One must take note that ATANL is NOT the arctangent of the
// value Arg_Y/Arg_X; but rather ATANL and arctan are related
// in a slightly more complicated way as follows:
//
// Let U := max(|Arg_X|, |Arg_Y|);  V := min(|Arg_X|, |Arg_Y|);
// sign_X be the sign bit of Arg_X, i.e., sign_X is 0 or 1;
// s_X    be the sign     of Arg_X, i.e., s_X = (-1)^sign_X;
//
// sign_Y be the sign bit of Arg_Y, i.e., sign_Y is 0 or 1;
// s_Y    be the sign     of Arg_Y, i.e., s_Y = (-1)^sign_Y;
//
// swap   be 0  if |Arg_X| >= |Arg_Y|  and 1 otherwise.
//
// Then, ATANL(Arg_Y, Arg_X) =
//
//       /    arctan(V/U)     \      sign_X = 0 & swap = 0
//       | pi/2 - arctan(V/U) |      sign_X = 0 & swap = 1
// s_Y * |                    |
//       |  pi  - arctan(V/U) |      sign_X = 1 & swap = 0
//       \ pi/2 + arctan(V/U) /      sign_X = 1 & swap = 1
//
//
// This relationship also suggest that the algorithm's major
// task is to calculate arctan(V/U) for 0 < V <= U; and the
// final Result is given by
//
//      s_Y * { (P_hi + P_lo) + sigma * arctan(V/U) }
//
// where
//
//   (P_hi,P_lo) represents M(sign_X,swap)*(pi/2) accurately
//
//   M(sign_X,swap) = 0  for sign_X = 0 and swap = 0
//              1  for swap   = 1
//              2  for sign_X = 1 and swap = 0
//
// and
//
//   sigma = { (sign_X  XOR  swap) :  -1.0 : 1.0 }
//
//      =  (-1) ^ ( sign_X XOR swap )
//
// Both (P_hi,P_lo) and sigma can be stored in a table and fetched
// using (sign_X,swap) as an index. (P_hi, P_lo) can be stored as a
// double-precision, and single-precision pair; and sigma can
// obviously be just a single-precision number.
//
// In the algorithm we propose, arctan(V/U) is calculated to high accuracy
// as A_hi + A_lo. Consequently, the Result ATANL( Arg_Y, Arg_X ) is
// given by
//
//    s_Y*P_hi + s_Y*sigma*A_hi + s_Y*(sigma*A_lo + P_lo)
//
// We now discuss the calculation of arctan(V/U) for 0 < V <= U.
//
// For (V/U) < 2^(-3), we use a simple polynomial of the form
//
//      z + z^3*(P_1 + z^2*(P_2 + z^2*(P_3 + ... + P_8)))
//
// where z = V/U.
//
// For the sake of accuracy, the first term "z" must approximate V/U to
// extra precision. For z^3 and higher power, a working precision
// approximation to V/U suffices. Thus, we obtain:
//
//      z_hi + z_lo = V/U  to extra precision and
//      z           = V/U  to working precision
//
// The value arctan(V/U) is delivered as two pieces (A_hi, A_lo)
//
//      (A_hi,A_lo) = (z_hi, z^3*(P_1 + ... + P_8) + z_lo).
//
//
// For 2^(-3) <= (V/U) <= 1, we use a table-driven approach.
// Consider
//
//      (V/U) = 2^k * 1.b_1 b_2 .... b_63 b_64 b_65 ....
//
// Define
//
//       z_hi = 2^k * 1.b_1 b_2 b_3 b_4 1
//
// then
//                                            /                \ 
//                                            |  (V/U) - z_hi  |

//      arctan(V/U) = arctan(z_hi) + acrtan| -------------- |
//                                            | 1 + (V/U)*z_hi |
//                                            \                /
//
//                                            /                \ 
//                                            |   V - z_hi*U   |

//                  = arctan(z_hi) + acrtan| -------------- |
//                                            |   U + V*z_hi   |
//                                            \                /
//
//                  = arctan(z_hi) + acrtan( V' / U' )
//
//
// where
//
//      V' = V - U*z_hi;   U' = U + V*z_hi.
//
// Let
//
//      w_hi + w_lo  = V'/U' to extra precision and
//           w       = V'/U' to working precision
//
// then we can approximate arctan(V'/U') by
//
//      arctan(V'/U') = w_hi + w_lo
//                     + w^3*(Q_1 + w^2*(Q_2 + w^2*(Q_3 + w^2*Q_4)))
//
//                       = w_hi + w_lo + poly
//
// Finally, arctan(z_hi) is calculated beforehand and stored in a table
// as Tbl_hi, Tbl_lo. Thus,
//
//      (A_hi, A_lo) = (Tbl_hi, w_hi+(poly+(w_lo+Tbl_lo)))
//
// This completes the mathematical description.
//
//
// Algorithm
// -------------
//
// Step 0. Check for unsupported format.
//
//    If
//       ( expo(Arg_X) not zero AND msb(Arg_X) = 0 ) OR
//       ( expo(Arg_Y) not zero AND msb(Arg_Y) = 0 )
//
//    then one of the arguments is unsupported. Generate an
//    invalid and return qNaN.
//
// Step 1. Initialize
//
//    Normalize Arg_X and Arg_Y and set the following
//
//    sign_X :=  sign_bit(Arg_X)
//    s_Y    := (sign_bit(Arg_Y)==0? 1.0 : -1.0)
//    swap   := (|Arg_X| >= |Arg_Y|?   0 :  1  )
//    U      := max( |Arg_X|, |Arg_Y| )
//    V      := min( |Arg_X|, |Arg_Y| )
//
//    execute: frcap E, pred, V, U
//    If pred is 0, go to Step 5 for special cases handling.
//
// Step 2. Decide on branch.
//
//    Q := E * V
//    If Q < 2^(-3) go to Step 4 for simple polynomial case.
//
// Step 3. Table-driven algorithm.
//
//    Q is represented as
//
//      2^(-k) * 1.b_1 b_2 b_3 ... b_63; k = 0,-1,-2,-3
//
// and that if k = 0, b_1 = b_2 = b_3 = b_4 = 0.
//
// Define
//
//      z_hi := 2^(-k) * 1.b_1 b_2 b_3 b_4 1
//
// (note that there are 49 possible values of z_hi).
//
//      ...We now calculate V' and U'. While V' is representable
//      ...as a 64-bit number because of cancellation, U' is
//      ...not in general a 64-bit number. Obtaining U' accurately
//      ...requires two working precision numbers
//
//      U_prime_hi := U + V * z_hi            ...WP approx. to U'
//      U_prime_lo := ( U - U_prime_hi ) + V*z_hi ...observe order
//      V_prime    := V - U * z_hi             ...this is exact
//
//         C_hi := frcpa (1.0, U_prime_hi)  ...C_hi approx 1/U'_hi
//
//      loop 3 times
//         C_hi := C_hi + C_hi*(1.0 - C_hi*U_prime_hi)
//
//      ...at this point C_hi is (1/U_prime_hi) to roughly 64 bits
//
//      w_hi := V_prime * C_hi     ...w_hi is V_prime/U_prime to
//                     ...roughly working precision
//
//         ...note that we want w_hi + w_lo to approximate
//      ...V_prime/(U_prime_hi + U_prime_lo) to extra precision
//         ...but for now, w_hi is good enough for the polynomial
//      ...calculation.
//
//         wsq  := w_hi*w_hi
//      poly := w_hi*wsq*(Q_1 + wsq*(Q_2 + wsq*(Q_3 + wsq*Q_4)))
//
//      Fetch
//      (Tbl_hi, Tbl_lo) = atan(z_hi) indexed by (k,b_1,b_2,b_3,b_4)
//      ...Tbl_hi is a double-precision number
//      ...Tbl_lo is a single-precision number
//
//         (P_hi, P_lo) := M(sign_X,swap)*(Pi_by_2_hi, Pi_by_2_lo)
//      ...as discussed previous. Again; the implementation can
//      ...chose to fetch P_hi and P_lo from a table indexed by
//      ...(sign_X, swap).
//      ...P_hi is a double-precision number;
//      ...P_lo is a single-precision number.
//
//      ...calculate w_lo so that w_hi + w_lo is V'/U' accurately
//         w_lo := ((V_prime - w_hi*U_prime_hi) -
//              w_hi*U_prime_lo) * C_hi     ...observe order
//
//
//      ...Ready to deliver arctan(V'/U') as A_hi, A_lo
//      A_hi := Tbl_hi
//      A_lo := w_hi + (poly + (Tbl_lo + w_lo)) ...observe order
//
//      ...Deliver final Result
//      ...s_Y*P_hi + s_Y*sigma*A_hi + s_Y*(sigma*A_lo + P_lo)
//
//      sigma := ( (sign_X XOR swap) ? -1.0 : 1.0 )
//      ...sigma can be obtained by a table lookup using
//      ...(sign_X,swap) as index and stored as single precision
//         ...sigma should be calculated earlier
//
//      P_hi := s_Y*P_hi
//      A_hi := s_Y*A_hi
//
//      Res_hi := P_hi + sigma*A_hi     ...this is exact because
//                          ...both P_hi and Tbl_hi
//                          ...are double-precision
//                          ...and |Tbl_hi| > 2^(-4)
//                          ...P_hi is either 0 or
//                          ...between (1,4)
//
//      Res_lo := sigma*A_lo + P_lo
//
//      Return Res_hi + s_Y*Res_lo in user-defined rounding control
//
// Step 4. Simple polynomial case.
//
//    ...E and Q are inherited from Step 2.
//
//    A_hi := Q     ...Q is inherited from Step 2 Q approx V/U
//
//    loop 3 times
//       E := E + E2(1.0 - E*U1
//    ...at this point E approximates 1/U to roughly working precision
//
//    z := V * E     ...z approximates V/U to roughly working precision
//    zsq := z * z
//    z8 := zsq * zsq; z8 := z8 * z8
//
//    poly1 := P_4 + zsq*(P_5 + zsq*(P_6 + zsq*(P_7 + zsq*P_8)))
//    poly2 := zsq*(P_1 + zsq*(P_2 + zsq*P_3))
//
//    poly  := poly1 + z8*poly2
//
//    z_lo := (V - A_hi*U)*E
//
//    A_lo := z*poly + z_lo
//    ...A_hi, A_lo approximate arctan(V/U) accurately
//
//    (P_hi, P_lo) := M(sign_X,swap)*(Pi_by_2_hi, Pi_by_2_lo)
//    ...one can store the M(sign_X,swap) as single precision
//    ...values
//
//    ...Deliver final Result
//    ...s_Y*P_hi + s_Y*sigma*A_hi + s_Y*(sigma*A_lo + P_lo)
//
//    sigma := ( (sign_X XOR swap) ? -1.0 : 1.0 )
//    ...sigma can be obtained by a table lookup using
//    ...(sign_X,swap) as index and stored as single precision
//    ...sigma should be calculated earlier
//
//    P_hi := s_Y*P_hi
//    A_hi := s_Y*A_hi
//
//    Res_hi := P_hi + sigma*A_hi          ...need to compute
//                          ...P_hi + sigma*A_hi
//                          ...exactly
//
//    tmp    := (P_hi - Res_hi) + sigma*A_hi
//
//    Res_lo := s_Y*(sigma*A_lo + P_lo) + tmp
//
//    Return Res_hi + Res_lo in user-defined rounding control
//
// Step 5. Special Cases
//
//    If pred is 0 where pred is obtained in
//        frcap E, pred, V, U
//
//    we are in one of those special cases of 0,+-inf or NaN
//
//    If one of U and V is NaN, return U+V (which will generate
//    invalid in case one is a signaling NaN). Otherwise,
//    return the Result as described in the table
//
//
//
//      \ Y |
//     X \  |  +0  | -0  |  +inf |  -inf  |  finite non-zero
//        \ |      |     |       |        |
//    ______________________________________________________
//          |            |       |        |
//     +-0  |   Invalid/ |  pi/2 | -pi/2  |  sign(Y)*pi/2
//          |    qNaN    |       |        |
//  --------------------------------------------------------
//          |      |     |       |        |
//     +inf |  +0  | -0  |  pi/4 | -pi/4  |  sign(Y)*0
//  --------------------------------------------------------
//          |      |     |       |        |
//     -inf |  +pi | -pi | 3pi/4 | -3pi/4 |  sign(Y)*pi
//  --------------------------------------------------------
//   finite |    X>0?    |  pi/2 | -pi/2  |
//  non-zero| sign(Y)*0: |       |        |      N/A
//       | sign(Y)*pi |       |        |
//
//

#include "libm_support.h"

ArgY_orig   =   f8
Result      =   f8
FR_RESULT   =   f8
ArgX_orig   =   f9
ArgX        =   f10
FR_X        =   f10
ArgY        =   f11
FR_Y        =   f11
s_Y         =   f12
U           =   f13
V           =   f14
E           =   f15
Q           =   f32
z_hi        =   f33
U_prime_hi  =   f34
U_prime_lo  =   f35
V_prime     =   f36
C_hi        =   f37
w_hi        =   f38
w_lo        =   f39
wsq         =   f40
poly        =   f41
Tbl_hi      =   f42
Tbl_lo      =   f43
P_hi        =   f44
P_lo        =   f45
A_hi        =   f46
A_lo        =   f47
sigma       =   f48
Res_hi      =   f49
Res_lo      =   f50
Z           =   f52
zsq         =   f53
z8          =   f54
poly1       =   f55
poly2       =   f56
z_lo        =   f57
tmp         =   f58
P_1         =   f59
Q_1         =   f60
P_2         =   f61
Q_2         =   f62
P_3         =   f63
Q_3         =   f64
P_4         =   f65
Q_4         =   f66
P_5         =   f67
P_6         =   f68
P_7         =   f69
P_8         =   f70
TWO_TO_NEG3 =   f71
U_hold      =   f72
C_hi_hold   =   f73
E_hold      =   f74
M           =   f75
ArgX_abs    =   f76
ArgY_abs    =   f77
Result_lo   =   f78
A_temp      =   f79
GR_SAVE_PFS   = r33
GR_SAVE_B0    = r34
GR_SAVE_GP    = r35
sign_X        = r36
sign_Y        = r37 
swap          = r38 
table_ptr1    = r39 
table_ptr2    = r40 
k             = r41 
lookup        = r42 
exp_ArgX      = r43 
exp_ArgY      = r44 
exponent_Q    = r45 
significand_Q = r46 
special       = r47 
special1      = r48 
GR_Parameter_X      = r49
GR_Parameter_Y      = r50
GR_Parameter_RESULT = r51
GR_Parameter_TAG    = r52
int_temp            = r52

#ifdef _LIBC
.rodata
#else
.data
#endif
.align 64 

Constants_atan:
ASM_TYPE_DIRECTIVE(Constants_atan,@object)
data4    0x54442D18, 0x3FF921FB, 0x248D3132, 0x3E000000
//       double pi/2, single lo_pi/2, two**(-3)
data4    0xAAAAAAA3, 0xAAAAAAAA, 0x0000BFFD, 0x00000000 // P_1
data4    0xCCCC54B2, 0xCCCCCCCC, 0x00003FFC, 0x00000000 // P_2
data4    0x47E4D0C2, 0x92492492, 0x0000BFFC, 0x00000000 // P_3
data4    0x58870889, 0xE38E38E0, 0x00003FFB, 0x00000000 // P_4
data4    0x290149F8, 0xBA2E895B, 0x0000BFFB, 0x00000000 // P_5
data4    0x250F733D, 0x9D88E6D4, 0x00003FFB, 0x00000000 // P_6
data4    0xFB8745A0, 0x884E51FF, 0x0000BFFB, 0x00000000 // P_7
data4    0x394396BD, 0xE1C7412B, 0x00003FFA, 0x00000000 // P_8
data4    0xAAAAA52F, 0xAAAAAAAA, 0x0000BFFD, 0x00000000 // Q_1
data4    0xC75B60D3, 0xCCCCCCCC, 0x00003FFC, 0x00000000 // Q_2
data4    0x011F1940, 0x924923AD, 0x0000BFFC, 0x00000000 // Q_3
data4    0x2A5F89BD, 0xE36F716D, 0x00003FFB, 0x00000000 // Q_4
//
//    Entries Tbl_hi  (double precision)
//    B = 1+Index/16+1/32  Index = 0
//    Entries Tbl_lo (single precision)
//    B = 1+Index/16+1/32  Index = 0
//
data4   0xA935BD8E, 0x3FE9A000, 0x23ACA08F, 0x00000000
//
//    Entries Tbl_hi  (double precision) Index = 0,1,...,15
//    B = 2^(-1)*(1+Index/16+1/32)
//    Entries Tbl_lo (single precision)
//    Index = 0,1,...,15  B = 2^(-1)*(1+Index/16+1/32)
//
data4   0x7F175A34, 0x3FDE77EB, 0x238729EE, 0x00000000
data4   0x73C1A40B, 0x3FE0039C, 0x249334DB, 0x00000000
data4   0x5B5B43DA, 0x3FE0C614, 0x22CBA7D1, 0x00000000
data4   0x88BE7C13, 0x3FE1835A, 0x246310E7, 0x00000000
data4   0xE2CC9E6A, 0x3FE23B71, 0x236210E5, 0x00000000
data4   0x8406CBCA, 0x3FE2EE62, 0x2462EAF5, 0x00000000
data4   0x1CD41719, 0x3FE39C39, 0x24B73EF3, 0x00000000
data4   0x5B795B55, 0x3FE44506, 0x24C11260, 0x00000000
data4   0x5BB6EC04, 0x3FE4E8DE, 0x242519EE, 0x00000000
data4   0x1F732FBA, 0x3FE587D8, 0x24D4346C, 0x00000000
data4   0x115D7B8D, 0x3FE6220D, 0x24ED487B, 0x00000000
data4   0x920B3D98, 0x3FE6B798, 0x2495FF1E, 0x00000000
data4   0x8FBA8E0F, 0x3FE74897, 0x223D9531, 0x00000000
data4   0x289FA093, 0x3FE7D528, 0x242B0411, 0x00000000
data4   0x576CC2C5, 0x3FE85D69, 0x2335B374, 0x00000000
data4   0xA99CC05D, 0x3FE8E17A, 0x24C27CFB, 0x00000000
//
//    Entries Tbl_hi  (double precision) Index = 0,1,...,15
//    B = 2^(-2)*(1+Index/16+1/32)
//    Entries Tbl_lo (single precision)
//    Index = 0,1,...,15  B = 2^(-2)*(1+Index/16+1/32)
//
data4    0x510665B5, 0x3FD025FA, 0x24263482, 0x00000000
data4    0x362431C9, 0x3FD1151A, 0x242C8DC9, 0x00000000
data4    0x67E47C95, 0x3FD20255, 0x245CF9BA, 0x00000000
data4    0x7A823CFE, 0x3FD2ED98, 0x235C892C, 0x00000000
data4    0x29271134, 0x3FD3D6D1, 0x2389BE52, 0x00000000
data4    0x586890E6, 0x3FD4BDEE, 0x24436471, 0x00000000
data4    0x175E0F4E, 0x3FD5A2E0, 0x2389DBD4, 0x00000000
data4    0x9F5FA6FD, 0x3FD68597, 0x2476D43F, 0x00000000
data4    0x52817501, 0x3FD76607, 0x24711774, 0x00000000
data4    0xB8DF95D7, 0x3FD84422, 0x23EBB501, 0x00000000
data4    0x7CD0C662, 0x3FD91FDE, 0x23883A0C, 0x00000000
data4    0x66168001, 0x3FD9F930, 0x240DF63F, 0x00000000
data4    0x5422058B, 0x3FDAD00F, 0x23FE261A, 0x00000000
data4    0x378624A5, 0x3FDBA473, 0x23A8CD0E, 0x00000000
data4    0x0AAD71F8, 0x3FDC7655, 0x2422D1D0, 0x00000000
data4    0xC9EC862B, 0x3FDD45AE, 0x2344A109, 0x00000000
//
//    Entries Tbl_hi  (double precision) Index = 0,1,...,15
//    B = 2^(-3)*(1+Index/16+1/32)
//    Entries Tbl_lo (single precision)
//    Index = 0,1,...,15  B = 2^(-3)*(1+Index/16+1/32)
//
data4    0x84212B3D, 0x3FC068D5, 0x239874B6, 0x00000000
data4    0x41060850, 0x3FC16465, 0x2335E774, 0x00000000
data4    0x171A535C, 0x3FC25F6E, 0x233E36BE, 0x00000000
data4    0xEDEB99A3, 0x3FC359E8, 0x239680A3, 0x00000000
data4    0xC6092A9E, 0x3FC453CE, 0x230FB29E, 0x00000000
data4    0xBA11570A, 0x3FC54D18, 0x230C1418, 0x00000000
data4    0xFFB3AA73, 0x3FC645BF, 0x23F0564A, 0x00000000
data4    0xE8A7D201, 0x3FC73DBD, 0x23D4A5E1, 0x00000000
data4    0xE398EBC7, 0x3FC8350B, 0x23D4ADDA, 0x00000000
data4    0x7D050271, 0x3FC92BA3, 0x23BCB085, 0x00000000
data4    0x601081A5, 0x3FCA217E, 0x23BC841D, 0x00000000
data4    0x574D780B, 0x3FCB1696, 0x23CF4A8E, 0x00000000
data4    0x4D768466, 0x3FCC0AE5, 0x23BECC90, 0x00000000
data4    0x4E1D5395, 0x3FCCFE65, 0x2323DCD2, 0x00000000
data4    0x864C9D9D, 0x3FCDF110, 0x23F53F3A, 0x00000000
data4    0x451D980C, 0x3FCEE2E1, 0x23CCB11F, 0x00000000

data4    0x54442D18, 0x400921FB, 0x33145C07, 0x3CA1A626 // PI two doubles
data4    0x54442D18, 0x3FF921FB, 0x33145C07, 0x3C91A626 // PI_by_2 two dbles
data4    0x54442D18, 0x3FE921FB, 0x33145C07, 0x3C81A626 // PI_by_4 two dbles
data4    0x7F3321D2, 0x4002D97C, 0x4C9E8A0A, 0x3C9A7939 // 3PI_by_4 two dbles
ASM_SIZE_DIRECTIVE(Constants_atan)


.text
.proc atanl#
.global atanl#
.align 64

atanl: 
{ .mfb
	nop.m 999
(p0)   mov ArgX_orig = f1 
(p0)   br.cond.sptk atan2l ;;
}
.endp atanl
ASM_SIZE_DIRECTIVE(atanl)

.text
.proc atan2l#
.global atan2l#
#ifdef _LIBC
.proc __atan2l#
.global __atan2l#
.proc __ieee754_atan2l#
.global __ieee754_atan2l#
#endif
.align 64 


atan2l:
#ifdef _LIBC
__atan2l:
__ieee754_atan2l:
#endif
{ .mfi
alloc r32 = ar.pfs, 0, 17 , 4, 0
(p0)  mov   ArgY = ArgY_orig
}
{ .mfi
	nop.m 999
(p0)  mov   ArgX = ArgX_orig
	nop.i 999
};;
{ .mfi
	nop.m 999
(p0)   fclass.m.unc p7,p0 = ArgY_orig, 0x103
	nop.i 999 
}
{ .mfi
	nop.m 999
//
//
//  Save original input args and load table ptr.
//
(p0)   fclass.m.unc p6,p0 = ArgX_orig, 0x103
	nop.i 999
};;
{ .mfi
(p0)   addl      table_ptr1   = @ltoff(Constants_atan#), gp
(p0)   fclass.m.unc p0,p9 = ArgY_orig, 0x1FF
	nop.i 999 ;;
}
{ .mfi
       ld8 table_ptr1 = [table_ptr1]
(p0)   fclass.m.unc p0,p8 = ArgX_orig, 0x1FF
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fclass.m.unc p13,p0 = ArgY_orig, 0x0C3
	nop.i 999 ;;
}
{ .mfi
(p0)   fclass.m.unc p12,p0 = ArgX_orig, 0x0C3
	nop.i 999
}


//
//     Check for NatVals.
//     Check for everything - if false, then must be pseudo-zero
//     or pseudo-nan (IA unsupporteds).
//
{ .mib
	nop.m 999
	nop.i 999
(p6)   br.cond.spnt L(ATANL_NATVAL) ;;
}

{ .mib
	nop.m 999
	nop.i 999
(p7)   br.cond.spnt L(ATANL_NATVAL) ;;
}
{ .mib
(p0)   ldfd P_hi = [table_ptr1],8
	nop.i 999
(p8)   br.cond.spnt L(ATANL_UNSUPPORTED) ;;
}
{ .mbb
(p0)   add table_ptr2 = 96, table_ptr1
(p9)   br.cond.spnt L(ATANL_UNSUPPORTED)
//
//     Load double precision high-order part of pi
//
(p12)  br.cond.spnt L(ATANL_NAN) ;;
}
{ .mfb
	nop.m 999
(p0)   fnorm.s1 ArgX = ArgX
(p13)  br.cond.spnt L(ATANL_NAN) ;;
}
//
//     Normalize the input argument.
//     Branch out if NaN inputs
//
{ .mmf
(p0)   ldfs P_lo = [table_ptr1], 4
	nop.m 999
(p0)   fnorm.s1 ArgY = ArgY ;;
}
{ .mmf
	nop.m 999
(p0)   ldfs TWO_TO_NEG3 = [table_ptr1], 180
//
//     U = max(ArgX_abs,ArgY_abs)
//     V = min(ArgX_abs,ArgY_abs)
//     if PR1, swap = 0
//     if PR2, swap = 1
//
(p0)   mov M = f1 ;;
}
{ .mfi
	nop.m 999
//
//     Get exp and sign of ArgX
//     Get exp and sign of ArgY
//     Load 2**(-3) and increment ptr to Q_4.
//
(p0)   fmerge.s ArgX_abs = f1, ArgX
	nop.i 999 ;;
}
//
//     load single precision low-order part of pi = P_lo
//
{ .mfi
(p0)   getf.exp sign_X = ArgX
(p0)   fmerge.s ArgY_abs = f1, ArgY
	nop.i 999 ;;
}
{ .mii
(p0)   getf.exp sign_Y = ArgY
	nop.i 999 ;;
(p0)   shr sign_X = sign_X, 17 ;;
}
{ .mii
	nop.m 999
(p0)   shr sign_Y = sign_Y, 17 ;;
(p0)   cmp.eq.unc p8, p9 = 0x00000, sign_Y ;;
}
{ .mfi
	nop.m 999
//
//     Is ArgX_abs >= ArgY_abs
//     Is sign_Y == 0?
//
(p0)   fmax.s1 U = ArgX_abs, ArgY_abs
	nop.i 999
}
{ .mfi
	nop.m 999
//
//     ArgX_abs = |ArgX|
//     ArgY_abs = |ArgY|
//     sign_X is sign bit of ArgX
//     sign_Y is sign bit of ArgY
//
(p0)   fcmp.ge.s1 p6, p7 = ArgX_abs, ArgY_abs
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fmin.s1 V = ArgX_abs, ArgY_abs
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p8)   fadd.s1 s_Y = f0, f1
(p6)   cmp.eq.unc p10, p11 = 0x00000, sign_X
}
{ .mii
(p6)   add swap = r0, r0
	nop.i 999 ;;
(p7)   add swap = 1, r0
}
{ .mfi
	nop.m 999
//
//     Let M = 1.0
//     if p8, s_Y = 1.0
//     if p9, s_Y = -1.0
//
(p10)  fsub.s1 M = M, f1
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p9)   fsub.s1 s_Y = f0, f1
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   frcpa.s1 E, p6 = V, U
	nop.i 999 ;;
}
{ .mbb
	nop.m 999
//
//     E = frcpa(V,U)
//
(p6)   br.cond.sptk L(ATANL_STEP2)
(p0)   br.cond.spnt L(ATANL_SPECIAL_HANDLING) ;;
}
L(ATANL_STEP2): 
{ .mfi
	nop.m 999
(p0)   fmpy.s1 Q = E, V
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fcmp.eq.s0     p0, p9 = f1, ArgY_orig
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Is Q < 2**(-3)?
//
(p0)   fcmp.eq.s0     p0, p8 = f1, ArgX_orig
	nop.i 999
}
{ .mfi
	nop.m 999
(p11)  fadd.s1 M = M, f1
	nop.i 999 ;;
}
{ .mlx
	nop.m 999
// *************************************************
// ********************* STEP2 *********************
// *************************************************
(p0)   movl special = 0x8400000000000000
}
{ .mlx
	nop.m 999
//
//     lookup = b_1 b_2 b_3 B_4
//
(p0)   movl special1 = 0x0000000000000100 ;;
}
{ .mfi
	nop.m 999
//
//     Do fnorms to raise any denormal operand
//     exceptions.
//
(p0)   fmpy.s1 P_hi = M, P_hi
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fmpy.s1 P_lo = M, P_lo
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Q = E * V
//
(p0)   fcmp.lt.unc.s1 p6, p7 = Q, TWO_TO_NEG3
	nop.i 999 ;;
}
{ .mmb
(p0)   getf.sig significand_Q = Q
(p0)   getf.exp exponent_Q =  Q
	nop.b 999 ;;
}
{ .mmi
	nop.m 999 ;;
(p0)   andcm k = 0x0003, exponent_Q
(p0)   extr.u lookup = significand_Q, 59, 4 ;;
}
{ .mib
	nop.m 999
(p0)   dep special = lookup, special, 59, 4
//
//     Generate 1.b_1 b_2 b_3 b_4 1 0 0 0 ... 0
//
(p6)   br.cond.spnt L(ATANL_POLY) ;;
}
{ .mfi
(p0)   cmp.eq.unc p8, p9 = 0x0000, k
(p0)   fmpy.s1 P_hi = s_Y, P_hi
//
//     We waited a few extra cycles so P_lo and P_hi could be calculated.
//     Load the constant 256 for loading up table entries.
//
// *************************************************
// ******************** STEP3 **********************
// *************************************************
(p0)   add table_ptr2 = 16, table_ptr1
}
//
//     Let z_hi have exponent and sign of original Q
//     Load the Tbl_hi(0) else, increment pointer.
//
{ .mii
(p0)   ldfe Q_4 = [table_ptr1], -16
(p0)   xor swap = sign_X, swap ;;
(p9)   sub k = k, r0, 1
}
{ .mmi
(p0)   setf.sig z_hi = special
(p0)   ldfe Q_3 = [table_ptr1], -16
(p9)   add table_ptr2 = 16, table_ptr2 ;;
}
//
//     U_hold = U - U_prime_hi
//     k = k * 256 - Result can be 0, 256, or 512.
//
{ .mmb
(p0)   ldfe Q_2 = [table_ptr1], -16
(p8)   ldfd Tbl_hi = [table_ptr2], 8
	nop.b 999 ;;
}
//
//     U_prime_lo =  U_hold + V * z_hi
//     lookup -> lookup * 16 + k
//
{ .mmi
(p0)   ldfe Q_1 = [table_ptr1], -16 ;;
(p8)   ldfs Tbl_lo = [table_ptr2], 8
//
//     U_prime_hi = U + V * z_hi
//     Load the Tbl_lo(0)
//
(p9)   pmpy2.r k = k, special1 ;;
}
{ .mii
	nop.m 999
	nop.i 999 
	nop.i 999 ;;
}
{ .mii
	nop.m 999
	nop.i 999 
	nop.i 999 ;;
}
{ .mii
	nop.m 999
	nop.i 999 
	nop.i 999 ;;
}
{ .mii
	nop.m 999
	nop.i 999 ;;
(p9)   shladd lookup = lookup, 0x0004, k ;;
}
{ .mmi
(p9)   add table_ptr2 = table_ptr2, lookup ;;
//
//     V_prime =  V - U * z_hi
//
(p9)   ldfd Tbl_hi = [table_ptr2], 8
	nop.i 999 ;;
}
{ .mmf
	nop.m 999
//
//     C_hi = frcpa(1,U_prime_hi)
//
(p9)   ldfs Tbl_lo = [table_ptr2], 8
//
//     z_hi = s exp 1.b_1 b_2 b_3 b_4 1 0 0 0 ... 0
//     Point to beginning of Tbl_hi entries - k = 0.
//
(p0)   fmerge.se z_hi = Q, z_hi ;;
}
{ .mfi
	nop.m 999
(p0)   fma.s1 U_prime_hi = V, z_hi, U
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fnma.s1 V_prime = U, z_hi, V
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   mov A_hi = Tbl_hi
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fsub.s1 U_hold = U, U_prime_hi
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   frcpa.s1 C_hi, p6 = f1, U_prime_hi
	nop.i 999 ;;
}
{ .mfi
(p0)   cmp.eq.unc p7, p6 = 0x00000, swap
(p0)   fmpy.s1 A_hi = s_Y, A_hi
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     poly = wsq * poly
//
(p7)   fadd.s1 sigma = f0, f1
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fma.s1 U_prime_lo = z_hi, V, U_hold
	nop.i 999
}
{ .mfi
	nop.m 999
(p6)   fsub.s1 sigma = f0, f1
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fnma.s1 C_hi_hold = C_hi, U_prime_hi, f1
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     A_lo = A_lo + w_hi
//     A_hi = s_Y * A_hi
//
(p0)   fma.s1 Res_hi = sigma, A_hi, P_hi
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     C_hi_hold = 1 - C_hi * U_prime_hi (1)
//
(p0)   fma.s1 C_hi = C_hi_hold, C_hi, C_hi
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     C_hi = C_hi + C_hi * C_hi_hold    (1)
//
(p0)   fnma.s1 C_hi_hold = C_hi, U_prime_hi, f1
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     C_hi_hold = 1 - C_hi * U_prime_hi (2)
//
(p0)   fma.s1 C_hi = C_hi_hold, C_hi, C_hi
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     C_hi = C_hi + C_hi * C_hi_hold    (2)
//
(p0)   fnma.s1 C_hi_hold = C_hi, U_prime_hi, f1
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     C_hi_hold = 1 - C_hi * U_prime_hi (3)
//
(p0)   fma.s1 C_hi = C_hi_hold, C_hi, C_hi
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     C_hi = C_hi + C_hi * C_hi_hold    (3)
//
(p0)   fmpy.s1 w_hi = V_prime, C_hi
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     w_hi = V_prime * C_hi
//
(p0)   fmpy.s1 wsq = w_hi, w_hi
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fnma.s1 w_lo = w_hi, U_prime_hi, V_prime
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     wsq = w_hi * w_hi
//     w_lo =  = V_prime - w_hi * U_prime_hi
//
(p0)   fma.s1 poly =  wsq, Q_4, Q_3
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fnma.s1 w_lo = w_hi, U_prime_lo, w_lo
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     poly = Q_3 + wsq * Q_4
//     w_lo =  = w_lo - w_hi * U_prime_lo
//
(p0)   fma.s1 poly = wsq, poly, Q_2
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fmpy.s1 w_lo = C_hi, w_lo
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     poly = Q_2 + wsq * poly
//     w_lo =  = w_lo * C_hi
//
(p0)   fma.s1 poly = wsq, poly, Q_1
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fadd.s1 A_lo = Tbl_lo, w_lo
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Result  =  Res_hi + Res_lo * s_Y  (User Supplied Rounding Mode)
//
(p0)   fmpy.s0 Q_1 =  Q_1, Q_1
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     poly = Q_1 + wsq * poly
//     A_lo = Tbl_lo + w_lo
//     swap = xor(swap,sign_X)
//
(p0)   fmpy.s1 poly = wsq, poly
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Is (swap) != 0 ?
//     poly = wsq * poly
//     A_hi = Tbl_hi
//
(p0)   fmpy.s1 poly = w_hi, poly
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     if (PR_1) sigma = -1.0
//     if (PR_2) sigma =  1.0
//
(p0)   fadd.s1 A_lo = A_lo, poly
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     P_hi = s_Y * P_hi
//     A_lo = A_lo + poly
//
(p0)   fadd.s1 A_lo = A_lo, w_hi
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fma.s1 Res_lo = sigma, A_lo, P_lo
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//
//     Res_hi = P_hi + sigma * A_hi
//     Res_lo = P_lo + sigma * A_lo
//
(p0)   fma.s0 Result = Res_lo, s_Y, Res_hi
//
//     Raise inexact.
//
br.ret.sptk   b0 ;;
}
//
//     poly1 = P_5 + zsq * poly1
//     poly2 = zsq * poly2
//
L(ATANL_POLY): 
{ .mmf
(p0)   xor swap = sign_X, swap
	nop.m 999
(p0)   fnma.s1 E_hold = E, U, f1 ;;
}
{ .mfi
	nop.m 999
(p0)   mov A_temp = Q
//
//     poly1 = P_4 + zsq * poly1
//     swap = xor(swap,sign_X)
//
//     sign_X            gr_002
//     swap              gr_004
//     poly1 = poly1 <== Done with poly1
//     poly1 = P_4 + zsq * poly1
//     swap = xor(swap,sign_X)
//
(p0)   cmp.eq.unc p7, p6 = 0x00000, swap
}
{ .mfi
	nop.m 999
(p0)   fmpy.s1 P_hi = s_Y, P_hi
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p6)   fsub.s1 sigma = f0, f1
	nop.i 999
}
{ .mfi
	nop.m 999
(p7)   fadd.s1 sigma = f0, f1
	nop.i 999 ;;
}

// ***********************************************
// ******************** STEP4 ********************
// ***********************************************

{ .mmi
      nop.m 999
(p0)  addl           table_ptr1   = @ltoff(Constants_atan#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 table_ptr1 = [table_ptr1]
      nop.m 999
      nop.i 999
}
;;


{ .mfi
	nop.m 999
(p0)   fma.s1 E = E, E_hold, E
//
//     Following:
//     Iterate 3 times E = E + E*(1.0 - E*U)
//     Also load P_8, P_7, P_6, P_5, P_4
//     E_hold = 1.0 - E * U     (1)
//     A_temp = Q
//
(p0)   add table_ptr1 = 128, table_ptr1 ;;
}
{ .mmf
	nop.m 999
//
//     E = E + E_hold*E         (1)
//     Point to P_8.
//
(p0)   ldfe P_8 = [table_ptr1], -16
//
//     poly = z8*poly1 + poly2  (Typo in writeup)
//     Is (swap) != 0 ?
//
(p0)   fnma.s1 z_lo = A_temp, U, V ;;
}
{ .mmb
	nop.m 999
//
//     E_hold = 1.0 - E * U     (2)
//
(p0)   ldfe P_7 = [table_ptr1], -16
	nop.b 999 ;;
}
{ .mmb
	nop.m 999
//
//     E = E + E_hold*E         (2)
//
(p0)   ldfe P_6 = [table_ptr1], -16
	nop.b 999 ;;
}
{ .mmb
	nop.m 999
//
//     E_hold = 1.0 - E * U     (3)
//
(p0)   ldfe P_5 = [table_ptr1], -16
	nop.b 999 ;;
}
{ .mmf
	nop.m 999
//
//     E = E + E_hold*E         (3)
//
//
// At this point E approximates 1/U to roughly working precision
// z = V*E approximates V/U
//
(p0)   ldfe P_4 = [table_ptr1], -16
(p0)   fnma.s1 E_hold = E, U, f1 ;;
}
{ .mmb
	nop.m 999
//
//     Z =   V * E
//
(p0)   ldfe P_3 = [table_ptr1], -16
	nop.b 999 ;;
}
{ .mmb
	nop.m 999
//
//     zsq = Z * Z
//
(p0)   ldfe P_2 = [table_ptr1], -16
	nop.b 999 ;;
}
{ .mmb
	nop.m 999
//
//     z8 = zsq * zsq
//
(p0)   ldfe P_1 = [table_ptr1], -16
	nop.b 999 ;;
}
{ .mlx
	nop.m 999
(p0)   movl         int_temp = 0x24005
}
{ .mfi
	nop.m 999
(p0)   fma.s1 E = E, E_hold, E
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fnma.s1 E_hold = E, U, f1
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fma.s1 E = E, E_hold, E
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fmpy.s1 Z = V, E
	nop.i 999
}
{ .mfi
	nop.m 999
//
//     z_lo = V - A_temp * U
//     if (PR_2) sigma =  1.0
//
(p0)   fmpy.s1 z_lo = z_lo, E
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fmpy.s1 zsq = Z, Z
	nop.i 999
}
{ .mfi
	nop.m 999
//
//     z_lo = z_lo * E
//     if (PR_1) sigma = -1.0
//
(p0)   fadd.s1 A_hi = A_temp, z_lo
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     z8 = z8 * z8
//
//
//     Now what we want to do is
//     poly1 = P_4 + zsq*(P_5 + zsq*(P_6 + zsq*(P_7 + zsq*P_8)))
//     poly2 = zsq*(P_1 + zsq*(P_2 + zsq*P_3))
//
(p0)   fma.s1 poly1 = zsq, P_8, P_7
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fma.s1 poly2 = zsq, P_3, P_2
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fmpy.s1 z8 = zsq, zsq
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fsub.s1 A_temp = A_temp, A_hi
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     A_lo = Z * poly + z_lo
//
(p0)   fmerge.s     tmp = A_hi, A_hi
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     poly1 = P_7 + zsq * P_8
//     poly2 = P_2 + zsq * P_3
//
(p0)   fma.s1 poly1 = zsq, poly1, P_6
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fma.s1 poly2 = zsq, poly2, P_1
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fmpy.s1 z8 = z8, z8
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fadd.s1 z_lo = A_temp, z_lo
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     poly1 = P_6 + zsq * poly1
//     poly2 = P_2 + zsq * poly2
//
(p0)   fma.s1 poly1 = zsq, poly1, P_5
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fmpy.s1 poly2 = poly2, zsq
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Result  =  Res_hi + Res_lo  (User Supplied Rounding Mode)
//
(p0)   fmpy.s1 P_5 = P_5, P_5
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fma.s1 poly1 = zsq, poly1, P_4
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fma.s1 poly = z8, poly1, poly2
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Fixup added to force inexact later -
//     A_hi = A_temp + z_lo
//     z_lo = (A_temp - A_hi) + z_lo
//
(p0)   fma.s1 A_lo = Z, poly, z_lo
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fadd.s1      A_hi = tmp, A_lo
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fsub.s1      tmp = tmp, A_hi
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fmpy.s1 A_hi = s_Y, A_hi
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fadd.s1      A_lo = tmp, A_lo
	nop.i 999
}
{ .mfi
(p0)   setf.exp     tmp = int_temp
//
//     P_hi = s_Y * P_hi
//     A_hi = s_Y * A_hi
//
(p0)   fma.s1 Res_hi = sigma, A_hi, P_hi
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fclass.m.unc p6,p0 = A_lo, 0x007
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p6)   mov          A_lo = tmp
	nop.i 999
}
{ .mfi
	nop.m 999
//
//     Res_hi = P_hi + sigma * A_hi
//
(p0)   fsub.s1 tmp =  P_hi, Res_hi
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     tmp = P_hi - Res_hi
//
(p0)   fma.s1 tmp = A_hi, sigma, tmp
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fma.s1 sigma =  A_lo, sigma, P_lo
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     tmp   = sigma * A_hi  + tmp
//     sigma = A_lo * sigma  + P_lo
//
(p0)   fma.s1 Res_lo = s_Y, sigma, tmp
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//
//     Res_lo = s_Y * sigma + tmp
//
(p0)   fadd.s0 Result = Res_lo, Res_hi
br.ret.sptk   b0 ;;
}
L(ATANL_NATVAL): 
L(ATANL_UNSUPPORTED): 
L(ATANL_NAN): 
{ .mfb
	nop.m 999
(p0)   fmpy.s0 Result = ArgX,ArgY 
(p0)   br.ret.sptk   b0 ;;
}
L(ATANL_SPECIAL_HANDLING): 
{ .mfi
	nop.m 999
(p0)   fcmp.eq.s0     p0, p6 = f1, ArgY_orig
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)   fcmp.eq.s0     p0, p5 = f1, ArgX_orig
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)   fclass.m.unc p6, p7 = ArgY, 0x007
	nop.i 999
}
{ .mlx
	nop.m 999
(p0)   movl special = 992
}
;;


{ .mmi
      nop.m 999
(p0)  addl           table_ptr1   = @ltoff(Constants_atan#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 table_ptr1 = [table_ptr1]
      nop.m 999
      nop.i 999
}
;;


{ .mib
(p0)   add table_ptr1 = table_ptr1, special
	nop.i 999
(p7)   br.cond.spnt L(ATANL_ArgY_Not_ZERO) ;;
}
{ .mmf
(p0)   ldfd  Result = [table_ptr1], 8
	nop.m 999
(p6)   fclass.m.unc p14, p0 = ArgX, 0x035 ;;
}
{ .mmf
	nop.m 999
(p0)   ldfd  Result_lo = [table_ptr1], -8
(p6)   fclass.m.unc p15, p0 = ArgX, 0x036 ;;
}
{ .mfi
	nop.m 999
(p14)  fmerge.s Result = ArgY, f0
	nop.i 999
}
{ .mfi
	nop.m 999
(p6)   fclass.m.unc p13, p0 = ArgX, 0x007
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p14)  fmerge.s Result_lo = ArgY, f0
	nop.i 999 ;;
}
{ .mfi
(p13)  mov GR_Parameter_TAG = 36 
	nop.f 999
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
//
//     Return sign_Y * 0 when  ArgX > +0
//
(p15)  fmerge.s Result = ArgY, Result
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p15)  fmerge.s Result_lo = ArgY, Result_lo
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
//
//     Return sign_Y * 0 when  ArgX < -0
//
(p0)   fadd.s0 Result = Result, Result_lo
(p13)  br.cond.spnt __libm_error_region ;;
}
{ .mib
	nop.m 999
	nop.i 999
//
//     Call error support funciton for atan(0,0)
//
(p0)    br.ret.sptk   b0 ;;
}
L(ATANL_ArgY_Not_ZERO): 
{ .mfi
	nop.m 999
(p0)   fclass.m.unc p9, p10 = ArgY, 0x023
	nop.i 999 ;;
}
{ .mib
	nop.m 999
	nop.i 999
(p10)  br.cond.spnt  L(ATANL_ArgY_Not_INF) ;;
}
{ .mfi
	nop.m 999
(p9)   fclass.m.unc p6, p0 = ArgX, 0x017
	nop.i 999
}
{ .mfi
	nop.m 999
(p9)   fclass.m.unc p7, p0 = ArgX, 0x021
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p9)   fclass.m.unc p8, p0 = ArgX, 0x022
	nop.i 999 ;;
}
{ .mmi
(p6)   add table_ptr1 =  16, table_ptr1 ;;
(p0)   ldfd Result = [table_ptr1], 8
	nop.i 999 ;;
}
{ .mfi
(p0)   ldfd Result_lo = [table_ptr1], -8
	nop.f 999
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p6)   fmerge.s Result = ArgY, Result
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p6)   fmerge.s Result_lo = ArgY, Result_lo
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p6)    fadd.s0 Result = Result, Result_lo
(p6)    br.ret.sptk   b0 ;;
}
//
//     Load PI/2 and adjust its sign.
//     Return +PI/2 when ArgY = +Inf and ArgX = +/-0 or normal
//     Return -PI/2 when ArgY = -Inf and ArgX = +/-0 or normal
//
{ .mmi
(p7)   add table_ptr1 = 32, table_ptr1 ;;
(p7)   ldfd Result = [table_ptr1], 8
	nop.i 999 ;;
}
{ .mfi
(p7)   ldfd Result_lo = [table_ptr1], -8
	nop.f 999
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p7)   fmerge.s Result = ArgY, Result
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p7)   fmerge.s Result_lo = ArgY, Result_lo
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p7)    fadd.s0 Result = Result, Result_lo
(p7)    br.ret.sptk   b0 ;;
}
//
//     Load PI/4 and adjust its sign.
//     Return +PI/4 when ArgY = +Inf and ArgX = +Inf
//     Return -PI/4 when ArgY = -Inf and ArgX = +Inf
//
{ .mmi
(p8)   add table_ptr1 = 48, table_ptr1 ;;
(p8)   ldfd Result = [table_ptr1], 8
	nop.i 999 ;;
}
{ .mfi
(p8)   ldfd Result_lo = [table_ptr1], -8
	nop.f 999
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p8)   fmerge.s Result = ArgY, Result
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p8)   fmerge.s Result_lo = ArgY, Result_lo
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p8)   fadd.s0 Result = Result, Result_lo
(p8)   br.ret.sptk   b0 ;; 
}
L(ATANL_ArgY_Not_INF): 
{ .mfi
	nop.m 999
//
//     Load PI/4 and adjust its sign.
//     Return +3PI/4 when ArgY = +Inf and ArgX = -Inf
//     Return -3PI/4 when ArgY = -Inf and ArgX = -Inf
//
(p0)  fclass.m.unc p6, p0 = ArgX, 0x007
	nop.i 999
}
{ .mfi
	nop.m 999
(p0)  fclass.m.unc p7, p0 = ArgX, 0x021
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p0)  fclass.m.unc p8, p0 = ArgX, 0x022
	nop.i 999 ;;
}
{ .mmi
(p6)  add table_ptr1 = 16, table_ptr1 ;;
(p6)  ldfd Result = [table_ptr1], 8
	nop.i 999 ;;
}
{ .mfi
(p6)  ldfd Result_lo = [table_ptr1], -8
	nop.f 999
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p6)  fmerge.s Result = ArgY, Result
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p6)  fmerge.s Result_lo = ArgY, Result_lo
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p6)  fadd.s0 Result = Result, Result_lo
(p6)  br.ret.spnt   b0 ;;
}
{ .mfi
	nop.m 999
//
//    return = sign_Y * PI/2 when ArgX = 0
//
(p7)  fmerge.s Result = ArgY, f0
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p7)  fnorm.s0 Result = Result
(p7)  br.ret.spnt   b0 ;;
}
//
//    return = sign_Y * 0 when ArgX = Inf
//
{ .mmi
(p8)  ldfd Result = [table_ptr1], 8 ;;
(p8)  ldfd Result_lo = [table_ptr1], -8
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p8)  fmerge.s Result = ArgY, Result
	nop.i 999 ;;
}
{ .mfi
	nop.m 999
(p8)  fmerge.s Result_lo = ArgY, Result_lo
	nop.i 999 ;;
}
{ .mfb
	nop.m 999
(p8)  fadd.s0 Result = Result, Result_lo
(p8)  br.ret.sptk   b0 ;;
}
//
//    return = sign_Y * PI when ArgX = -Inf
//
.endp atan2l
ASM_SIZE_DIRECTIVE(atan2l)
ASM_SIZE_DIRECTIVE(__atan2l)
ASM_SIZE_DIRECTIVE(__ieee754_atan2l)
 
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
