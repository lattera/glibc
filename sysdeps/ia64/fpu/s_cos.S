.file "sincos.s"

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
// 2/02/00  Initial revision
// 4/02/00  Unwind support added.
// 6/16/00  Updated tables to enforce symmetry
// 8/31/00  Saved 2 cycles in main path, and 9 in other paths.
// 9/20/00  The updated tables regressed to an old version, so reinstated them
// 10/18/00 Changed one table entry to ensure symmetry
// 1/03/01  Improved speed, fixed flag settings for small arguments.

// API
//==============================================================
// double sin( double x);
// double cos( double x);
//
// Overview of operation
//==============================================================
//
// Step 1
// ======
// Reduce x to region -1/2*pi/2^k ===== 0 ===== +1/2*pi/2^k  where k=4
//    divide x by pi/2^k.
//    Multiply by 2^k/pi.
//    nfloat = Round result to integer (round-to-nearest)
//
// r = x -  nfloat * pi/2^k
//    Do this as (x -  nfloat * HIGH(pi/2^k)) - nfloat * LOW(pi/2^k) for increased accuracy.
//    pi/2^k is stored as two numbers that when added make pi/2^k.
//       pi/2^k = HIGH(pi/2^k) + LOW(pi/2^k)
//
// x = (nfloat * pi/2^k) + r
//    r is small enough that we can use a polynomial approximation
//    and is referred to as the reduced argument.
//
// Step 3
// ======
// Take the unreduced part and remove the multiples of 2pi.
// So nfloat = nfloat (with lower k+1 bits cleared) + lower k+1 bits
//
//    nfloat (with lower k+1 bits cleared) is a multiple of 2^(k+1)
//    N * 2^(k+1)
//    nfloat * pi/2^k = N * 2^(k+1) * pi/2^k + (lower k+1 bits) * pi/2^k
//    nfloat * pi/2^k = N * 2 * pi + (lower k+1 bits) * pi/2^k
//    nfloat * pi/2^k = N2pi + M * pi/2^k
//
//
// Sin(x) = Sin((nfloat * pi/2^k) + r)
//        = Sin(nfloat * pi/2^k) * Cos(r) + Cos(nfloat * pi/2^k) * Sin(r)
//
//          Sin(nfloat * pi/2^k) = Sin(N2pi + Mpi/2^k)
//                               = Sin(N2pi)Cos(Mpi/2^k) + Cos(N2pi)Sin(Mpi/2^k)
//                               = Sin(Mpi/2^k)
//
//          Cos(nfloat * pi/2^k) = Cos(N2pi + Mpi/2^k)
//                               = Cos(N2pi)Cos(Mpi/2^k) + Sin(N2pi)Sin(Mpi/2^k)
//                               = Cos(Mpi/2^k)
//
// Sin(x) = Sin(Mpi/2^k) Cos(r) + Cos(Mpi/2^k) Sin(r)
//
//
// Step 4
// ======
// 0 <= M < 2^(k+1)
// There are 2^(k+1) Sin entries in a table.
// There are 2^(k+1) Cos entries in a table.
//
// Get Sin(Mpi/2^k) and Cos(Mpi/2^k) by table lookup.
//
//
// Step 5
// ======
// Calculate Cos(r) and Sin(r) by polynomial approximation.
//
// Cos(r) = 1 + r^2 q1  + r^4 q2 + r^6 q3 + ... = Series for Cos
// Sin(r) = r + r^3 p1  + r^5 p2 + r^7 p3 + ... = Series for Sin
//
// and the coefficients q1, q2, ... and p1, p2, ... are stored in a table
//
//
// Calculate
// Sin(x) = Sin(Mpi/2^k) Cos(r) + Cos(Mpi/2^k) Sin(r)
//
// as follows
//
//    Sm = Sin(Mpi/2^k) and Cm = Cos(Mpi/2^k)
//    rsq = r*r
//
//
//    P = p1 + r^2p2 + r^4p3 + r^6p4
//    Q = q1 + r^2q2 + r^4q3 + r^6q4
//
//       rcub = r * rsq
//       Sin(r) = r + rcub * P
//              = r + r^3p1  + r^5p2 + r^7p3 + r^9p4 + ... = Sin(r)
//
//            The coefficients are not exactly these values, but almost.
//
//            p1 = -1/6  = -1/3!
//            p2 = 1/120 =  1/5!
//            p3 = -1/5040 = -1/7!
//            p4 = 1/362889 = 1/9!
//
//       P =  r + rcub * P
//
//    Answer = Sm Cos(r) + Cm P
//
//       Cos(r) = 1 + rsq Q
//       Cos(r) = 1 + r^2 Q
//       Cos(r) = 1 + r^2 (q1 + r^2q2 + r^4q3 + r^6q4)
//       Cos(r) = 1 + r^2q1 + r^4q2 + r^6q3 + r^8q4 + ...
//
//       Sm Cos(r) = Sm(1 + rsq Q)
//       Sm Cos(r) = Sm + Sm rsq Q
//       Sm Cos(r) = Sm + s_rsq Q
//       Q         = Sm + s_rsq Q
//
// Then,
//
//    Answer = Q + Cm P

#include "libm_support.h"

// Registers used
//==============================================================
// general input registers:
// r14 -> r19
// r32 -> r45

// predicate registers used:
// p6 -> p14

// floating-point registers used
// f9 -> f15
// f32 -> f61

// Assembly macros
//==============================================================
sind_NORM_f8                 = f9
sind_W                       = f10
sind_int_Nfloat              = f11
sind_Nfloat                  = f12

sind_r                       = f13
sind_rsq                     = f14
sind_rcub                    = f15

sind_Inv_Pi_by_16            = f32
sind_Pi_by_16_hi             = f33
sind_Pi_by_16_lo             = f34

sind_Inv_Pi_by_64            = f35
sind_Pi_by_64_hi             = f36
sind_Pi_by_64_lo             = f37

sind_Sm                      = f38
sind_Cm                      = f39

sind_P1                      = f40
sind_Q1                      = f41
sind_P2                      = f42
sind_Q2                      = f43
sind_P3                      = f44
sind_Q3                      = f45
sind_P4                      = f46
sind_Q4                      = f47

sind_P_temp1                 = f48
sind_P_temp2                 = f49

sind_Q_temp1                 = f50
sind_Q_temp2                 = f51

sind_P                       = f52
sind_Q                       = f53

sind_srsq                    = f54

sind_SIG_INV_PI_BY_16_2TO61  = f55
sind_RSHF_2TO61              = f56
sind_RSHF                    = f57
sind_2TOM61                  = f58
sind_NFLOAT                  = f59
sind_W_2TO61_RSH             = f60

fp_tmp                       = f61

/////////////////////////////////////////////////////////////

sind_AD_1                    = r33
sind_AD_2                    = r34
sind_exp_limit               = r35
sind_r_signexp               = r36
sind_AD_beta_table           = r37
sind_r_sincos                = r38

sind_r_exp                   = r39
sind_r_17_ones               = r40

sind_GR_sig_inv_pi_by_16     = r14
sind_GR_rshf_2to61           = r15
sind_GR_rshf                 = r16
sind_GR_exp_2tom61           = r17
sind_GR_n                    = r18
sind_GR_m                    = r19
sind_GR_32m                  = r19

gr_tmp                       = r41
GR_SAVE_PFS                  = r41
GR_SAVE_B0                   = r42
GR_SAVE_GP                   = r43


#ifdef _LIBC
.rodata
#else
.data
#endif

.align 16
double_sind_pi:
ASM_TYPE_DIRECTIVE(double_sind_pi,@object)
//   data8 0xA2F9836E4E44152A, 0x00004001 // 16/pi (significand loaded w/ setf)
//         c90fdaa22168c234
   data8 0xC90FDAA22168C234, 0x00003FFC // pi/16 hi
//         c4c6628b80dc1cd1  29024e088a
   data8 0xC4C6628B80DC1CD1, 0x00003FBC // pi/16 lo
ASM_SIZE_DIRECTIVE(double_sind_pi)

double_sind_pq_k4:
ASM_TYPE_DIRECTIVE(double_sind_pq_k4,@object)
   data8 0x3EC71C963717C63A // P4
   data8 0x3EF9FFBA8F191AE6 // Q4
   data8 0xBF2A01A00F4E11A8 // P3
   data8 0xBF56C16C05AC77BF // Q3
   data8 0x3F8111111110F167 // P2
   data8 0x3FA555555554DD45 // Q2
   data8 0xBFC5555555555555 // P1
   data8 0xBFDFFFFFFFFFFFFC // Q1
ASM_SIZE_DIRECTIVE(double_sind_pq_k4)


double_sin_cos_beta_k4:
ASM_TYPE_DIRECTIVE(double_sin_cos_beta_k4,@object)
data8 0x0000000000000000 , 0x00000000 // sin( 0 pi/16)  S0
data8 0x8000000000000000 , 0x00003fff // cos( 0 pi/16)  C0

data8 0xc7c5c1e34d3055b3 , 0x00003ffc // sin( 1 pi/16)  S1
data8 0xfb14be7fbae58157 , 0x00003ffe // cos( 1 pi/16)  C1

data8 0xc3ef1535754b168e , 0x00003ffd // sin( 2 pi/16)  S2
data8 0xec835e79946a3146 , 0x00003ffe // cos( 2 pi/16)  C2

data8 0x8e39d9cd73464364 , 0x00003ffe // sin( 3 pi/16)  S3
data8 0xd4db3148750d181a , 0x00003ffe // cos( 3 pi/16)  C3

data8 0xb504f333f9de6484 , 0x00003ffe // sin( 4 pi/16)  S4
data8 0xb504f333f9de6484 , 0x00003ffe // cos( 4 pi/16)  C4


data8 0xd4db3148750d181a , 0x00003ffe // sin( 5 pi/16)  C3
data8 0x8e39d9cd73464364 , 0x00003ffe // cos( 5 pi/16)  S3

data8 0xec835e79946a3146 , 0x00003ffe // sin( 6 pi/16)  C2
data8 0xc3ef1535754b168e , 0x00003ffd // cos( 6 pi/16)  S2

data8 0xfb14be7fbae58157 , 0x00003ffe // sin( 7 pi/16)  C1
data8 0xc7c5c1e34d3055b3 , 0x00003ffc // cos( 7 pi/16)  S1

data8 0x8000000000000000 , 0x00003fff // sin( 8 pi/16)  C0
data8 0x0000000000000000 , 0x00000000 // cos( 8 pi/16)  S0


data8 0xfb14be7fbae58157 , 0x00003ffe // sin( 9 pi/16)  C1
data8 0xc7c5c1e34d3055b3 , 0x0000bffc // cos( 9 pi/16)  -S1

data8 0xec835e79946a3146 , 0x00003ffe // sin(10 pi/16)  C2
data8 0xc3ef1535754b168e , 0x0000bffd // cos(10 pi/16)  -S2

data8 0xd4db3148750d181a , 0x00003ffe // sin(11 pi/16)  C3
data8 0x8e39d9cd73464364 , 0x0000bffe // cos(11 pi/16)  -S3

data8 0xb504f333f9de6484 , 0x00003ffe // sin(12 pi/16)  S4
data8 0xb504f333f9de6484 , 0x0000bffe // cos(12 pi/16)  -S4


data8 0x8e39d9cd73464364 , 0x00003ffe // sin(13 pi/16) S3
data8 0xd4db3148750d181a , 0x0000bffe // cos(13 pi/16) -C3

data8 0xc3ef1535754b168e , 0x00003ffd // sin(14 pi/16) S2
data8 0xec835e79946a3146 , 0x0000bffe // cos(14 pi/16) -C2

data8 0xc7c5c1e34d3055b3 , 0x00003ffc // sin(15 pi/16) S1
data8 0xfb14be7fbae58157 , 0x0000bffe // cos(15 pi/16) -C1

data8 0x0000000000000000 , 0x00000000 // sin(16 pi/16) S0
data8 0x8000000000000000 , 0x0000bfff // cos(16 pi/16) -C0


data8 0xc7c5c1e34d3055b3 , 0x0000bffc // sin(17 pi/16) -S1
data8 0xfb14be7fbae58157 , 0x0000bffe // cos(17 pi/16) -C1

data8 0xc3ef1535754b168e , 0x0000bffd // sin(18 pi/16) -S2
data8 0xec835e79946a3146 , 0x0000bffe // cos(18 pi/16) -C2

data8 0x8e39d9cd73464364 , 0x0000bffe // sin(19 pi/16) -S3
data8 0xd4db3148750d181a , 0x0000bffe // cos(19 pi/16) -C3

data8 0xb504f333f9de6484 , 0x0000bffe // sin(20 pi/16) -S4
data8 0xb504f333f9de6484 , 0x0000bffe // cos(20 pi/16) -S4


data8 0xd4db3148750d181a , 0x0000bffe // sin(21 pi/16) -C3
data8 0x8e39d9cd73464364 , 0x0000bffe // cos(21 pi/16) -S3

data8 0xec835e79946a3146 , 0x0000bffe // sin(22 pi/16) -C2
data8 0xc3ef1535754b168e , 0x0000bffd // cos(22 pi/16) -S2

data8 0xfb14be7fbae58157 , 0x0000bffe // sin(23 pi/16) -C1
data8 0xc7c5c1e34d3055b3 , 0x0000bffc // cos(23 pi/16) -S1

data8 0x8000000000000000 , 0x0000bfff // sin(24 pi/16) -C0
data8 0x0000000000000000 , 0x00000000 // cos(24 pi/16) S0


data8 0xfb14be7fbae58157 , 0x0000bffe // sin(25 pi/16) -C1
data8 0xc7c5c1e34d3055b3 , 0x00003ffc // cos(25 pi/16) S1

data8 0xec835e79946a3146 , 0x0000bffe // sin(26 pi/16) -C2
data8 0xc3ef1535754b168e , 0x00003ffd // cos(26 pi/16) S2

data8 0xd4db3148750d181a , 0x0000bffe // sin(27 pi/16) -C3
data8 0x8e39d9cd73464364 , 0x00003ffe // cos(27 pi/16) S3

data8 0xb504f333f9de6484 , 0x0000bffe // sin(28 pi/16) -S4
data8 0xb504f333f9de6484 , 0x00003ffe // cos(28 pi/16) S4


data8 0x8e39d9cd73464364 , 0x0000bffe // sin(29 pi/16) -S3
data8 0xd4db3148750d181a , 0x00003ffe // cos(29 pi/16) C3

data8 0xc3ef1535754b168e , 0x0000bffd // sin(30 pi/16) -S2
data8 0xec835e79946a3146 , 0x00003ffe // cos(30 pi/16) C2

data8 0xc7c5c1e34d3055b3 , 0x0000bffc // sin(31 pi/16) -S1
data8 0xfb14be7fbae58157 , 0x00003ffe // cos(31 pi/16) C1

data8 0x0000000000000000 , 0x00000000 // sin(32 pi/16) S0
data8 0x8000000000000000 , 0x00003fff // cos(32 pi/16) C0
ASM_SIZE_DIRECTIVE(double_sin_cos_beta_k4)

.align 32
.global sin#
.global cos#
#ifdef _LIBC
.global __sin#
.global __cos#
#endif

////////////////////////////////////////////////////////
// There are two entry points: sin and cos


// If from sin, p8 is true
// If from cos, p9 is true

.section .text
.proc  sin#
#ifdef _LIBC
.proc  __sin#
#endif
.align 32

sin:
#ifdef _LIBC
__sin:
#endif

{ .mlx
      alloc          r32=ar.pfs,1,13,0,0
      movl sind_GR_sig_inv_pi_by_16 = 0xA2F9836E4E44152A // significand of 16/pi
}
{ .mlx
      addl           sind_AD_1   = @ltoff(double_sind_pi), gp
      movl sind_GR_rshf_2to61 = 0x47b8000000000000 // 1.1000 2^(63+63-2)
}
;;

{ .mfi
      ld8 sind_AD_1 = [sind_AD_1]
      fnorm     sind_NORM_f8  = f8
      cmp.eq     p8,p9         = r0, r0
}
{ .mib
      mov sind_GR_exp_2tom61 = 0xffff-61 // exponent of scaling factor 2^-61
      mov            sind_r_sincos = 0x0
      br.cond.sptk   L(SIND_SINCOS)
}
;;

.endp sin
ASM_SIZE_DIRECTIVE(sin)


.section .text
.proc  cos#
#ifdef _LIBC
.proc  __cos#
#endif
.align 32
cos:
#ifdef _LIBC
__cos:
#endif

{ .mlx
      alloc          r32=ar.pfs,1,13,0,0
      movl sind_GR_sig_inv_pi_by_16 = 0xA2F9836E4E44152A // significand of 16/pi
}
{ .mlx
      addl           sind_AD_1   = @ltoff(double_sind_pi), gp
      movl sind_GR_rshf_2to61 = 0x47b8000000000000 // 1.1000 2^(63+63-2)
}
;;

{ .mfi
      ld8 sind_AD_1 = [sind_AD_1]
      fnorm.s1     sind_NORM_f8  = f8
      cmp.eq     p9,p8         = r0, r0
}
{ .mib
      mov sind_GR_exp_2tom61 = 0xffff-61 // exponent of scaling factor 2^-61
      mov            sind_r_sincos = 0x8
      br.cond.sptk   L(SIND_SINCOS)
}
;;


////////////////////////////////////////////////////////
// All entry points end up here.
// If from sin, sind_r_sincos is 0 and p8 is true
// If from cos, sind_r_sincos is 8 = 2^(k-1) and p9 is true
// We add sind_r_sincos to N

L(SIND_SINCOS):


// Form two constants we need
//  16/pi * 2^-2 * 2^63, scaled by 2^61 since we just loaded the significand
//  1.1000...000 * 2^(63+63-2) to right shift int(W) into the low significand
// fcmp used to set denormal, and invalid on snans
{ .mfi
      setf.sig sind_SIG_INV_PI_BY_16_2TO61 = sind_GR_sig_inv_pi_by_16
      fcmp.eq.s0 p12,p0=f8,f0
      mov       sind_r_17_ones    = 0x1ffff
}
{ .mlx
      setf.d sind_RSHF_2TO61 = sind_GR_rshf_2to61
      movl sind_GR_rshf = 0x43e8000000000000 // 1.1000 2^63 for right shift
}
;;

// Form another constant
//  2^-61 for scaling Nfloat
// 0x10009 is register_bias + 10.
// So if f8 > 2^10 = Gamma, go to DBX
{ .mfi
      setf.exp sind_2TOM61 = sind_GR_exp_2tom61
      fclass.m  p13,p0 = f8, 0x23           // Test for x inf
      mov       sind_exp_limit = 0x10009
}
;;

// Load the two pieces of pi/16
// Form another constant
//  1.1000...000 * 2^63, the right shift constant
{ .mmf
      ldfe      sind_Pi_by_16_hi  = [sind_AD_1],16
      setf.d sind_RSHF = sind_GR_rshf
      fclass.m  p14,p0 = f8, 0xc3           // Test for x nan
}
;;

{ .mfi
      ldfe      sind_Pi_by_16_lo  = [sind_AD_1],16
(p13) frcpa.s0 f8,p12=f0,f0               // force qnan indef for x=inf
      addl gr_tmp = -1,r0
}
{ .mfb
      addl           sind_AD_beta_table   = @ltoff(double_sin_cos_beta_k4), gp
      nop.f 999
(p13) br.ret.spnt    b0 ;;                // Exit for x=inf
}

// Start loading P, Q coefficients
// SIN(0)
{ .mfi
      ldfpd      sind_P4,sind_Q4 = [sind_AD_1],16
(p8)  fclass.m.unc  p6,p0 = f8, 0x07      // Test for sin(0)
      nop.i 999
}
{ .mfb
      addl           sind_AD_beta_table   = @ltoff(double_sin_cos_beta_k4), gp
(p14) fma.d f8=f8,f1,f0                   // qnan for x=nan
(p14) br.ret.spnt    b0 ;;                // Exit for x=nan
}


// COS(0)
{ .mfi
      getf.exp  sind_r_signexp    = f8
(p9)  fclass.m.unc  p7,p0 = f8, 0x07      // Test for sin(0)
      nop.i 999
}
{ .mfi
      ld8 sind_AD_beta_table = [sind_AD_beta_table]
      nop.f 999
      nop.i 999 ;;
}

{ .mmb
      ldfpd      sind_P3,sind_Q3 = [sind_AD_1],16
      setf.sig fp_tmp = gr_tmp // Create constant such that fmpy sets inexact
(p6)  br.ret.spnt    b0 ;;
}

{ .mfb
      and       sind_r_exp = sind_r_17_ones, sind_r_signexp
(p7)  fmerge.s      f8 = f1,f1
(p7)  br.ret.spnt    b0 ;;
}

// p10 is true if we must call routines to handle larger arguments
// p10 is true if f8 exp is > 0x10009

{ .mfi
      ldfpd      sind_P2,sind_Q2 = [sind_AD_1],16
      nop.f 999
      cmp.ge  p10,p0 = sind_r_exp,sind_exp_limit
}
;;

// sind_W          = x * sind_Inv_Pi_by_16
// Multiply x by scaled 16/pi and add large const to shift integer part of W to
//   rightmost bits of significand
{ .mfi
      ldfpd      sind_P1,sind_Q1 = [sind_AD_1]
      fma.s1 sind_W_2TO61_RSH = sind_NORM_f8,sind_SIG_INV_PI_BY_16_2TO61,sind_RSHF_2TO61
      nop.i 999
}
{ .mbb
(p10) cmp.ne.unc p11,p12=sind_r_sincos,r0  // p11 call __libm_cos_double_dbx
                                           // p12 call __libm_sin_double_dbx
(p11) br.cond.spnt L(COSD_DBX)
(p12) br.cond.spnt L(SIND_DBX)
}
;;


// sind_NFLOAT = Round_Int_Nearest(sind_W)
// This is done by scaling back by 2^-61 and subtracting the shift constant
{ .mfi
      nop.m 999
      fms.s1 sind_NFLOAT = sind_W_2TO61_RSH,sind_2TOM61,sind_RSHF
      nop.i 999 ;;
}


// get N = (int)sind_int_Nfloat
{ .mfi
      getf.sig  sind_GR_n = sind_W_2TO61_RSH
      nop.f 999
      nop.i 999 ;;
}

// Add 2^(k-1) (which is in sind_r_sincos) to N
// sind_r          = -sind_Nfloat * sind_Pi_by_16_hi + x
// sind_r          = sind_r -sind_Nfloat * sind_Pi_by_16_lo
{ .mfi
      add       sind_GR_n = sind_GR_n, sind_r_sincos
      fnma.s1  sind_r      = sind_NFLOAT, sind_Pi_by_16_hi, sind_NORM_f8
      nop.i 999 ;;
}


// Get M (least k+1 bits of N)
{ .mmi
      and       sind_GR_m = 0x1f,sind_GR_n ;;
      nop.m 999
      shl       sind_GR_32m = sind_GR_m,5 ;;
}

// Add 32*M to address of sin_cos_beta table
{ .mmi
      add       sind_AD_2 = sind_GR_32m, sind_AD_beta_table
      nop.m 999
      nop.i 999 ;;
}

{ .mfi
      ldfe      sind_Sm = [sind_AD_2],16
(p8)  fclass.m.unc p10,p0=f8,0x0b  // If sin, note denormal input to set uflow
      nop.i 999 ;;
}

{ .mfi
      ldfe      sind_Cm = [sind_AD_2]
      fnma.s1  sind_r      = sind_NFLOAT, sind_Pi_by_16_lo,  sind_r
      nop.i 999 ;;
}

// get rsq
{ .mfi
      nop.m 999
      fma.s1   sind_rsq  = sind_r, sind_r,   f0
      nop.i 999
}
{ .mfi
      nop.m 999
      fmpy.s0  fp_tmp = fp_tmp,fp_tmp // fmpy forces inexact flag
      nop.i 999 ;;
}

// form P and Q series
{ .mfi
      nop.m 999
      fma.s1      sind_P_temp1 = sind_rsq, sind_P4, sind_P3
      nop.i 999
}

{ .mfi
      nop.m 999
      fma.s1      sind_Q_temp1 = sind_rsq, sind_Q4, sind_Q3
      nop.i 999 ;;
}

// get rcube and sm*rsq
{ .mfi
      nop.m 999
      fmpy.s1     sind_srsq    = sind_Sm,sind_rsq
      nop.i 999
}

{ .mfi
      nop.m 999
      fmpy.s1     sind_rcub    = sind_r, sind_rsq
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
      fma.s1      sind_Q_temp2 = sind_rsq, sind_Q_temp1, sind_Q2
      nop.i 999
}

{ .mfi
      nop.m 999
      fma.s1      sind_P_temp2 = sind_rsq, sind_P_temp1, sind_P2
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
      fma.s1      sind_Q       = sind_rsq, sind_Q_temp2, sind_Q1
      nop.i 999
}

{ .mfi
      nop.m 999
      fma.s1      sind_P       = sind_rsq, sind_P_temp2, sind_P1
      nop.i 999 ;;
}

// Get final P and Q
{ .mfi
      nop.m 999
      fma.s1   sind_Q = sind_srsq,sind_Q, sind_Sm
      nop.i 999
}

{ .mfi
      nop.m 999
      fma.s1   sind_P = sind_rcub,sind_P, sind_r
      nop.i 999 ;;
}

// If sin(denormal), force inexact to be set
{ .mfi
      nop.m 999
(p10) fmpy.d.s0 fp_tmp = f8,f8
      nop.i 999 ;;
}

// Final calculation
{ .mfb
      nop.m 999
      fma.d    f8     = sind_Cm, sind_P, sind_Q
      br.ret.sptk    b0 ;;
}
.endp cos#
ASM_SIZE_DIRECTIVE(cos#)



.proc __libm_callout_1s
__libm_callout_1s:
L(SIND_DBX):
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
{ .mib
      nop.m 999
      nop.i 999
      br.call.sptk.many   b0=__libm_sin_double_dbx# ;;
}
;;


{ .mfi
       mov gp        = GR_SAVE_GP
       nop.f  999
       mov b0        = GR_SAVE_B0
}
;;

{ .mib
      nop.m 999
      mov ar.pfs    = GR_SAVE_PFS
      br.ret.sptk     b0 ;;
}
.endp  __libm_callout_1s
ASM_SIZE_DIRECTIVE(__libm_callout_1s)


.proc __libm_callout_1c
__libm_callout_1c:
L(COSD_DBX):
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
{ .mib
      nop.m 999
      nop.i 999
      br.call.sptk.many   b0=__libm_cos_double_dbx# ;;
}
;;


{ .mfi
       mov gp        = GR_SAVE_GP
       nop.f  999
       mov b0        = GR_SAVE_B0
}
;;

{ .mib
      nop.m 999
      mov ar.pfs    = GR_SAVE_PFS
      br.ret.sptk     b0 ;;
}
.endp  __libm_callout_1c
ASM_SIZE_DIRECTIVE(__libm_callout_1c)


// ====================================================================
// ====================================================================

// These functions calculate the sin and cos for inputs
// greater than 2^10
// __libm_sin_double_dbx# and __libm_cos_double_dbx#

// *********************************************************************
// *********************************************************************
//
// Function:   Combined sin(x) and cos(x), where
//
//             sin(x) = sine(x), for double precision x values
//             cos(x) = cosine(x), for double precision x values
//
// *********************************************************************
//
// Accuracy:       Within .7 ulps for 80-bit floating point values
//                 Very accurate for double precision values
//
// *********************************************************************
//
// Resources Used:
//
//    Floating-Point Registers: f8 (Input and Return Value)
//                              f32-f99
//
//    General Purpose Registers:
//      r32-r43
//      r44-r45 (Used to pass arguments to pi_by_2 reduce routine)
//
//    Predicate Registers:      p6-p13
//
// *********************************************************************
//
//  IEEE Special Conditions:
//
//    Denormal  fault raised on denormal inputs
//    Overflow exceptions do not occur
//    Underflow exceptions raised when appropriate for sin
//    (No specialized error handling for this routine)
//    Inexact raised when appropriate by algorithm
//
//    sin(SNaN) = QNaN
//    sin(QNaN) = QNaN
//    sin(inf) = QNaN
//    sin(+/-0) = +/-0
//    cos(inf) = QNaN
//    cos(SNaN) = QNaN
//    cos(QNaN) = QNaN
//    cos(0) = 1
//
// *********************************************************************
//
//  Mathematical Description
//  ========================
//
//  The computation of FSIN and FCOS is best handled in one piece of
//  code. The main reason is that given any argument Arg, computation
//  of trigonometric functions first calculate N and an approximation
//  to alpha where
//
//  Arg = N pi/2 + alpha, |alpha| <= pi/4.
//
//  Since
//
//  cos( Arg ) = sin( (N+1) pi/2 + alpha ),
//
//  therefore, the code for computing sine will produce cosine as long
//  as 1 is added to N immediately after the argument reduction
//  process.
//
//  Let M = N if sine
//      N+1 if cosine.
//
//  Now, given
//
//  Arg = M pi/2  + alpha, |alpha| <= pi/4,
//
//  let I = M mod 4, or I be the two lsb of M when M is represented
//  as 2's complement. I = [i_0 i_1]. Then
//
//  sin( Arg ) = (-1)^i_0  sin( alpha )	if i_1 = 0,
//             = (-1)^i_0  cos( alpha )     if i_1 = 1.
//
//  For example:
//       if M = -1, I = 11
//         sin ((-pi/2 + alpha) = (-1) cos (alpha)
//       if M = 0, I = 00
//         sin (alpha) = sin (alpha)
//       if M = 1, I = 01
//         sin (pi/2 + alpha) = cos (alpha)
//       if M = 2, I = 10
//         sin (pi + alpha) = (-1) sin (alpha)
//       if M = 3, I = 11
//         sin ((3/2)pi + alpha) = (-1) cos (alpha)
//
//  The value of alpha is obtained by argument reduction and
//  represented by two working precision numbers r and c where
//
//  alpha =  r  +  c     accurately.
//
//  The reduction method is described in a previous write up.
//  The argument reduction scheme identifies 4 cases. For Cases 2
//  and 4, because |alpha| is small, sin(r+c) and cos(r+c) can be
//  computed very easily by 2 or 3 terms of the Taylor series
//  expansion as follows:
//
//  Case 2:
//  -------
//
//  sin(r + c) = r + c - r^3/6	accurately
//  cos(r + c) = 1 - 2^(-67)	accurately
//
//  Case 4:
//  -------
//
//  sin(r + c) = r + c - r^3/6 + r^5/120	accurately
//  cos(r + c) = 1 - r^2/2 + r^4/24		accurately
//
//  The only cases left are Cases 1 and 3 of the argument reduction
//  procedure. These two cases will be merged since after the
//  argument is reduced in either cases, we have the reduced argument
//  represented as r + c and that the magnitude |r + c| is not small
//  enough to allow the usage of a very short approximation.
//
//  The required calculation is either
//
//  sin(r + c)  =  sin(r)  +  correction,  or
//  cos(r + c)  =  cos(r)  +  correction.
//
//  Specifically,
//
//	sin(r + c) = sin(r) + c sin'(r) + O(c^2)
//		   = sin(r) + c cos (r) + O(c^2)
//		   = sin(r) + c(1 - r^2/2)  accurately.
//  Similarly,
//
//	cos(r + c) = cos(r) - c sin(r) + O(c^2)
//		   = cos(r) - c(r - r^3/6)  accurately.
//
//  We therefore concentrate on accurately calculating sin(r) and
//  cos(r) for a working-precision number r, |r| <= pi/4 to within
//  0.1% or so.
//
//  The greatest challenge of this task is that the second terms of
//  the Taylor series
//
//	r - r^3/3! + r^r/5! - ...
//
//  and
//
//	1 - r^2/2! + r^4/4! - ...
//
//  are not very small when |r| is close to pi/4 and the rounding
//  errors will be a concern if simple polynomial accumulation is
//  used. When |r| < 2^-3, however, the second terms will be small
//  enough (6 bits or so of right shift) that a normal Horner
//  recurrence suffices. Hence there are two cases that we consider
//  in the accurate computation of sin(r) and cos(r), |r| <= pi/4.
//
//  Case small_r: |r| < 2^(-3)
//  --------------------------
//
//  Since Arg = M pi/4 + r + c accurately, and M mod 4 is [i_0 i_1],
//  we have
//
//	sin(Arg) = (-1)^i_0 * sin(r + c)	if i_1 = 0
//		 = (-1)^i_0 * cos(r + c) 	if i_1 = 1
//
//  can be accurately approximated by
//
//  sin(Arg) = (-1)^i_0 * [sin(r) + c]	if i_1 = 0
//           = (-1)^i_0 * [cos(r) - c*r] if i_1 = 1
//
//  because |r| is small and thus the second terms in the correction
//  are unneccessary.
//
//  Finally, sin(r) and cos(r) are approximated by polynomials of
//  moderate lengths.
//
//  sin(r) =  r + S_1 r^3 + S_2 r^5 + ... + S_5 r^11
//  cos(r) =  1 + C_1 r^2 + C_2 r^4 + ... + C_5 r^10
//
//  We can make use of predicates to selectively calculate
//  sin(r) or cos(r) based on i_1.
//
//  Case normal_r: 2^(-3) <= |r| <= pi/4
//  ------------------------------------
//
//  This case is more likely than the previous one if one considers
//  r to be uniformly distributed in [-pi/4 pi/4]. Again,
//
//  sin(Arg) = (-1)^i_0 * sin(r + c)	if i_1 = 0
//           = (-1)^i_0 * cos(r + c) 	if i_1 = 1.
//
//  Because |r| is now larger, we need one extra term in the
//  correction. sin(Arg) can be accurately approximated by
//
//  sin(Arg) = (-1)^i_0 * [sin(r) + c(1-r^2/2)]      if i_1 = 0
//           = (-1)^i_0 * [cos(r) - c*r*(1 - r^2/6)]    i_1 = 1.
//
//  Finally, sin(r) and cos(r) are approximated by polynomials of
//  moderate lengths.
//
//	sin(r) =  r + PP_1_hi r^3 + PP_1_lo r^3 +
//	              PP_2 r^5 + ... + PP_8 r^17
//
//	cos(r) =  1 + QQ_1 r^2 + QQ_2 r^4 + ... + QQ_8 r^16
//
//  where PP_1_hi is only about 16 bits long and QQ_1 is -1/2.
//  The crux in accurate computation is to calculate
//
//  r + PP_1_hi r^3   or  1 + QQ_1 r^2
//
//  accurately as two pieces: U_hi and U_lo. The way to achieve this
//  is to obtain r_hi as a 10 sig. bit number that approximates r to
//  roughly 8 bits or so of accuracy. (One convenient way is
//
//  r_hi := frcpa( frcpa( r ) ).)
//
//  This way,
//
//	r + PP_1_hi r^3 =  r + PP_1_hi r_hi^3 +
//	                        PP_1_hi (r^3 - r_hi^3)
//		        =  [r + PP_1_hi r_hi^3]  +
//			   [PP_1_hi (r - r_hi)
//			      (r^2 + r_hi r + r_hi^2) ]
//		        =  U_hi  +  U_lo
//
//  Since r_hi is only 10 bit long and PP_1_hi is only 16 bit long,
//  PP_1_hi * r_hi^3 is only at most 46 bit long and thus computed
//  exactly. Furthermore, r and PP_1_hi r_hi^3 are of opposite sign
//  and that there is no more than 8 bit shift off between r and
//  PP_1_hi * r_hi^3. Hence the sum, U_hi, is representable and thus
//  calculated without any error. Finally, the fact that
//
//	|U_lo| <= 2^(-8) |U_hi|
//
//  says that U_hi + U_lo is approximating r + PP_1_hi r^3 to roughly
//  8 extra bits of accuracy.
//
//  Similarly,
//
//	1 + QQ_1 r^2  =  [1 + QQ_1 r_hi^2]  +
//	                    [QQ_1 (r - r_hi)(r + r_hi)]
//		      =  U_hi  +  U_lo.
//
//  Summarizing, we calculate r_hi = frcpa( frcpa( r ) ).
//
//  If i_1 = 0, then
//
//    U_hi := r + PP_1_hi * r_hi^3
//    U_lo := PP_1_hi * (r - r_hi) * (r^2 + r*r_hi + r_hi^2)
//    poly := PP_1_lo r^3 + PP_2 r^5 + ... + PP_8 r^17
//    correction := c * ( 1 + C_1 r^2 )
//
//  Else ...i_1 = 1
//
//    U_hi := 1 + QQ_1 * r_hi * r_hi
//    U_lo := QQ_1 * (r - r_hi) * (r + r_hi)
//    poly := QQ_2 * r^4 + QQ_3 * r^6 + ... + QQ_8 r^16
//    correction := -c * r * (1 + S_1 * r^2)
//
//  End
//
//  Finally,
//
//	V := poly + ( U_lo + correction )
//
//                 /    U_hi  +  V         if i_0 = 0
//	result := |
//                 \  (-U_hi) -  V         if i_0 = 1
//
//  It is important that in the last step, negation of U_hi is
//  performed prior to the subtraction which is to be performed in
//  the user-set rounding mode.
//
//
//  Algorithmic Description
//  =======================
//
//  The argument reduction algorithm is tightly integrated into FSIN
//  and FCOS which share the same code. The following is complete and
//  self-contained. The argument reduction description given
//  previously is repeated below.
//
//
//  Step 0. Initialization.
//
//   If FSIN is invoked, set N_inc := 0; else if FCOS is invoked,
//   set N_inc := 1.
//
//  Step 1. Check for exceptional and special cases.
//
//   * If Arg is +-0, +-inf, NaN, NaT, go to Step 10 for special
//     handling.
//   * If |Arg| < 2^24, go to Step 2 for reduction of moderate
//     arguments. This is the most likely case.
//   * If |Arg| < 2^63, go to Step 8 for pre-reduction of large
//     arguments.
//   * If |Arg| >= 2^63, go to Step 10 for special handling.
//
//  Step 2. Reduction of moderate arguments.
//
//  If |Arg| < pi/4 	...quick branch
//     N_fix := N_inc	(integer)
//     r     := Arg
//     c     := 0.0
//     Branch to Step 4, Case_1_complete
//  Else 		...cf. argument reduction
//     N     := Arg * two_by_PI	(fp)
//     N_fix := fcvt.fx( N )	(int)
//     N     := fcvt.xf( N_fix )
//     N_fix := N_fix + N_inc
//     s     := Arg - N * P_1	(first piece of pi/2)
//     w     := -N * P_2	(second piece of pi/2)
//
//     If |s| >= 2^(-33)
//        go to Step 3, Case_1_reduce
//     Else
//        go to Step 7, Case_2_reduce
//     Endif
//  Endif
//
//  Step 3. Case_1_reduce.
//
//  r := s + w
//  c := (s - r) + w	...observe order
//
//  Step 4. Case_1_complete
//
//  ...At this point, the reduced argument alpha is
//  ...accurately represented as r + c.
//  If |r| < 2^(-3), go to Step 6, small_r.
//
//  Step 5. Normal_r.
//
//  Let [i_0 i_1] by the 2 lsb of N_fix.
//  FR_rsq  := r * r
//  r_hi := frcpa( frcpa( r ) )
//  r_lo := r - r_hi
//
//  If i_1 = 0, then
//    poly := r*FR_rsq*(PP_1_lo + FR_rsq*(PP_2 + ... FR_rsq*PP_8))
//    U_hi := r + PP_1_hi*r_hi*r_hi*r_hi	...any order
//    U_lo := PP_1_hi*r_lo*(r*r + r*r_hi + r_hi*r_hi)
//    correction := c + c*C_1*FR_rsq		...any order
//  Else
//    poly := FR_rsq*FR_rsq*(QQ_2 + FR_rsq*(QQ_3 + ... + FR_rsq*QQ_8))
//    U_hi := 1 + QQ_1 * r_hi * r_hi		...any order
//    U_lo := QQ_1 * r_lo * (r + r_hi)
//    correction := -c*(r + S_1*FR_rsq*r)	...any order
//  Endif
//
//  V := poly + (U_lo + correction)	...observe order
//
//  result := (i_0 == 0?   1.0 : -1.0)
//
//  Last instruction in user-set rounding mode
//
//  result := (i_0 == 0?   result*U_hi + V :
//                        result*U_hi - V)
//
//  Return
//
//  Step 6. Small_r.
//
//  ...Use flush to zero mode without causing exception
//    Let [i_0 i_1] be the two lsb of N_fix.
//
//  FR_rsq := r * r
//
//  If i_1 = 0 then
//     z := FR_rsq*FR_rsq; z := FR_rsq*z *r
//     poly_lo := S_3 + FR_rsq*(S_4 + FR_rsq*S_5)
//     poly_hi := r*FR_rsq*(S_1 + FR_rsq*S_2)
//     correction := c
//     result := r
//  Else
//     z := FR_rsq*FR_rsq; z := FR_rsq*z
//     poly_lo := C_3 + FR_rsq*(C_4 + FR_rsq*C_5)
//     poly_hi := FR_rsq*(C_1 + FR_rsq*C_2)
//     correction := -c*r
//     result := 1
//  Endif
//
//  poly := poly_hi + (z * poly_lo + correction)
//
//  If i_0 = 1, result := -result
//
//  Last operation. Perform in user-set rounding mode
//
//  result := (i_0 == 0?     result + poly :
//                          result - poly )
//  Return
//
//  Step 7. Case_2_reduce.
//
//  ...Refer to the write up for argument reduction for
//  ...rationale. The reduction algorithm below is taken from
//  ...argument reduction description and integrated this.
//
//  w := N*P_3
//  U_1 := N*P_2 + w		...FMA
//  U_2 := (N*P_2 - U_1) + w	...2 FMA
//  ...U_1 + U_2 is  N*(P_2+P_3) accurately
//
//  r := s - U_1
//  c := ( (s - r) - U_1 ) - U_2
//
//  ...The mathematical sum r + c approximates the reduced
//  ...argument accurately. Note that although compared to
//  ...Case 1, this case requires much more work to reduce
//  ...the argument, the subsequent calculation needed for
//  ...any of the trigonometric function is very little because
//  ...|alpha| < 1.01*2^(-33) and thus two terms of the
//  ...Taylor series expansion suffices.
//
//  If i_1 = 0 then
//     poly := c + S_1 * r * r * r	...any order
//     result := r
//  Else
//     poly := -2^(-67)
//     result := 1.0
//  Endif
//
//  If i_0 = 1, result := -result
//
//  Last operation. Perform in user-set rounding mode
//
//  result := (i_0 == 0?     result + poly :
//                           result - poly )
//
//  Return
//
//
//  Step 8. Pre-reduction of large arguments.
//
//  ...Again, the following reduction procedure was described
//  ...in the separate write up for argument reduction, which
//  ...is tightly integrated here.

//  N_0 := Arg * Inv_P_0
//  N_0_fix := fcvt.fx( N_0 )
//  N_0 := fcvt.xf( N_0_fix)

//  Arg' := Arg - N_0 * P_0
//  w := N_0 * d_1
//  N := Arg' * two_by_PI
//  N_fix := fcvt.fx( N )
//  N := fcvt.xf( N_fix )
//  N_fix := N_fix + N_inc
//
//  s := Arg' - N * P_1
//  w := w - N * P_2
//
//  If |s| >= 2^(-14)
//     go to Step 3
//  Else
//     go to Step 9
//  Endif
//
//  Step 9. Case_4_reduce.
//
//    ...first obtain N_0*d_1 and -N*P_2 accurately
//   U_hi := N_0 * d_1		V_hi := -N*P_2
//   U_lo := N_0 * d_1 - U_hi	V_lo := -N*P_2 - U_hi	...FMAs
//
//   ...compute the contribution from N_0*d_1 and -N*P_3
//   w := -N*P_3
//   w := w + N_0*d_2
//   t := U_lo + V_lo + w		...any order
//
//   ...at this point, the mathematical value
//   ...s + U_hi + V_hi  + t approximates the true reduced argument
//   ...accurately. Just need to compute this accurately.
//
//   ...Calculate U_hi + V_hi accurately:
//   A := U_hi + V_hi
//   if |U_hi| >= |V_hi| then
//      a := (U_hi - A) + V_hi
//   else
//      a := (V_hi - A) + U_hi
//   endif
//   ...order in computing "a" must be observed. This branch is
//   ...best implemented by predicates.
//   ...A + a  is U_hi + V_hi accurately. Moreover, "a" is
//   ...much smaller than A: |a| <= (1/2)ulp(A).
//
//   ...Just need to calculate   s + A + a + t
//   C_hi := s + A		t := t + a
//   C_lo := (s - C_hi) + A
//   C_lo := C_lo + t
//
//   ...Final steps for reduction
//   r := C_hi + C_lo
//   c := (C_hi - r) + C_lo
//
//   ...At this point, we have r and c
//   ...And all we need is a couple of terms of the corresponding
//   ...Taylor series.
//
//   If i_1 = 0
//      poly := c + r*FR_rsq*(S_1 + FR_rsq*S_2)
//      result := r
//   Else
//      poly := FR_rsq*(C_1 + FR_rsq*C_2)
//      result := 1
//   Endif
//
//   If i_0 = 1, result := -result
//
//   Last operation. Perform in user-set rounding mode
//
//   result := (i_0 == 0?     result + poly :
//                            result - poly )
//   Return
//
//   Large Arguments: For arguments above 2**63, a Payne-Hanek
//   style argument reduction is used and pi_by_2 reduce is called.
//


#ifdef _LIBC
.rodata
#else
.data
#endif
.align 64

FSINCOS_CONSTANTS:
ASM_TYPE_DIRECTIVE(FSINCOS_CONSTANTS,@object)
data4 0x4B800000, 0xCB800000, 0x00000000,0x00000000 // two**24, -two**24
data4 0x4E44152A, 0xA2F9836E, 0x00003FFE,0x00000000 // Inv_pi_by_2
data4 0xCE81B9F1, 0xC84D32B0, 0x00004016,0x00000000 // P_0
data4 0x2168C235, 0xC90FDAA2, 0x00003FFF,0x00000000 // P_1
data4 0xFC8F8CBB, 0xECE675D1, 0x0000BFBD,0x00000000 // P_2
data4 0xACC19C60, 0xB7ED8FBB, 0x0000BF7C,0x00000000 // P_3
data4 0x5F000000, 0xDF000000, 0x00000000,0x00000000 // two_to_63, -two_to_63
data4 0x6EC6B45A, 0xA397E504, 0x00003FE7,0x00000000 // Inv_P_0
data4 0xDBD171A1, 0x8D848E89, 0x0000BFBF,0x00000000 // d_1
data4 0x18A66F8E, 0xD5394C36, 0x0000BF7C,0x00000000 // d_2
data4 0x2168C234, 0xC90FDAA2, 0x00003FFE,0x00000000 // pi_by_4
data4 0x2168C234, 0xC90FDAA2, 0x0000BFFE,0x00000000 // neg_pi_by_4
data4 0x3E000000, 0xBE000000, 0x00000000,0x00000000 // two**-3, -two**-3
data4 0x2F000000, 0xAF000000, 0x9E000000,0x00000000 // two**-33, -two**-33, -two**-67
data4 0xA21C0BC9, 0xCC8ABEBC, 0x00003FCE,0x00000000 // PP_8
data4 0x720221DA, 0xD7468A05, 0x0000BFD6,0x00000000 // PP_7
data4 0x640AD517, 0xB092382F, 0x00003FDE,0x00000000 // PP_6
data4 0xD1EB75A4, 0xD7322B47, 0x0000BFE5,0x00000000 // PP_5
data4 0xFFFFFFFE, 0xFFFFFFFF, 0x0000BFFD,0x00000000 // C_1
data4 0x00000000, 0xAAAA0000, 0x0000BFFC,0x00000000 // PP_1_hi
data4 0xBAF69EEA, 0xB8EF1D2A, 0x00003FEC,0x00000000 // PP_4
data4 0x0D03BB69, 0xD00D00D0, 0x0000BFF2,0x00000000 // PP_3
data4 0x88888962, 0x88888888, 0x00003FF8,0x00000000 // PP_2
data4 0xAAAB0000, 0xAAAAAAAA, 0x0000BFEC,0x00000000 // PP_1_lo
data4 0xC2B0FE52, 0xD56232EF, 0x00003FD2,0x00000000 // QQ_8
data4 0x2B48DCA6, 0xC9C99ABA, 0x0000BFDA,0x00000000 // QQ_7
data4 0x9C716658, 0x8F76C650, 0x00003FE2,0x00000000 // QQ_6
data4 0xFDA8D0FC, 0x93F27DBA, 0x0000BFE9,0x00000000 // QQ_5
data4 0xAAAAAAAA, 0xAAAAAAAA, 0x0000BFFC,0x00000000 // S_1
data4 0x00000000, 0x80000000, 0x0000BFFE,0x00000000 // QQ_1
data4 0x0C6E5041, 0xD00D00D0, 0x00003FEF,0x00000000 // QQ_4
data4 0x0B607F60, 0xB60B60B6, 0x0000BFF5,0x00000000 // QQ_3
data4 0xAAAAAA9B, 0xAAAAAAAA, 0x00003FFA,0x00000000 // QQ_2
data4 0xFFFFFFFE, 0xFFFFFFFF, 0x0000BFFD,0x00000000 // C_1
data4 0xAAAA719F, 0xAAAAAAAA, 0x00003FFA,0x00000000 // C_2
data4 0x0356F994, 0xB60B60B6, 0x0000BFF5,0x00000000 // C_3
data4 0xB2385EA9, 0xD00CFFD5, 0x00003FEF,0x00000000 // C_4
data4 0x292A14CD, 0x93E4BD18, 0x0000BFE9,0x00000000 // C_5
data4 0xAAAAAAAA, 0xAAAAAAAA, 0x0000BFFC,0x00000000 // S_1
data4 0x888868DB, 0x88888888, 0x00003FF8,0x00000000 // S_2
data4 0x055EFD4B, 0xD00D00D0, 0x0000BFF2,0x00000000 // S_3
data4 0x839730B9, 0xB8EF1C5D, 0x00003FEC,0x00000000 // S_4
data4 0xE5B3F492, 0xD71EA3A4, 0x0000BFE5,0x00000000 // S_5
data4 0x38800000, 0xB8800000, 0x00000000            // two**-14, -two**-14
ASM_SIZE_DIRECTIVE(FSINCOS_CONSTANTS)

FR_Input_X        = f8
FR_Neg_Two_to_M3  = f32
FR_Two_to_63      = f32
FR_Two_to_24      = f33
FR_Pi_by_4        = f33
FR_Two_to_M14     = f34
FR_Two_to_M33     = f35
FR_Neg_Two_to_24  = f36
FR_Neg_Pi_by_4    = f36
FR_Neg_Two_to_M14 = f37
FR_Neg_Two_to_M33 = f38
FR_Neg_Two_to_M67 = f39
FR_Inv_pi_by_2    = f40
FR_N_float        = f41
FR_N_fix          = f42
FR_P_1            = f43
FR_P_2            = f44
FR_P_3            = f45
FR_s              = f46
FR_w              = f47
FR_c              = f48
FR_r              = f49
FR_Z              = f50
FR_A              = f51
FR_a              = f52
FR_t              = f53
FR_U_1            = f54
FR_U_2            = f55
FR_C_1            = f56
FR_C_2            = f57
FR_C_3            = f58
FR_C_4            = f59
FR_C_5            = f60
FR_S_1            = f61
FR_S_2            = f62
FR_S_3            = f63
FR_S_4            = f64
FR_S_5            = f65
FR_poly_hi        = f66
FR_poly_lo        = f67
FR_r_hi           = f68
FR_r_lo           = f69
FR_rsq            = f70
FR_r_cubed        = f71
FR_C_hi           = f72
FR_N_0            = f73
FR_d_1            = f74
FR_V              = f75
FR_V_hi           = f75
FR_V_lo           = f76
FR_U_hi           = f77
FR_U_lo           = f78
FR_U_hiabs        = f79
FR_V_hiabs        = f80
FR_PP_8           = f81
FR_QQ_8           = f81
FR_PP_7           = f82
FR_QQ_7           = f82
FR_PP_6           = f83
FR_QQ_6           = f83
FR_PP_5           = f84
FR_QQ_5           = f84
FR_PP_4           = f85
FR_QQ_4           = f85
FR_PP_3           = f86
FR_QQ_3           = f86
FR_PP_2           = f87
FR_QQ_2           = f87
FR_QQ_1           = f88
FR_N_0_fix        = f89
FR_Inv_P_0        = f90
FR_corr           = f91
FR_poly           = f92
FR_d_2            = f93
FR_Two_to_M3      = f94
FR_Neg_Two_to_63  = f94
FR_P_0            = f95
FR_C_lo           = f96
FR_PP_1           = f97
FR_PP_1_lo        = f98
FR_ArgPrime       = f99

GR_Table_Base  = r32
GR_Table_Base1 = r33
GR_i_0         = r34
GR_i_1         = r35
GR_N_Inc       = r36
GR_Sin_or_Cos  = r37

GR_SAVE_B0     = r39
GR_SAVE_GP     = r40
GR_SAVE_PFS    = r41

.section .text
.proc __libm_sin_double_dbx#
.align 64
__libm_sin_double_dbx:

{ .mlx
alloc GR_Table_Base = ar.pfs,0,12,2,0
       movl GR_Sin_or_Cos = 0x0 ;;
}

{ .mmi
      nop.m 999
      addl           GR_Table_Base   = @ltoff(FSINCOS_CONSTANTS#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Base = [GR_Table_Base]
      nop.m 999
      nop.i 999
}
;;


{ .mib
      nop.m 999
      nop.i 999
       br.cond.sptk L(SINCOS_CONTINUE) ;;
}

.endp __libm_sin_double_dbx#
ASM_SIZE_DIRECTIVE(__libm_sin_double_dbx)

.section .text
.proc __libm_cos_double_dbx#
__libm_cos_double_dbx:

{ .mlx
alloc GR_Table_Base= ar.pfs,0,12,2,0
       movl GR_Sin_or_Cos = 0x1 ;;
}

{ .mmi
      nop.m 999
      addl           GR_Table_Base   = @ltoff(FSINCOS_CONSTANTS#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Base = [GR_Table_Base]
      nop.m 999
      nop.i 999
}
;;

//
//     Load Table Address
//
L(SINCOS_CONTINUE):

{ .mmi
       add GR_Table_Base1 = 96, GR_Table_Base
       ldfs	FR_Two_to_24 = [GR_Table_Base], 4
       nop.i 999
}
;;

{ .mmi
      nop.m 999
//
//     Load 2**24, load 2**63.
//
       ldfs	FR_Neg_Two_to_24 = [GR_Table_Base], 12
       mov   r41 = ar.pfs ;;
}

{ .mfi
       ldfs	FR_Two_to_63 = [GR_Table_Base1], 4
//
//     Check for unnormals - unsupported operands. We do not want
//     to generate denormal exception
//     Check for NatVals, QNaNs, SNaNs, +/-Infs
//     Check for EM unsupporteds
//     Check for Zero
//
       fclass.m.unc  p6, p8 =  FR_Input_X, 0x1E3
       mov   r40 = gp ;;
}

{ .mfi
      nop.m 999
       fclass.nm.unc p8, p0 =  FR_Input_X, 0x1FF
// GR_Sin_or_Cos denotes
       mov   r39 = b0
}

{ .mfb
       ldfs	FR_Neg_Two_to_63 = [GR_Table_Base1], 12
       fclass.m.unc p10, p0 = FR_Input_X, 0x007
(p6)   br.cond.spnt L(SINCOS_SPECIAL) ;;
}

{ .mib
      nop.m 999
      nop.i 999
(p8)   br.cond.spnt L(SINCOS_SPECIAL) ;;
}

{ .mib
      nop.m 999
      nop.i 999
//
//     Branch if +/- NaN, Inf.
//     Load -2**24, load -2**63.
//
(p10)  br.cond.spnt L(SINCOS_ZERO) ;;
}

{ .mmb
       ldfe	FR_Inv_pi_by_2 = [GR_Table_Base], 16
       ldfe	FR_Inv_P_0 = [GR_Table_Base1], 16
      nop.b 999 ;;
}

{ .mmb
      nop.m 999
       ldfe		FR_d_1 = [GR_Table_Base1], 16
      nop.b 999 ;;
}
//
//     Raise possible denormal operand flag with useful fcmp
//     Is x <= -2**63
//     Load Inv_P_0 for pre-reduction
//     Load Inv_pi_by_2
//

{ .mmb
       ldfe		FR_P_0 = [GR_Table_Base], 16
       ldfe	FR_d_2 = [GR_Table_Base1], 16
      nop.b 999 ;;
}
//
//     Load P_0
//     Load d_1
//     Is x >= 2**63
//     Is x <= -2**24?
//

{ .mmi
       ldfe	FR_P_1 = [GR_Table_Base], 16 ;;
//
//     Load P_1
//     Load d_2
//     Is x >= 2**24?
//
       ldfe	FR_P_2 = [GR_Table_Base], 16
      nop.i 999 ;;
}

{ .mmf
      nop.m 999
       ldfe	FR_P_3 = [GR_Table_Base], 16
       fcmp.le.unc.s1	p7, p8 = FR_Input_X, FR_Neg_Two_to_24
}

{ .mfi
      nop.m 999
//
//     Branch if +/- zero.
//     Decide about the paths to take:
//     If -2**24 < FR_Input_X < 2**24 - CASE 1 OR 2
//     OTHERWISE - CASE 3 OR 4
//
       fcmp.le.unc.s0	p10, p11 = FR_Input_X, FR_Neg_Two_to_63
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)   fcmp.ge.s1 p7, p0 = FR_Input_X, FR_Two_to_24
      nop.i 999
}

{ .mfi
       ldfe	FR_Pi_by_4 = [GR_Table_Base1], 16
(p11)  fcmp.ge.s1	p10, p0 = FR_Input_X, FR_Two_to_63
      nop.i 999 ;;
}

{ .mmi
       ldfe	FR_Neg_Pi_by_4 = [GR_Table_Base1], 16 ;;
       ldfs	FR_Two_to_M3 = [GR_Table_Base1], 4
      nop.i 999 ;;
}

{ .mib
       ldfs	FR_Neg_Two_to_M3 = [GR_Table_Base1], 12
      nop.i 999
//
//     Load P_2
//     Load P_3
//     Load pi_by_4
//     Load neg_pi_by_4
//     Load 2**(-3)
//     Load -2**(-3).
//
(p10)  br.cond.spnt L(SINCOS_ARG_TOO_LARGE) ;;
}

{ .mib
      nop.m 999
      nop.i 999
//
//     Branch out if x >= 2**63. Use Payne-Hanek Reduction
//
(p7)   br.cond.spnt L(SINCOS_LARGER_ARG) ;;
}

{ .mfi
      nop.m 999
//
//     Branch if Arg <= -2**24 or Arg >= 2**24 and use pre-reduction.
//
       fma.s1	FR_N_float = FR_Input_X, FR_Inv_pi_by_2, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
       fcmp.lt.unc.s1	p6, p7 = FR_Input_X, FR_Pi_by_4
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     Select the case when |Arg| < pi/4
//     Else Select the case when |Arg| >= pi/4
//
       fcvt.fx.s1 FR_N_fix = FR_N_float
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     N  = Arg * 2/pi
//     Check if Arg < pi/4
//
(p6)   fcmp.gt.s1 p6, p7 = FR_Input_X, FR_Neg_Pi_by_4
      nop.i 999 ;;
}
//
//     Case 2: Convert integer N_fix back to normalized floating-point value.
//     Case 1: p8 is only affected  when p6 is set
//

{ .mfi
(p7)   ldfs FR_Two_to_M33 = [GR_Table_Base1], 4
//
//     Grab the integer part of N and call it N_fix
//
(p6)   fmerge.se FR_r = FR_Input_X, FR_Input_X
//     If |x| < pi/4, r = x and c = 0
//     lf |x| < pi/4, is x < 2**(-3).
//     r = Arg
//     c = 0
(p6)   mov GR_N_Inc = GR_Sin_or_Cos ;;
}

{ .mmf
      nop.m 999
(p7)   ldfs FR_Neg_Two_to_M33 = [GR_Table_Base1], 4
(p6)   fmerge.se FR_c = f0, f0
}

{ .mfi
      nop.m 999
(p6)   fcmp.lt.unc.s1	p8, p9 = FR_Input_X, FR_Two_to_M3
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     lf |x| < pi/4, is -2**(-3)< x < 2**(-3) - set p8.
//     If |x| >= pi/4,
//     Create the right N for |x| < pi/4 and otherwise
//     Case 2: Place integer part of N in GP register
//
(p7)   fcvt.xf FR_N_float = FR_N_fix
      nop.i 999 ;;
}

{ .mmf
      nop.m 999
(p7)   getf.sig	GR_N_Inc = FR_N_fix
(p8)   fcmp.gt.s1 p8, p0 = FR_Input_X, FR_Neg_Two_to_M3 ;;
}

{ .mib
      nop.m 999
      nop.i 999
//
//     Load 2**(-33), -2**(-33)
//
(p8)   br.cond.spnt L(SINCOS_SMALL_R) ;;
}

{ .mib
      nop.m 999
      nop.i 999
(p6)   br.cond.sptk L(SINCOS_NORMAL_R) ;;
}
//
//     if |x| < pi/4, branch based on |x| < 2**(-3) or otherwise.
//
//
//     In this branch, |x| >= pi/4.
//

{ .mfi
       ldfs FR_Neg_Two_to_M67 = [GR_Table_Base1], 8
//
//     Load -2**(-67)
//
       fnma.s1	FR_s = FR_N_float, FR_P_1, FR_Input_X
//
//     w = N * P_2
//     s = -N * P_1  + Arg
//
       add GR_N_Inc = GR_N_Inc, GR_Sin_or_Cos
}

{ .mfi
      nop.m 999
       fma.s1	FR_w = FR_N_float, FR_P_2, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     Adjust N_fix by N_inc to determine whether sine or
//     cosine is being calculated
//
       fcmp.lt.unc.s1 p7, p6 = FR_s, FR_Two_to_M33
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p7)   fcmp.gt.s1 p7, p6 = FR_s, FR_Neg_Two_to_M33
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//     Remember x >= pi/4.
//     Is s <= -2**(-33) or s >= 2**(-33) (p6)
//     or -2**(-33) < s < 2**(-33) (p7)
(p6)   fms.s1 FR_r = FR_s, f1, FR_w
      nop.i 999
}

{ .mfi
      nop.m 999
(p7)   fma.s1 FR_w = FR_N_float, FR_P_3, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p7)   fma.s1 FR_U_1 = FR_N_float, FR_P_2, FR_w
      nop.i 999
}

{ .mfi
      nop.m 999
(p6)   fms.s1 FR_c = FR_s, f1, FR_r
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     For big s: r = s - w: No futher reduction is necessary
//     For small s: w = N * P_3 (change sign) More reduction
//
(p6)   fcmp.lt.unc.s1 p8, p9 = FR_r, FR_Two_to_M3
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)   fcmp.gt.s1 p8, p9 = FR_r, FR_Neg_Two_to_M3
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p7)   fms.s1 FR_r = FR_s, f1, FR_U_1
      nop.i 999
}

{ .mfb
      nop.m 999
//
//     For big s: Is |r| < 2**(-3)?
//     For big s: c = S - r
//     For small s: U_1 = N * P_2 + w
//
//     If p8 is set, prepare to branch to Small_R.
//     If p9 is set, prepare to branch to Normal_R.
//     For big s,  r is complete here.
//
(p6)   fms.s1 FR_c = FR_c, f1, FR_w
//
//     For big s: c = c + w (w has not been negated.)
//     For small s: r = S - U_1
//
(p8)   br.cond.spnt	L(SINCOS_SMALL_R) ;;
}

{ .mib
      nop.m 999
      nop.i 999
(p9)   br.cond.sptk	L(SINCOS_NORMAL_R) ;;
}

{ .mfi
(p7)   add GR_Table_Base1 = 224, GR_Table_Base1
//
//     Branch to SINCOS_SMALL_R or SINCOS_NORMAL_R
//
(p7)   fms.s1 FR_U_2 = FR_N_float, FR_P_2, FR_U_1
//
//     c = S - U_1
//     r = S_1 * r
//
//
(p7)   extr.u	GR_i_1 = GR_N_Inc, 0, 1
}

{ .mmi
      nop.m 999 ;;
//
//     Get [i_0,i_1] - two lsb of N_fix_gr.
//     Do dummy fmpy so inexact is always set.
//
(p7)   cmp.eq.unc p9, p10 = 0x0, GR_i_1
(p7)   extr.u	GR_i_0 = GR_N_Inc, 1, 1 ;;
}
//
//     For small s: U_2 = N * P_2 - U_1
//     S_1 stored constant - grab the one stored with the
//     coefficients.
//

{ .mfi
(p7)   ldfe FR_S_1 = [GR_Table_Base1], 16
//
//     Check if i_1 and i_0  != 0
//
(p10)  fma.s1	FR_poly = f0, f1, FR_Neg_Two_to_M67
(p7)   cmp.eq.unc p11, p12 = 0x0, GR_i_0 ;;
}

{ .mfi
      nop.m 999
(p7)   fms.s1	FR_s = FR_s, f1, FR_r
      nop.i 999
}

{ .mfi
      nop.m 999
//
//     S = S - r
//     U_2 = U_2 + w
//     load S_1
//
(p7)   fma.s1	FR_rsq = FR_r, FR_r, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p7)   fma.s1	FR_U_2 = FR_U_2, f1, FR_w
      nop.i 999
}

{ .mfi
      nop.m 999
(p7)   fmerge.se FR_Input_X = FR_r, FR_r
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p10)  fma.s1 FR_Input_X = f0, f1, f1
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     FR_rsq = r * r
//     Save r as the result.
//
(p7)   fms.s1	FR_c = FR_s, f1, FR_U_1
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     if ( i_1 ==0) poly = c + S_1*r*r*r
//     else Result = 1
//
(p12)  fnma.s1 FR_Input_X = FR_Input_X, f1, f0
      nop.i 999
}

{ .mfi
      nop.m 999
(p7)   fma.s1	FR_r = FR_S_1, FR_r, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p7)   fma.d.s0	FR_S_1 = FR_S_1, FR_S_1, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     If i_1 != 0, poly = 2**(-67)
//
(p7)   fms.s1 FR_c = FR_c, f1, FR_U_2
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     c = c - U_2
//
(p9)   fma.s1 FR_poly = FR_r, FR_rsq, FR_c
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     i_0 != 0, so Result = -Result
//
(p11)  fma.d.s0 FR_Input_X = FR_Input_X, f1, FR_poly
      nop.i 999 ;;
}

{ .mfb
      nop.m 999
(p12)  fms.d.s0 FR_Input_X = FR_Input_X, f1, FR_poly
//
//     if (i_0 == 0),  Result = Result + poly
//     else            Result = Result - poly
//
       br.ret.sptk   b0 ;;
}
L(SINCOS_LARGER_ARG):

{ .mfi
      nop.m 999
       fma.s1 FR_N_0 = FR_Input_X, FR_Inv_P_0, f0
      nop.i 999
}
;;

//     This path for argument > 2*24
//     Adjust table_ptr1 to beginning of table.
//

{ .mmi
      nop.m 999
      addl           GR_Table_Base   = @ltoff(FSINCOS_CONSTANTS#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Base = [GR_Table_Base]
      nop.m 999
      nop.i 999
}
;;


//
//     Point to  2*-14
//     N_0 = Arg * Inv_P_0
//

{ .mmi
       add GR_Table_Base = 688, GR_Table_Base ;;
       ldfs FR_Two_to_M14 = [GR_Table_Base], 4
      nop.i 999 ;;
}

{ .mfi
       ldfs FR_Neg_Two_to_M14 = [GR_Table_Base], 0
      nop.f 999
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     Load values 2**(-14) and -2**(-14)
//
       fcvt.fx.s1 FR_N_0_fix = FR_N_0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     N_0_fix  = integer part of N_0
//
       fcvt.xf FR_N_0 = FR_N_0_fix
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     Make N_0 the integer part
//
       fnma.s1 FR_ArgPrime = FR_N_0, FR_P_0, FR_Input_X
      nop.i 999
}

{ .mfi
      nop.m 999
       fma.s1 FR_w = FR_N_0, FR_d_1, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     Arg' = -N_0 * P_0 + Arg
//     w  = N_0 * d_1
//
       fma.s1 FR_N_float = FR_ArgPrime, FR_Inv_pi_by_2, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     N = A' * 2/pi
//
       fcvt.fx.s1 FR_N_fix = FR_N_float
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     N_fix is the integer part
//
       fcvt.xf FR_N_float = FR_N_fix
      nop.i 999 ;;
}

{ .mfi
       getf.sig GR_N_Inc = FR_N_fix
      nop.f 999
      nop.i 999 ;;
}

{ .mii
      nop.m 999
      nop.i 999 ;;
       add GR_N_Inc = GR_N_Inc, GR_Sin_or_Cos ;;
}

{ .mfi
      nop.m 999
//
//     N is the integer part of the reduced-reduced argument.
//     Put the integer in a GP register
//
       fnma.s1 FR_s = FR_N_float, FR_P_1, FR_ArgPrime
      nop.i 999
}

{ .mfi
      nop.m 999
       fnma.s1 FR_w = FR_N_float, FR_P_2, FR_w
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     s = -N*P_1 + Arg'
//     w = -N*P_2 + w
//     N_fix_gr = N_fix_gr + N_inc
//
       fcmp.lt.unc.s1 p9, p8 = FR_s, FR_Two_to_M14
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p9)   fcmp.gt.s1 p9, p8 = FR_s, FR_Neg_Two_to_M14
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     For |s|  > 2**(-14) r = S + w (r complete)
//     Else       U_hi = N_0 * d_1
//
(p9)   fma.s1 FR_V_hi = FR_N_float, FR_P_2, f0
      nop.i 999
}

{ .mfi
      nop.m 999
(p9)   fma.s1 FR_U_hi = FR_N_0, FR_d_1, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     Either S <= -2**(-14) or S >= 2**(-14)
//     or -2**(-14) < s < 2**(-14)
//
(p8)   fma.s1 FR_r = FR_s, f1, FR_w
      nop.i 999
}

{ .mfi
      nop.m 999
(p9)   fma.s1 FR_w = FR_N_float, FR_P_3, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     We need abs of both U_hi and V_hi - don't
//     worry about switched sign of V_hi.
//
(p9)   fms.s1 FR_A = FR_U_hi, f1, FR_V_hi
      nop.i 999
}

{ .mfi
      nop.m 999
//
//     Big s: finish up c = (S - r) + w (c complete)
//     Case 4: A =  U_hi + V_hi
//     Note: Worry about switched sign of V_hi, so subtract instead of add.
//
(p9)   fnma.s1 FR_V_lo = FR_N_float, FR_P_2, FR_V_hi
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p9)   fms.s1 FR_U_lo = FR_N_0, FR_d_1, FR_U_hi
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p9)   fmerge.s FR_V_hiabs = f0, FR_V_hi
      nop.i 999
}

{ .mfi
      nop.m 999
//     For big s: c = S - r
//     For small s do more work: U_lo = N_0 * d_1 - U_hi
//
(p9)   fmerge.s FR_U_hiabs = f0, FR_U_hi
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     For big s: Is |r| < 2**(-3)
//     For big s: if p12 set, prepare to branch to Small_R.
//     For big s: If p13 set, prepare to branch to Normal_R.
//
(p8)   fms.s1 FR_c = FR_s, f1, FR_r
      nop.i 999
}

{ .mfi
      nop.m 999
//
//     For small S: V_hi = N * P_2
//                  w = N * P_3
//     Note the product does not include the (-) as in the writeup
//     so (-) missing for V_hi and w.
//
(p8)   fcmp.lt.unc.s1 p12, p13 = FR_r, FR_Two_to_M3
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p12)  fcmp.gt.s1 p12, p13 = FR_r, FR_Neg_Two_to_M3
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p8)   fma.s1 FR_c = FR_c, f1, FR_w
      nop.i 999
}

{ .mfb
      nop.m 999
(p9)   fms.s1 FR_w = FR_N_0, FR_d_2, FR_w
(p12)  br.cond.spnt L(SINCOS_SMALL_R) ;;
}

{ .mib
      nop.m 999
      nop.i 999
(p13)  br.cond.sptk L(SINCOS_NORMAL_R) ;;
}

{ .mfi
      nop.m 999
//
//     Big s: Vector off when |r| < 2**(-3).  Recall that p8 will be true.
//     The remaining stuff is for Case 4.
//     Small s: V_lo = N * P_2 + U_hi (U_hi is in place of V_hi in writeup)
//     Note: the (-) is still missing for V_lo.
//     Small s: w = w + N_0 * d_2
//     Note: the (-) is now incorporated in w.
//
(p9)   fcmp.ge.unc.s1 p10, p11 = FR_U_hiabs, FR_V_hiabs
       extr.u	GR_i_1 = GR_N_Inc, 0, 1 ;;
}

{ .mfi
      nop.m 999
//
//     C_hi = S + A
//
(p9)   fma.s1 FR_t = FR_U_lo, f1, FR_V_lo
       extr.u	GR_i_0 = GR_N_Inc, 1, 1 ;;
}

{ .mfi
      nop.m 999
//
//     t = U_lo + V_lo
//
//
(p10)  fms.s1 FR_a = FR_U_hi, f1, FR_A
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p11)  fma.s1 FR_a = FR_V_hi, f1, FR_A
      nop.i 999
}
;;

{ .mmi
      nop.m 999
      addl           GR_Table_Base   = @ltoff(FSINCOS_CONSTANTS#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Base = [GR_Table_Base]
      nop.m 999
      nop.i 999
}
;;


{ .mfi
       add GR_Table_Base = 528, GR_Table_Base
//
//     Is U_hiabs >= V_hiabs?
//
(p9)   fma.s1 FR_C_hi = FR_s, f1, FR_A
      nop.i 999 ;;
}

{ .mmi
       ldfe FR_C_1 = [GR_Table_Base], 16 ;;
       ldfe FR_C_2 = [GR_Table_Base], 64
      nop.i 999 ;;
}

{ .mmf
      nop.m 999
//
//     c = c + C_lo  finished.
//     Load  C_2
//
       ldfe	FR_S_1 = [GR_Table_Base], 16
//
//     C_lo = S - C_hi
//
       fma.s1 FR_t = FR_t, f1, FR_w ;;
}
//
//     r and c have been computed.
//     Make sure ftz mode is set - should be automatic when using wre
//     |r| < 2**(-3)
//     Get [i_0,i_1] - two lsb of N_fix.
//     Load S_1
//

{ .mfi
       ldfe FR_S_2 = [GR_Table_Base], 64
//
//     t = t + w
//
(p10)  fms.s1 FR_a = FR_a, f1, FR_V_hi
       cmp.eq.unc p9, p10 = 0x0, GR_i_0
}

{ .mfi
      nop.m 999
//
//     For larger u than v: a = U_hi - A
//     Else a = V_hi - A (do an add to account for missing (-) on V_hi
//
       fms.s1 FR_C_lo = FR_s, f1, FR_C_hi
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p11)  fms.s1 FR_a = FR_U_hi, f1, FR_a
       cmp.eq.unc p11, p12 = 0x0, GR_i_1
}

{ .mfi
      nop.m 999
//
//     If u > v: a = (U_hi - A)  + V_hi
//     Else      a = (V_hi - A)  + U_hi
//     In each case account for negative missing from V_hi.
//
       fma.s1 FR_C_lo = FR_C_lo, f1, FR_A
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     C_lo = (S - C_hi) + A
//
       fma.s1 FR_t = FR_t, f1, FR_a
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     t = t + a
//
       fma.s1 FR_C_lo = FR_C_lo, f1, FR_t
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     C_lo = C_lo + t
//     Adjust Table_Base to beginning of table
//
       fma.s1 FR_r = FR_C_hi, f1, FR_C_lo
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     Load S_2
//
       fma.s1 FR_rsq = FR_r, FR_r, f0
      nop.i 999
}

{ .mfi
      nop.m 999
//
//     Table_Base points to C_1
//     r = C_hi + C_lo
//
       fms.s1 FR_c = FR_C_hi, f1, FR_r
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     if i_1 ==0: poly = S_2 * FR_rsq + S_1
//     else        poly = C_2 * FR_rsq + C_1
//
(p11)  fma.s1 FR_Input_X = f0, f1, FR_r
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p12)  fma.s1 FR_Input_X = f0, f1, f1
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     Compute r_cube = FR_rsq * r
//
(p11)  fma.s1 FR_poly = FR_rsq, FR_S_2, FR_S_1
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p12)  fma.s1 FR_poly = FR_rsq, FR_C_2, FR_C_1
      nop.i 999
}

{ .mfi
      nop.m 999
//
//     Compute FR_rsq = r * r
//     Is i_1 == 0 ?
//
       fma.s1 FR_r_cubed = FR_rsq, FR_r, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     c = C_hi - r
//     Load  C_1
//
       fma.s1 FR_c = FR_c, f1, FR_C_lo
      nop.i 999
}

{ .mfi
      nop.m 999
//
//     if i_1 ==0: poly = r_cube * poly + c
//     else        poly = FR_rsq * poly
//
(p10)  fms.s1 FR_Input_X = f0, f1, FR_Input_X
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     if i_1 ==0: Result = r
//     else        Result = 1.0
//
(p11)  fma.s1 FR_poly = FR_r_cubed, FR_poly, FR_c
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p12)  fma.s1 FR_poly = FR_rsq, FR_poly, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//     if i_0 !=0: Result = -Result
//
(p9)   fma.d.s0 FR_Input_X = FR_Input_X, f1, FR_poly
      nop.i 999 ;;
}

{ .mfb
      nop.m 999
(p10)  fms.d.s0 FR_Input_X = FR_Input_X, f1, FR_poly
//
//     if i_0 == 0: Result = Result + poly
//     else         Result = Result - poly
//
       br.ret.sptk   b0 ;;
}
L(SINCOS_SMALL_R):

{ .mii
      nop.m 999
      	extr.u	GR_i_1 = GR_N_Inc, 0, 1 ;;
//
//
//      Compare both i_1 and i_0 with 0.
//      if i_1 == 0, set p9.
//      if i_0 == 0, set p11.
//
      	cmp.eq.unc p9, p10 = 0x0, GR_i_1 ;;
}

{ .mfi
      nop.m 999
      	fma.s1 FR_rsq = FR_r, FR_r, f0
      	extr.u	GR_i_0 = GR_N_Inc, 1, 1 ;;
}

{ .mfi
      nop.m 999
//
// 	Z = Z * FR_rsq
//
(p10)	fnma.s1	FR_c = FR_c, FR_r, f0
      	cmp.eq.unc p11, p12 = 0x0, GR_i_0
}
;;

// ******************************************************************
// ******************************************************************
// ******************************************************************
//      r and c have been computed.
//      We know whether this is the sine or cosine routine.
//      Make sure ftz mode is set - should be automatic when using wre
//      |r| < 2**(-3)
//
//      Set table_ptr1 to beginning of constant table.
//      Get [i_0,i_1] - two lsb of N_fix_gr.
//

{ .mmi
      nop.m 999
      addl           GR_Table_Base   = @ltoff(FSINCOS_CONSTANTS#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Base = [GR_Table_Base]
      nop.m 999
      nop.i 999
}
;;


//
//      Set table_ptr1 to point to S_5.
//      Set table_ptr1 to point to C_5.
//      Compute FR_rsq = r * r
//

{ .mfi
(p9)  	add GR_Table_Base = 672, GR_Table_Base
(p10)	fmerge.s FR_r = f1, f1
(p10) 	add GR_Table_Base = 592, GR_Table_Base ;;
}
//
//      Set table_ptr1 to point to S_5.
//      Set table_ptr1 to point to C_5.
//

{ .mmi
(p9)  	ldfe FR_S_5 = [GR_Table_Base], -16 ;;
//
//      if (i_1 == 0) load S_5
//      if (i_1 != 0) load C_5
//
(p9)  	ldfe FR_S_4 = [GR_Table_Base], -16
      nop.i 999 ;;
}

{ .mmf
(p10) 	ldfe FR_C_5 = [GR_Table_Base], -16
//
//      Z = FR_rsq * FR_rsq
//
(p9)  	ldfe FR_S_3 = [GR_Table_Base], -16
//
//      Compute FR_rsq = r * r
//      if (i_1 == 0) load S_4
//      if (i_1 != 0) load C_4
//
       	fma.s1 FR_Z = FR_rsq, FR_rsq, f0 ;;
}
//
//      if (i_1 == 0) load S_3
//      if (i_1 != 0) load C_3
//

{ .mmi
(p9)  	ldfe FR_S_2 = [GR_Table_Base], -16 ;;
//
//      if (i_1 == 0) load S_2
//      if (i_1 != 0) load C_2
//
(p9)  	ldfe FR_S_1 = [GR_Table_Base], -16
      nop.i 999
}

{ .mmi
(p10) 	ldfe FR_C_4 = [GR_Table_Base], -16 ;;
(p10)  	ldfe FR_C_3 = [GR_Table_Base], -16
      nop.i 999 ;;
}

{ .mmi
(p10) 	ldfe FR_C_2 = [GR_Table_Base], -16 ;;
(p10) 	ldfe FR_C_1 = [GR_Table_Base], -16
      nop.i 999
}

{ .mfi
      nop.m 999
//
//      if (i_1 != 0):
//      poly_lo = FR_rsq * C_5 + C_4
//      poly_hi = FR_rsq * C_2 + C_1
//
(p9)  	fma.s1 FR_Z = FR_Z, FR_r, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      if (i_1 == 0) load S_1
//      if (i_1 != 0) load C_1
//
(p9)  	fma.s1 FR_poly_lo = FR_rsq, FR_S_5, FR_S_4
      nop.i 999
}

{ .mfi
      nop.m 999
//
//      c = -c * r
//      dummy fmpy's to flag inexact.
//
(p9)	fma.d.s0 FR_S_4 = FR_S_4, FR_S_4, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      poly_lo = FR_rsq * poly_lo + C_3
//      poly_hi = FR_rsq * poly_hi
//
        fma.s1	FR_Z = FR_Z, FR_rsq, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p9)  	fma.s1 FR_poly_hi = FR_rsq, FR_S_2, FR_S_1
      nop.i 999
}

{ .mfi
      nop.m 999
//
//      if (i_1 == 0):
//      poly_lo = FR_rsq * S_5 + S_4
//      poly_hi = FR_rsq * S_2 + S_1
//
(p10) 	fma.s1 FR_poly_lo = FR_rsq, FR_C_5, FR_C_4
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      if (i_1 == 0):
//      Z = Z * r  for only one of the small r cases - not there
//      in original implementation notes.
//
(p9)  	fma.s1 FR_poly_lo = FR_rsq, FR_poly_lo, FR_S_3
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p10) 	fma.s1 FR_poly_hi = FR_rsq, FR_C_2, FR_C_1
      nop.i 999
}

{ .mfi
      nop.m 999
(p10)	fma.d.s0 FR_C_1 = FR_C_1, FR_C_1, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p9)  	fma.s1 FR_poly_hi = FR_poly_hi, FR_rsq, f0
      nop.i 999
}

{ .mfi
      nop.m 999
//
//      poly_lo = FR_rsq * poly_lo + S_3
//      poly_hi = FR_rsq * poly_hi
//
(p10) 	fma.s1 FR_poly_lo = FR_rsq, FR_poly_lo, FR_C_3
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p10) 	fma.s1 FR_poly_hi = FR_poly_hi, FR_rsq, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
// 	if (i_1 == 0): dummy fmpy's to flag inexact
// 	r = 1
//
(p9)	fma.s1 FR_poly_hi = FR_r, FR_poly_hi, f0
      nop.i 999
}

{ .mfi
      nop.m 999
//
// 	poly_hi = r * poly_hi
//
        fma.s1	FR_poly = FR_Z, FR_poly_lo, FR_c
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p12)	fms.s1	FR_r = f0, f1, FR_r
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      poly_hi = Z * poly_lo + c
// 	if i_0 == 1: r = -r
//
     	fma.s1	FR_poly = FR_poly, f1, FR_poly_hi
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p12)	fms.d.s0 FR_Input_X = FR_r, f1, FR_poly
      nop.i 999
}

{ .mfb
      nop.m 999
//
//      poly = poly + poly_hi
//
(p11)	fma.d.s0 FR_Input_X = FR_r, f1, FR_poly
//
//      if (i_0 == 0) Result = r + poly
//      if (i_0 != 0) Result = r - poly
//
       br.ret.sptk   b0 ;;
}
L(SINCOS_NORMAL_R):

{ .mii
      nop.m 999
    	extr.u	GR_i_1 = GR_N_Inc, 0, 1 ;;
//
//      Set table_ptr1 and table_ptr2 to base address of
//      constant table.
    	cmp.eq.unc p9, p10 = 0x0, GR_i_1 ;;
}

{ .mfi
      nop.m 999
    	fma.s1	FR_rsq = FR_r, FR_r, f0
    	extr.u	GR_i_0 = GR_N_Inc, 1, 1 ;;
}

{ .mfi
      nop.m 999
    	frcpa.s1 FR_r_hi, p6 = f1, FR_r
    	cmp.eq.unc p11, p12 = 0x0, GR_i_0
}
;;

// ******************************************************************
// ******************************************************************
// ******************************************************************
//
//      r and c have been computed.
//      We known whether this is the sine or cosine routine.
//      Make sure ftz mode is set - should be automatic when using wre
//      Get [i_0,i_1] - two lsb of N_fix_gr alone.
//

{ .mmi
      nop.m 999
      addl           GR_Table_Base   = @ltoff(FSINCOS_CONSTANTS#), gp
      nop.i 999
}
;;

{ .mmi
      ld8 GR_Table_Base = [GR_Table_Base]
      nop.m 999
      nop.i 999
}
;;


{ .mfi
(p10)	add GR_Table_Base = 384, GR_Table_Base
(p12)	fms.s1 FR_Input_X = f0, f1, f1
(p9)	add GR_Table_Base = 224, GR_Table_Base ;;
}

{ .mmf
      nop.m 999
(p10)	ldfe FR_QQ_8 = [GR_Table_Base], 16
//
//      if (i_1==0) poly = poly * FR_rsq + PP_1_lo
//      else        poly = FR_rsq * poly
//
(p11)	fma.s1 FR_Input_X = f0, f1, f1 ;;
}

{ .mmf
(p10)	ldfe FR_QQ_7 = [GR_Table_Base], 16
//
// 	Adjust table pointers based on i_0
//      Compute rsq = r * r
//
(p9)	ldfe FR_PP_8 = [GR_Table_Base], 16
    	fma.s1 FR_r_cubed = FR_r, FR_rsq, f0 ;;
}

{ .mmf
(p9)	ldfe FR_PP_7 = [GR_Table_Base], 16
(p10)	ldfe FR_QQ_6 = [GR_Table_Base], 16
//
//      Load PP_8 and QQ_8; PP_7 and QQ_7
//
    	frcpa.s1 FR_r_hi, p6 = f1, FR_r_hi ;;
}
//
//      if (i_1==0) poly =   PP_7 + FR_rsq * PP_8.
//      else        poly =   QQ_7 + FR_rsq * QQ_8.
//

{ .mmb
(p9)	ldfe FR_PP_6 = [GR_Table_Base], 16
(p10)	ldfe FR_QQ_5 = [GR_Table_Base], 16
      nop.b 999 ;;
}

{ .mmb
(p9)	ldfe FR_PP_5 = [GR_Table_Base], 16
(p10)	ldfe FR_S_1 = [GR_Table_Base], 16
      nop.b 999 ;;
}

{ .mmb
(p10)	ldfe FR_QQ_1 = [GR_Table_Base], 16
(p9)	ldfe FR_C_1 = [GR_Table_Base], 16
      nop.b 999 ;;
}

{ .mmi
(p10)	ldfe FR_QQ_4 = [GR_Table_Base], 16 ;;
(p9)	ldfe FR_PP_1 = [GR_Table_Base], 16
      nop.i 999 ;;
}

{ .mmf
(p10)	ldfe FR_QQ_3 = [GR_Table_Base], 16
//
//      if (i_1=0) corr = corr + c*c
//      else       corr = corr * c
//
(p9)	ldfe FR_PP_4 = [GR_Table_Base], 16
(p10)	fma.s1 FR_poly = FR_rsq, FR_QQ_8, FR_QQ_7 ;;
}
//
//      if (i_1=0) poly = rsq * poly + PP_5
//      else       poly = rsq * poly + QQ_5
//      Load PP_4 or QQ_4
//

{ .mmf
(p9)	ldfe FR_PP_3 = [GR_Table_Base], 16
(p10)	ldfe FR_QQ_2 = [GR_Table_Base], 16
//
//      r_hi =   frcpa(frcpa(r)).
//      r_cube = r * FR_rsq.
//
(p9)	fma.s1 FR_poly = FR_rsq, FR_PP_8, FR_PP_7 ;;
}
//
//      Do dummy multiplies so inexact is always set.
//

{ .mfi
(p9)	ldfe FR_PP_2 = [GR_Table_Base], 16
//
//      r_lo = r - r_hi
//
(p9)	fma.s1 FR_U_lo = FR_r_hi, FR_r_hi, f0
      nop.i 999 ;;
}

{ .mmf
      nop.m 999
(p9)	ldfe FR_PP_1_lo = [GR_Table_Base], 16
(p10)	fma.s1 FR_corr = FR_S_1, FR_r_cubed, FR_r
}

{ .mfi
      nop.m 999
(p10)	fma.s1 FR_poly = FR_rsq, FR_poly, FR_QQ_6
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      if (i_1=0) U_lo = r_hi * r_hi
//      else       U_lo = r_hi + r
//
(p9)	fma.s1 FR_corr = FR_C_1, FR_rsq, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      if (i_1=0) corr = C_1 * rsq
//      else       corr = S_1 * r_cubed + r
//
(p9)	fma.s1 FR_poly = FR_rsq, FR_poly, FR_PP_6
      nop.i 999
}

{ .mfi
      nop.m 999
(p10)	fma.s1 FR_U_lo = FR_r_hi, f1, FR_r
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      if (i_1=0) U_hi = r_hi + U_hi
//      else       U_hi = QQ_1 * U_hi + 1
//
(p9)	fma.s1 FR_U_lo = FR_r, FR_r_hi, FR_U_lo
      nop.i 999
}

{ .mfi
      nop.m 999
//
//      U_hi = r_hi * r_hi
//
    	fms.s1 FR_r_lo = FR_r, f1, FR_r_hi
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      Load PP_1, PP_6, PP_5, and C_1
//      Load QQ_1, QQ_6, QQ_5, and S_1
//
    	fma.s1 FR_U_hi = FR_r_hi, FR_r_hi, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p10)	fma.s1 FR_poly = FR_rsq, FR_poly, FR_QQ_5
      nop.i 999
}

{ .mfi
      nop.m 999
(p10)	fnma.s1	FR_corr = FR_corr, FR_c, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      if (i_1=0) U_lo = r * r_hi + U_lo
//      else       U_lo = r_lo * U_lo
//
(p9)	fma.s1 FR_corr = FR_corr, FR_c, FR_c
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p9)	fma.s1 FR_poly = FR_rsq, FR_poly, FR_PP_5
      nop.i 999
}

{ .mfi
      nop.m 999
//
//      if (i_1 =0) U_hi = r + U_hi
//      if (i_1 =0) U_lo = r_lo * U_lo
//
//
(p9)	fma.d.s0 FR_PP_5 = FR_PP_5, FR_PP_4, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p9)	fma.s1 FR_U_lo = FR_r, FR_r, FR_U_lo
      nop.i 999
}

{ .mfi
      nop.m 999
(p10)	fma.s1 FR_U_lo = FR_r_lo, FR_U_lo, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      if (i_1=0) poly = poly * rsq + PP_6
//      else       poly = poly * rsq + QQ_6
//
(p9)	fma.s1 FR_U_hi = FR_r_hi, FR_U_hi, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p10)	fma.s1 FR_poly = FR_rsq, FR_poly, FR_QQ_4
      nop.i 999
}

{ .mfi
      nop.m 999
(p10)	fma.s1 FR_U_hi = FR_QQ_1, FR_U_hi, f1
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p10)	fma.d.s0 FR_QQ_5 = FR_QQ_5, FR_QQ_5, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      if (i_1!=0) U_hi = PP_1 * U_hi
//      if (i_1!=0) U_lo = r * r  + U_lo
//      Load PP_3 or QQ_3
//
(p9)	fma.s1 FR_poly = FR_rsq, FR_poly, FR_PP_4
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p9)	fma.s1 FR_U_lo = FR_r_lo, FR_U_lo, f0
      nop.i 999
}

{ .mfi
      nop.m 999
(p10)	fma.s1 FR_U_lo = FR_QQ_1,FR_U_lo, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p9)	fma.s1 FR_U_hi = FR_PP_1, FR_U_hi, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p10)	fma.s1 FR_poly = FR_rsq, FR_poly, FR_QQ_3
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      Load PP_2, QQ_2
//
(p9)	fma.s1 FR_poly = FR_rsq, FR_poly, FR_PP_3
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      if (i_1==0) poly = FR_rsq * poly  + PP_3
//      else        poly = FR_rsq * poly  + QQ_3
//      Load PP_1_lo
//
(p9)	fma.s1 FR_U_lo = FR_PP_1, FR_U_lo, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      if (i_1 =0) poly = poly * rsq + pp_r4
//      else        poly = poly * rsq + qq_r4
//
(p9)	fma.s1 FR_U_hi = FR_r, f1, FR_U_hi
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p10)	fma.s1 FR_poly = FR_rsq, FR_poly, FR_QQ_2
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      if (i_1==0) U_lo =  PP_1_hi * U_lo
//      else        U_lo =  QQ_1 * U_lo
//
(p9)	fma.s1 FR_poly = FR_rsq, FR_poly, FR_PP_2
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      if (i_0==0)  Result = 1
//      else         Result = -1
//
     	fma.s1 FR_V = FR_U_lo, f1, FR_corr
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p10)	fma.s1 FR_poly = FR_rsq, FR_poly, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      if (i_1==0) poly =  FR_rsq * poly + PP_2
//      else poly =  FR_rsq * poly + QQ_2
//
(p9)	fma.s1 FR_poly = FR_rsq, FR_poly, FR_PP_1_lo
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p10)	fma.s1 FR_poly = FR_rsq, FR_poly, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      V = U_lo + corr
//
(p9)	fma.s1 FR_poly = FR_r_cubed, FR_poly, f0
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
//
//      if (i_1==0) poly = r_cube * poly
//      else        poly = FR_rsq * poly
//
    	fma.s1	FR_V = FR_poly, f1, FR_V
      nop.i 999 ;;
}

{ .mfi
      nop.m 999
(p12)	fms.d.s0 FR_Input_X = FR_Input_X, FR_U_hi, FR_V
      nop.i 999
}

{ .mfb
      nop.m 999
//
//      V = V + poly
//
(p11)	fma.d.s0 FR_Input_X = FR_Input_X, FR_U_hi, FR_V
//
//      if (i_0==0) Result = Result * U_hi + V
//      else        Result = Result * U_hi - V
//
       br.ret.sptk   b0 ;;
}

//
//      If cosine, FR_Input_X = 1
//      If sine, FR_Input_X = +/-Zero (Input FR_Input_X)
//      Results are exact, no exceptions
//
L(SINCOS_ZERO):

{ .mmb
        cmp.eq.unc p6, p7 = 0x1, GR_Sin_or_Cos
      nop.m 999
      nop.b 999 ;;
}

{ .mfi
      nop.m 999
(p7)    fmerge.s FR_Input_X = FR_Input_X, FR_Input_X
      nop.i 999
}

{ .mfb
      nop.m 999
(p6)    fmerge.s FR_Input_X = f1, f1
       br.ret.sptk   b0 ;;
}

L(SINCOS_SPECIAL):

//
//      Path for Arg = +/- QNaN, SNaN, Inf
//      Invalid can be raised. SNaNs
//      become QNaNs
//

{ .mfb
      nop.m 999
        fmpy.d.s0 FR_Input_X = FR_Input_X, f0
        br.ret.sptk   b0 ;;
}
.endp __libm_cos_double_dbx#
ASM_SIZE_DIRECTIVE(__libm_cos_double_dbx#)



//
//      Call int pi_by_2_reduce(double* x, double *y)
//      for |arguments| >= 2**63
//      Address to save r and c as double
//
//
//      psp    sp+64
//             sp+48  -> f0 c
//      r45    sp+32  -> f0 r
//      r44 -> sp+16  -> InputX
//      sp     sp     -> scratch provided to callee



.proc __libm_callout_2
__libm_callout_2:
L(SINCOS_ARG_TOO_LARGE):

.prologue
{ .mfi
        add   r45=-32,sp                        // Parameter: r address
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
        stfe [r45] = f0,16                      // Clear Parameter r on stack
        add  r44 = 16,sp                        // Parameter x address
.save   b0, GR_SAVE_B0
        mov GR_SAVE_B0=b0                       // Save b0
};;
.body
{ .mib
        stfe [r45] = f0,-16                     // Clear Parameter c on stack
        nop.i 0
        nop.b 0
}
{ .mib
        stfe [r44] = FR_Input_X                 // Store Parameter x on stack
        nop.i 0
        br.call.sptk b0=__libm_pi_by_2_reduce# ;;
};;


{ .mii
        ldfe  FR_Input_X =[r44],16
//
//      Get r and c off stack
//
        adds  GR_Table_Base1 = -16, GR_Table_Base1
//
//      Get r and c off stack
//
        add   GR_N_Inc = GR_Sin_or_Cos,r8 ;;
}
{ .mmb
        ldfe  FR_r =[r45],16
//
//      Get X off the stack
//      Readjust Table ptr
//
        ldfs FR_Two_to_M3 = [GR_Table_Base1],4
        nop.b 999 ;;
}
{ .mmb
        ldfs FR_Neg_Two_to_M3 = [GR_Table_Base1],0
        ldfe  FR_c =[r45]
        nop.b 999 ;;
}

{ .mfi
.restore sp
        add   sp = 64,sp                       // Restore stack pointer
        fcmp.lt.unc.s1  p6, p0 = FR_r, FR_Two_to_M3
        mov   b0 = GR_SAVE_B0                  // Restore return address
};;
{ .mib
        mov   gp = GR_SAVE_GP                  // Restore gp
        mov   ar.pfs = GR_SAVE_PFS             // Restore ar.pfs
        nop.b 0
};;


{ .mfi
      nop.m 999
(p6)    fcmp.gt.unc.s1	p6, p0 = FR_r, FR_Neg_Two_to_M3
      nop.i 999 ;;
}

{ .mib
      nop.m 999
      nop.i 999
(p6)    br.cond.spnt L(SINCOS_SMALL_R) ;;
}

{ .mib
      nop.m 999
      nop.i 999
        br.cond.sptk L(SINCOS_NORMAL_R) ;;
}

.endp __libm_callout_2
ASM_SIZE_DIRECTIVE(__libm_callout_2)

.type   __libm_pi_by_2_reduce#,@function
.global __libm_pi_by_2_reduce#


.type __libm_sin_double_dbx#,@function
.global __libm_sin_double_dbx#
.type __libm_cos_double_dbx#,@function
.global __libm_cos_double_dbx#
