/* file: libm_support.h */


// Copyright (c) 2000 - 2002, Intel Corporation
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

// History: 02/02/2000 Initial version 
//          2/28/2000 added tags for logb and nextafter
//          3/22/2000 Changes to support _LIB_VERSIONIMF variable
//                    and filled some enum gaps. Added support for C99.  
//          5/31/2000 added prototypes for __libm_frexp_4l/8l
//          8/10/2000 Changed declaration of _LIB_VERSIONIMF to work for library
//                    builds and other application builds (precompiler directives).
//          8/11/2000 Added pointers-to-matherr-functions declarations to allow
//                    for user-defined matherr functions in the dll build.
//         12/07/2000 Added scalbn error_types values.
//          5/01/2001 Added error_types values for C99 nearest integer 
//                    functions.
//          6/07/2001 Added error_types values for fdim.
//          6/18/2001 Added include of complex_support.h.
//          8/03/2001 Added error_types values for nexttoward, scalbln.
//          8/23/2001 Corrected tag numbers from 186 and higher.
//          8/27/2001 Added check for long int and long long int definitions.
//         12/10/2001 Added error_types for erfc.
//         12/27/2001 Added error_types for degree argument functions.
//         01/02/2002 Added error_types for tand, cotd.
//         01/04/2002 Delete include of complex_support.h
//         01/23/2002 Deleted prototypes for __libm_frexp*.  Added check for
//                    multiple int, long int, and long long int definitions.
//         05/20/2002 Added error_types for cot.
//         06/27/2002 Added error_types for sinhcosh.
//         12/05/2002 Added error_types for annuity and compound
//         04/10/2003 Added error_types for tgammal/tgamma/tgammaf
//

void __libm_sincos_pi4(double,double*,double*,int);
void __libm_y0y1(double , double *, double *);
void __libm_j0j1(double , double *, double *);
double __libm_j0(double);
double __libm_j1(double);
double __libm_jn(int,double);
double __libm_y0(double);
double __libm_y1(double);
double __libm_yn(int,double);
double __libm_copysign (double, double);
float __libm_copysignf (float, float);
long double __libm_copysignl (long double, long double);

extern double sqrt(double);
extern double fabs(double);
extern double log(double);
extern double log1p(double);
extern double sqrt(double);
extern double sin(double);
extern double exp(double);
extern double modf(double, double *);
extern double asinh(double);
extern double acosh(double);
extern double atanh(double);
extern double tanh(double);
extern double erf(double);
extern double erfc(double);
extern double j0(double);
extern double j1(double);
extern double jn(int, double);
extern double y0(double);
extern double y1(double);
extern double yn(int, double);

extern float  fabsf(float);
extern float  asinhf(float);
extern float  acoshf(float);
extern float  atanhf(float);
extern float  tanhf(float);
extern float  erff(float);
extern float  erfcf(float);
extern float  j0f(float);
extern float  j1f(float);
extern float  jnf(int, float);
extern float  y0f(float);
extern float  y1f(float);
extern float  ynf(int, float);

extern long double log1pl(long double);
extern long double logl(long double);
extern long double sqrtl(long double);
extern long double expl(long double);
extern long double fabsl(long double);

#if !(defined(SIZE_INT_32) || defined(SIZE_INT_64))
#error integer size not established; define SIZE_INT_32 or SIZE_INT_64
#endif

#if (defined(SIZE_INT_32) && defined(SIZE_INT_64))
#error multiple integer size definitions; define SIZE_INT_32 or SIZE_INT_64
#endif

#if !(defined(SIZE_LONG_INT_32) || defined(SIZE_LONG_INT_64))
#error long int size not established; define SIZE_LONG_INT_32 or SIZE_LONG_INT_64
#endif

#if (defined(SIZE_LONG_INT_32) && defined(SIZE_LONG_INT_64))
#error multiple long int size definitions; define SIZE_LONG_INT_32 or SIZE_LONG_INT_64
#endif

#if !(defined(SIZE_LONG_LONG_INT_32) || defined(SIZE_LONG_LONG_INT_64))
#error long long int size not established; define SIZE_LONG_LONG_INT_32 or SIZE_LONG_LONG_INT_64
#endif

#if (defined(SIZE_LONG_LONG_INT_32) && defined(SIZE_LONG_LONG_INT_64))
#error multiple long long int size definitions; define SIZE_LONG_LONG_INT_32 or SIZE_LONG_LONG_INT_64
#endif

typedef enum
{
  logl_zero=0,   logl_negative,                  /*  0,  1 */
  log_zero,      log_negative,                   /*  2,  3 */
  logf_zero,     logf_negative,                  /*  4,  5 */
  log10l_zero,   log10l_negative,                /*  6,  7 */
  log10_zero,    log10_negative,                 /*  8,  9 */
  log10f_zero,   log10f_negative,                /* 10, 11 */
  expl_overflow, expl_underflow,                 /* 12, 13 */
  exp_overflow,  exp_underflow,                  /* 14, 15 */
  expf_overflow, expf_underflow,                 /* 16, 17 */
  powl_overflow, powl_underflow,                 /* 18, 19 */
  powl_zero_to_zero,                             /* 20     */
  powl_zero_to_negative,                         /* 21     */
  powl_neg_to_non_integer,                       /* 22     */
  powl_nan_to_zero,                              /* 23     */
  pow_overflow,  pow_underflow,                  /* 24, 25 */
  pow_zero_to_zero,                              /* 26     */ 
  pow_zero_to_negative,                          /* 27     */
  pow_neg_to_non_integer,                        /* 28     */
  pow_nan_to_zero,                               /* 29     */
  powf_overflow, powf_underflow,                 /* 30, 31 */
  powf_zero_to_zero,                             /* 32     */
  powf_zero_to_negative,                         /* 33     */ 
  powf_neg_to_non_integer,                       /* 34     */ 
  powf_nan_to_zero,                              /* 35     */
  atan2l_zero,                                   /* 36     */
  atan2_zero,                                    /* 37     */
  atan2f_zero,                                   /* 38     */
  expm1l_overflow,                               /* 39     */
  expm1l_underflow,                              /* 40     */
  expm1_overflow,                                /* 41     */
  expm1_underflow,                               /* 42     */
  expm1f_overflow,                               /* 43     */
  expm1f_underflow,                              /* 44     */
  hypotl_overflow,                               /* 45     */
  hypot_overflow,                                /* 46     */
  hypotf_overflow,                               /* 47     */
  sqrtl_negative,                                /* 48     */
  sqrt_negative,                                 /* 49     */
  sqrtf_negative,                                /* 50     */
  scalbl_overflow, scalbl_underflow,             /* 51, 52  */
  scalb_overflow,  scalb_underflow,              /* 53, 54  */
  scalbf_overflow, scalbf_underflow,             /* 55, 56  */
  acosl_gt_one, acos_gt_one, acosf_gt_one,       /* 57, 58, 59 */
  asinl_gt_one, asin_gt_one, asinf_gt_one,       /* 60, 61, 62 */
  coshl_overflow, cosh_overflow, coshf_overflow, /* 63, 64, 65 */
  y0l_zero, y0l_negative,y0l_gt_loss,            /* 66, 67, 68 */
  y0_zero, y0_negative,y0_gt_loss,               /* 69, 70, 71 */
  y0f_zero, y0f_negative,y0f_gt_loss,            /* 72, 73, 74 */
  y1l_zero, y1l_negative,y1l_gt_loss,            /* 75, 76, 77 */ 
  y1_zero, y1_negative,y1_gt_loss,               /* 78, 79, 80 */ 
  y1f_zero, y1f_negative,y1f_gt_loss,            /* 81, 82, 83 */ 
  ynl_zero, ynl_negative,ynl_gt_loss,            /* 84, 85, 86 */
  yn_zero, yn_negative,yn_gt_loss,               /* 87, 88, 89 */
  ynf_zero, ynf_negative,ynf_gt_loss,            /* 90, 91, 92 */
  j0l_gt_loss,                                   /* 93 */ 
  j0_gt_loss,                                    /* 94 */
  j0f_gt_loss,                                   /* 95 */
  j1l_gt_loss,                                   /* 96 */
  j1_gt_loss,                                    /* 97 */
  j1f_gt_loss,                                   /* 98 */
  jnl_gt_loss,                                   /* 99 */
  jn_gt_loss,                                    /* 100 */
  jnf_gt_loss,                                   /* 101 */
  lgammal_overflow, lgammal_negative,lgammal_reserve, /* 102, 103, 104 */
  lgamma_overflow, lgamma_negative,lgamma_reserve,    /* 105, 106, 107 */
  lgammaf_overflow, lgammaf_negative, lgammaf_reserve,/* 108, 109, 110 */
  gammal_overflow,gammal_negative, gammal_reserve,    /* 111, 112, 113 */
  gamma_overflow, gamma_negative, gamma_reserve,      /* 114, 115, 116 */
  gammaf_overflow,gammaf_negative,gammaf_reserve,     /* 117, 118, 119 */   
  fmodl_by_zero,                                 /* 120 */
  fmod_by_zero,                                  /* 121 */
  fmodf_by_zero,                                 /* 122 */
  remainderl_by_zero,                            /* 123 */
  remainder_by_zero,                             /* 124 */
  remainderf_by_zero,                            /* 125 */
  sinhl_overflow, sinh_overflow, sinhf_overflow, /* 126, 127, 128 */
  atanhl_gt_one, atanhl_eq_one,                  /* 129, 130 */
  atanh_gt_one, atanh_eq_one,                    /* 131, 132 */
  atanhf_gt_one, atanhf_eq_one,                  /* 133, 134 */
  acoshl_lt_one,                                 /* 135 */
  acosh_lt_one,                                  /* 136 */
  acoshf_lt_one,                                 /* 137 */
  log1pl_zero,   log1pl_negative,                /* 138, 139 */
  log1p_zero,    log1p_negative,                 /* 140, 141 */
  log1pf_zero,   log1pf_negative,                /* 142, 143 */
  ldexpl_overflow,   ldexpl_underflow,           /* 144, 145 */
  ldexp_overflow,    ldexp_underflow,            /* 146, 147 */
  ldexpf_overflow,   ldexpf_underflow,           /* 148, 149 */
  logbl_zero,   logb_zero, logbf_zero,           /* 150, 151, 152 */
  nextafterl_overflow,   nextafter_overflow,  
  nextafterf_overflow,                           /* 153, 154, 155 */
  ilogbl_zero,  ilogb_zero, ilogbf_zero,         /* 156, 157, 158 */
  exp2l_overflow, exp2l_underflow,               /* 159, 160 */
  exp2_overflow,  exp2_underflow,                /* 161, 162 */
  exp2f_overflow, exp2f_underflow,               /* 163, 164 */
  exp10l_overflow, exp10_overflow,
  exp10f_overflow,                               /* 165, 166, 167 */
  log2l_zero,    log2l_negative,                 /* 168, 169 */
  log2_zero,     log2_negative,                  /* 170, 171 */
  log2f_zero,    log2f_negative,                 /* 172, 173 */
  scalbnl_overflow, scalbnl_underflow,           /* 174, 175 */
  scalbn_overflow,  scalbn_underflow,            /* 176, 177 */
  scalbnf_overflow, scalbnf_underflow,           /* 178, 179 */
  remquol_by_zero,                               /* 180 */
  remquo_by_zero,                                /* 181 */
  remquof_by_zero,                               /* 182 */
  lrintl_large, lrint_large, lrintf_large,       /* 183, 184, 185 */
  llrintl_large, llrint_large, llrintf_large,    /* 186, 187, 188 */
  lroundl_large, lround_large, lroundf_large,    /* 189, 190, 191 */
  llroundl_large, llround_large, llroundf_large, /* 192, 193, 194 */
  fdiml_overflow, fdim_overflow, fdimf_overflow, /* 195, 196, 197 */
  nexttowardl_overflow,   nexttoward_overflow,   
  nexttowardf_overflow,                          /* 198, 199, 200 */
  scalblnl_overflow, scalblnl_underflow,         /* 201, 202 */
  scalbln_overflow,  scalbln_underflow,          /* 203, 204 */
  scalblnf_overflow, scalblnf_underflow,         /* 205, 206 */
  erfcl_underflow, erfc_underflow, erfcf_underflow, /* 207, 208, 209 */
  acosdl_gt_one, acosd_gt_one, acosdf_gt_one,    /* 210, 211, 212 */
  asindl_gt_one, asind_gt_one, asindf_gt_one,    /* 213, 214, 215 */
  atan2dl_zero, atan2d_zero, atan2df_zero,       /* 216, 217, 218 */
  tandl_overflow, tand_overflow, tandf_overflow, /* 219, 220, 221 */
  cotdl_overflow, cotd_overflow, cotdf_overflow, /* 222, 223, 224 */
  cotl_overflow, cot_overflow, cotf_overflow,    /* 225, 226, 227 */
  sinhcoshl_overflow, sinhcosh_overflow, sinhcoshf_overflow, /* 228, 229, 230 */
  annuityl_by_zero, annuity_by_zero, annuityf_by_zero, /* 231, 232, 233 */
  annuityl_less_m1, annuity_less_m1, annuityf_less_m1, /* 234, 235, 236 */
  annuityl_overflow, annuity_overflow, annuityf_overflow, /* 237, 238, 239 */
  annuityl_underflow, annuity_underflow, annuityf_underflow, /* 240, 241, 242 */
  compoundl_by_zero, compound_by_zero, compoundf_by_zero, /* 243, 244, 245 */
  compoundl_less_m1, compound_less_m1, compoundf_less_m1, /* 246, 247, 248 */
  compoundl_overflow, compound_overflow, compoundf_overflow, /* 249, 250, 251 */
  compoundl_underflow, compound_underflow, compoundf_underflow, /* 252, 253, 254 */
  tgammal_overflow, tgammal_negative, tgammal_reserve, /* 255, 256, 257 */
  tgamma_overflow, tgamma_negative, tgamma_reserve, /* 258, 259, 260 */
  tgammaf_overflow, tgammaf_negative, tgammaf_reserve, /* 261, 262, 263 */
} error_types;

void __libm_error_support(void*,void*,void*,error_types);
#ifdef _LIBC
libc_hidden_proto(__libm_error_support)
#endif

#define HI_SIGNIFICAND_LESS(X, HI) ((X)->hi_significand < 0x ## HI)
#define f64abs(x) ((x) < 0.0 ? -(x) : (x))

#if !defined(__USE_EXTERNAL_FPMEMTYP_H__)

#define BIAS_32  0x007F
#define BIAS_64  0x03FF
#define BIAS_80  0x3FFF

#define MAXEXP_32  0x00FE
#define MAXEXP_64  0x07FE
#define MAXEXP_80  0x7FFE

#define EXPINF_32  0x00FF
#define EXPINF_64  0x07FF
#define EXPINF_80  0x7FFF

struct fp32 { /*// sign:1 exponent:8 significand:23 (implied leading 1)*/
#if defined(SIZE_INT_32)
    unsigned significand:23;
    unsigned exponent:8;
    unsigned sign:1;
#elif defined(SIZE_INT_64)
    unsigned significand:23;
    unsigned exponent:8;
    unsigned sign:1;
#endif
};

struct fp64 { /*/ sign:1 exponent:11 significand:52 (implied leading 1)*/
#if defined(SIZE_INT_32)
    unsigned lo_significand:32;
    unsigned hi_significand:20;
    unsigned exponent:11;
    unsigned sign:1;
#elif defined(SIZE_INT_64)
    unsigned significand:52;
    unsigned exponent:11;
    unsigned sign:1;
#endif
};

struct fp80 { /*/ sign:1 exponent:15 significand:64 (NO implied bits) */
#if defined(SIZE_INT_32)
    unsigned         lo_significand;
    unsigned         hi_significand;
    unsigned         exponent:15;
    unsigned         sign:1;
#elif defined(SIZE_INT_64)
    unsigned         significand;
    unsigned         exponent:15;
    unsigned         sign:1;
#endif
};

#endif /*__USE_EXTERNAL_FPMEMTYP_H__*/

/* macros to form a double value in hex representation (unsigned int type) */

#define DOUBLE_HEX(hi,lo) 0x##lo,0x##hi /*LITTLE_ENDIAN*/

/* macros to form a long double value in hex representation (unsigned short type) */

#if defined(_WIN32) || defined(_WIN64)
#define LDOUBLE_ALIGN 16
#else
#define LDOUBLE_ALIGN 12
#endif

#if (LDOUBLE_ALIGN == 16)
#define _XPD_ ,0x0000,0x0000,0x0000
#else /*12*/
#define _XPD_ ,0x0000
#endif

#define LDOUBLE_HEX(w4,w3,w2,w1,w0) 0x##w0,0x##w1,0x##w2,0x##w3,0x##w4 _XPD_ /*LITTLE_ENDIAN*/

/* macros to sign-expand low 'num' bits of 'val' to native integer */

#if defined(SIZE_INT_32)
# define SIGN_EXPAND(val,num)  ((int)(val) << (32-(num))) >> (32-(num)) /* sign expand of 'num' LSBs */
#elif defined(SIZE_INT_64)
# define SIGN_EXPAND(val,num)  ((int)(val) << (64-(num))) >> (64-(num)) /* sign expand of 'num' LSBs */
#endif

/* macros to form pointers to FP number on-the-fly */

#define FP32(f)  ((struct fp32 *)&f)
#define FP64(d)  ((struct fp64 *)&d)
#define FP80(ld) ((struct fp80 *)&ld)

/* macros to extract signed low and high doubleword of long double */

#if defined(SIZE_INT_32)
# define HI_DWORD_80(ld) ((((FP80(ld)->sign << 15) | FP80(ld)->exponent) << 16) | \
                          ((FP80(ld)->hi_significand >> 16) & 0xFFFF))
# define LO_DWORD_80(ld) SIGN_EXPAND(FP80(ld)->lo_significand, 32)
#elif defined(SIZE_INT_64)
# define HI_DWORD_80(ld) ((((FP80(ld)->sign << 15) | FP80(ld)->exponent) << 16) | \
                          ((FP80(ld)->significand >> 48) & 0xFFFF))
# define LO_DWORD_80(ld) SIGN_EXPAND(FP80(ld)->significand, 32)
#endif

/* macros to extract hi bits of significand.
 * note that explicit high bit do not count (returns as is)
 */

#if defined(SIZE_INT_32)
# define HI_SIGNIFICAND_80(X,NBITS) ((X)->hi_significand >> (31 - (NBITS)))
#elif defined(SIZE_INT_64)
# define HI_SIGNIFICAND_80(X,NBITS) ((X)->significand >> (63 - (NBITS)))
#endif

/* macros to check, whether a significand bits are all zero, or some of them are non-zero.
 * note that SIGNIFICAND_ZERO_80 tests high bit also, but SIGNIFICAND_NONZERO_80 does not
 */

#define SIGNIFICAND_ZERO_32(X)     ((X)->significand == 0)
#define SIGNIFICAND_NONZERO_32(X)  ((X)->significand != 0)

#if defined(SIZE_INT_32)
# define SIGNIFICAND_ZERO_64(X)    (((X)->hi_significand == 0) && ((X)->lo_significand == 0))
# define SIGNIFICAND_NONZERO_64(X) (((X)->hi_significand != 0) || ((X)->lo_significand != 0))
#elif defined(SIZE_INT_64)
# define SIGNIFICAND_ZERO_64(X)    ((X)->significand == 0)
# define SIGNIFICAND_NONZERO_64(X) ((X)->significand != 0)
#endif

#if defined(SIZE_INT_32)
# define SIGNIFICAND_ZERO_80(X)    (((X)->hi_significand == 0x00000000) && ((X)->lo_significand == 0))
# define SIGNIFICAND_NONZERO_80(X) (((X)->hi_significand != 0x80000000) || ((X)->lo_significand != 0))
#elif defined(SIZE_INT_64)
# define SIGNIFICAND_ZERO_80(X)    ((X)->significand == 0x0000000000000000)
# define SIGNIFICAND_NONZERO_80(X) ((X)->significand != 0x8000000000000000)
#endif

/* macros to compare long double with constant value, represented as hex */

#define SIGNIFICAND_EQ_HEX_32(X,BITS) ((X)->significand == 0x ## BITS)
#define SIGNIFICAND_GT_HEX_32(X,BITS) ((X)->significand >  0x ## BITS)
#define SIGNIFICAND_GE_HEX_32(X,BITS) ((X)->significand >= 0x ## BITS)
#define SIGNIFICAND_LT_HEX_32(X,BITS) ((X)->significand <  0x ## BITS)
#define SIGNIFICAND_LE_HEX_32(X,BITS) ((X)->significand <= 0x ## BITS)

#if defined(SIZE_INT_32)
# define SIGNIFICAND_EQ_HEX_64(X,HI,LO) \
    (((X)->hi_significand == 0x ## HI) && ((X)->lo_significand == 0x ## LO))
# define SIGNIFICAND_GT_HEX_64(X,HI,LO) (((X)->hi_significand > 0x ## HI) || \
    (((X)->hi_significand == 0x ## HI) && ((X)->lo_significand >  0x ## LO)))
# define SIGNIFICAND_GE_HEX_64(X,HI,LO) (((X)->hi_significand > 0x ## HI) || \
    (((X)->hi_significand == 0x ## HI) && ((X)->lo_significand >= 0x ## LO)))
# define SIGNIFICAND_LT_HEX_64(X,HI,LO) (((X)->hi_significand < 0x ## HI) || \
    (((X)->hi_significand == 0x ## HI) && ((X)->lo_significand <  0x ## LO)))
# define SIGNIFICAND_LE_HEX_64(X,HI,LO) (((X)->hi_significand < 0x ## HI) || \
    (((X)->hi_significand == 0x ## HI) && ((X)->lo_significand <= 0x ## LO)))
#elif defined(SIZE_INT_64)
# define SIGNIFICAND_EQ_HEX_64(X,HI,LO) ((X)->significand == 0x ## HI ## LO)
# define SIGNIFICAND_GT_HEX_64(X,HI,LO) ((X)->significand >  0x ## HI ## LO)
# define SIGNIFICAND_GE_HEX_64(X,HI,LO) ((X)->significand >= 0x ## HI ## LO)
# define SIGNIFICAND_LT_HEX_64(X,HI,LO) ((X)->significand <  0x ## HI ## LO)
# define SIGNIFICAND_LE_HEX_64(X,HI,LO) ((X)->significand <= 0x ## HI ## LO)
#endif
	
#if defined(SIZE_INT_32)
# define SIGNIFICAND_EQ_HEX_80(X,HI,LO) \
    (((X)->hi_significand == 0x ## HI) && ((X)->lo_significand == 0x ## LO))
# define SIGNIFICAND_GT_HEX_80(X,HI,LO) (((X)->hi_significand > 0x ## HI) || \
    (((X)->hi_significand == 0x ## HI) && ((X)->lo_significand >  0x ## LO)))
# define SIGNIFICAND_GE_HEX_80(X,HI,LO) (((X)->hi_significand > 0x ## HI) || \
    (((X)->hi_significand == 0x ## HI) && ((X)->lo_significand >= 0x ## LO)))
# define SIGNIFICAND_LT_HEX_80(X,HI,LO) (((X)->hi_significand < 0x ## HI) || \
    (((X)->hi_significand == 0x ## HI) && ((X)->lo_significand <  0x ## LO)))
# define SIGNIFICAND_LE_HEX_80(X,HI,LO) (((X)->hi_significand < 0x ## HI) || \
    (((X)->hi_significand == 0x ## HI) && ((X)->lo_significand <= 0x ## LO)))
#elif defined(SIZE_INT_64)
# define SIGNIFICAND_EQ_HEX_80(X,HI,LO) ((X)->significand == 0x ## HI ## LO)
# define SIGNIFICAND_GT_HEX_80(X,HI,LO) ((X)->significand >  0x ## HI ## LO)
# define SIGNIFICAND_GE_HEX_80(X,HI,LO) ((X)->significand >= 0x ## HI ## LO)
# define SIGNIFICAND_LT_HEX_80(X,HI,LO) ((X)->significand <  0x ## HI ## LO)
# define SIGNIFICAND_LE_HEX_80(X,HI,LO) ((X)->significand <= 0x ## HI ## LO)
#endif

#define VALUE_EQ_HEX_32(X,EXP,BITS) \
   (((X)->exponent == (EXP)) && (SIGNIFICAND_EQ_HEX_32(X, BITS)))
#define VALUE_GT_HEX_32(X,EXP,BITS) (((X)->exponent > (EXP)) || \
   (((X)->exponent == (EXP)) && (SIGNIFICAND_GT_HEX_32(X, BITS))))
#define VALUE_GE_HEX_32(X,EXP,BITS) (((X)->exponent > (EXP)) || \
   (((X)->exponent == (EXP)) && (SIGNIFICAND_GE_HEX_32(X, BITS))))
#define VALUE_LT_HEX_32(X,EXP,BITS) (((X)->exponent < (EXP)) || \
   (((X)->exponent == (EXP)) && (SIGNIFICAND_LT_HEX_32(X, BITS))))
#define VALUE_LE_HEX_32(X,EXP,BITS) (((X)->exponent < (EXP)) || \
   (((X)->exponent == (EXP)) && (SIGNIFICAND_LE_HEX_32(X, BITS))))

#define VALUE_EQ_HEX_64(X,EXP,HI,LO) \
   (((X)->exponent == (EXP)) && (SIGNIFICAND_EQ_HEX_64(X, HI, LO)))
#define VALUE_GT_HEX_64(X,EXP,HI,LO) (((X)->exponent > (EXP)) || \
   (((X)->exponent == (EXP)) && (SIGNIFICAND_GT_HEX_64(X, HI, LO))))
#define VALUE_GE_HEX_64(X,EXP,HI,LO) (((X)->exponent > (EXP)) || \
   (((X)->exponent == (EXP)) && (SIGNIFICAND_GE_HEX_64(X, HI, LO))))
#define VALUE_LT_HEX_64(X,EXP,HI,LO) (((X)->exponent < (EXP)) || \
   (((X)->exponent == (EXP)) && (SIGNIFICAND_LT_HEX_64(X, HI, LO))))
#define VALUE_LE_HEX_64(X,EXP,HI,LO) (((X)->exponent < (EXP)) || \
   (((X)->exponent == (EXP)) && (SIGNIFICAND_LE_HEX_64(X, HI, LO))))

#define VALUE_EQ_HEX_80(X,EXP,HI,LO) \
   (((X)->exponent == (EXP)) && (SIGNIFICAND_EQ_HEX_80(X, HI, LO)))
#define VALUE_GT_HEX_80(X,EXP,HI,LO) (((X)->exponent > (EXP)) || \
   (((X)->exponent == (EXP)) && (SIGNIFICAND_GT_HEX_80(X, HI, LO))))
#define VALUE_GE_HEX_80(X,EXP,HI,LO) (((X)->exponent > (EXP)) || \
   (((X)->exponent == (EXP)) && (SIGNIFICAND_GE_HEX_80(X, HI, LO))))
#define VALUE_LT_HEX_80(X,EXP,HI,LO) (((X)->exponent < (EXP)) || \
   (((X)->exponent == (EXP)) && (SIGNIFICAND_LT_HEX_80(X, HI, LO))))
#define VALUE_LE_HEX_80(X,EXP,HI,LO) (((X)->exponent < (EXP)) || \
   (((X)->exponent == (EXP)) && (SIGNIFICAND_LE_HEX_80(X, HI, LO))))

/* macros to compare two long doubles */

#define SIGNIFICAND_EQ_32(X,Y) ((X)->significand == (Y)->significand)
#define SIGNIFICAND_GT_32(X,Y) ((X)->significand > (Y)->significand)
#define SIGNIFICAND_GE_32(X,Y) ((X)->significand >= (Y)->significand)
#define SIGNIFICAND_LT_32(X,Y) ((X)->significand < (Y)->significand)
#define SIGNIFICAND_LE_32(X,Y) ((X)->significand <= (Y)->significand)

#if defined(SIZE_INT_32)
# define SIGNIFICAND_EQ_64(X,Y) \
	(((X)->hi_significand == (Y)->hi_significand) && ((X)->lo_significand == (Y)->lo_significand))
# define SIGNIFICAND_GT_64(X,Y) (((X)->hi_significand > (Y)->hi_significand) || \
	(((X)->hi_significand == (Y)->hi_significand) && ((X)->lo_significand >  (Y)->lo_significand)))
# define SIGNIFICAND_GE_64(X,Y) (((X)->hi_significand > (Y)->hi_significand) || \
	(((X)->hi_significand == (Y)->hi_significand) && ((X)->lo_significand >= (Y)->lo_significand)))
# define SIGNIFICAND_LT_64(X,Y) (((X)->hi_significand < (Y)->hi_significand) || \
    (((X)->hi_significand == (Y)->hi_significand) && ((X)->lo_significand <  (Y)->lo_significand)))
# define SIGNIFICAND_LE_64(X,Y) (((X)->hi_significand < (Y)->hi_significand) || \
    (((X)->hi_significand == (Y)->hi_significand) && ((X)->lo_significand <= (Y)->lo_significand)))
#elif defined(SIZE_INT_64)
# define SIGNIFICAND_EQ_64(X,Y) ((X)->significand == (Y)->significand)
# define SIGNIFICAND_GT_64(X,Y) ((X)->significand >  (Y)->significand)
# define SIGNIFICAND_GE_64(X,Y) ((X)->significand >= (Y)->significand)
# define SIGNIFICAND_LT_64(X,Y) ((X)->significand <  (Y)->significand)
# define SIGNIFICAND_LE_64(X,Y) ((X)->significand <= (Y)->significand)
#endif

#if defined(SIZE_INT_32)
# define SIGNIFICAND_EQ_80(X,Y) \
    (((X)->hi_significand == (Y)->hi_significand) && ((X)->lo_significand == (Y)->lo_significand))
# define SIGNIFICAND_GT_80(X,Y) (((X)->hi_significand > (Y)->hi_significand) || \
    (((X)->hi_significand == (Y)->hi_significand) && ((X)->lo_significand >  (Y)->lo_significand)))
# define SIGNIFICAND_GE_80(X,Y) (((X)->hi_significand > (Y)->hi_significand) || \
    (((X)->hi_significand == (Y)->hi_significand) && ((X)->lo_significand >= (Y)->lo_significand)))
# define SIGNIFICAND_LT_80(X,Y) (((X)->hi_significand < (Y)->hi_significand) || \
    (((X)->hi_significand == (Y)->hi_significand) && ((X)->lo_significand <  (Y)->lo_significand)))
# define SIGNIFICAND_LE_80(X,Y) (((X)->hi_significand < (Y)->hi_significand) || \
    (((X)->hi_significand == (Y)->hi_significand) && ((X)->lo_significand <= (Y)->lo_significand)))
#elif defined(SIZE_INT_64)
# define SIGNIFICAND_EQ_80(X,Y) ((X)->significand == (Y)->significand)
# define SIGNIFICAND_GT_80(X,Y) ((X)->significand >  (Y)->significand)
# define SIGNIFICAND_GE_80(X,Y) ((X)->significand >= (Y)->significand)
# define SIGNIFICAND_LT_80(X,Y) ((X)->significand <  (Y)->significand)
# define SIGNIFICAND_LE_80(X,Y) ((X)->significand <= (Y)->significand)
#endif

#define VALUE_EQ_32(X,Y) \
   (((X)->exponent == (Y)->exponent) && (SIGNIFICAND_EQ_32(X, Y)))
#define VALUE_GT_32(X,Y) (((X)->exponent > (Y)->exponent) || \
   (((X)->exponent == (Y)->exponent) && (SIGNIFICAND_GT_32(X, Y))))
#define VALUE_GE_32(X,Y) (((X)->exponent > (Y)->exponent) || \
   (((X)->exponent == (Y)->exponent) && (SIGNIFICAND_GE_32(X, Y))))
#define VALUE_LT_32(X,Y) (((X)->exponent < (Y)->exponent) || \
   (((X)->exponent == (Y)->exponent) && (SIGNIFICAND_LT_32(X, Y))))
#define VALUE_LE_32(X,Y) (((X)->exponent < (Y)->exponent) || \
   (((X)->exponent == (Y)->exponent) && (SIGNIFICAND_LE_32(X, Y))))
   
#define VALUE_EQ_64(X,Y) \
   (((X)->exponent == (Y)->exponent) && (SIGNIFICAND_EQ_64(X, Y)))
#define VALUE_GT_64(X,Y) (((X)->exponent > (Y)->exponent) || \
   (((X)->exponent == (Y)->exponent) && (SIGNIFICAND_GT_64(X, Y))))
#define VALUE_GE_64(X,Y) (((X)->exponent > (Y)->exponent) || \
   (((X)->exponent == (Y)->exponent) && (SIGNIFICAND_GE_64(X, Y))))
#define VALUE_LT_64(X,Y) (((X)->exponent < (Y)->exponent) || \
   (((X)->exponent == (Y)->exponent) && (SIGNIFICAND_LT_64(X, Y))))
#define VALUE_LE_64(X,Y) (((X)->exponent < (Y)->exponent) || \
   (((X)->exponent == (Y)->exponent) && (SIGNIFICAND_LE_64(X, Y))))
   
#define VALUE_EQ_80(X,Y) \
   (((X)->exponent == (Y)->exponent) && (SIGNIFICAND_EQ_80(X, Y)))
#define VALUE_GT_80(X,Y) (((X)->exponent > (Y)->exponent) || \
   (((X)->exponent == (Y)->exponent) && (SIGNIFICAND_GT_80(X, Y))))
#define VALUE_GE_80(X,Y) (((X)->exponent > (Y)->exponent) || \
   (((X)->exponent == (Y)->exponent) && (SIGNIFICAND_GE_80(X, Y))))
#define VALUE_LT_80(X,Y) (((X)->exponent < (Y)->exponent) || \
   (((X)->exponent == (Y)->exponent) && (SIGNIFICAND_LT_80(X, Y))))
#define VALUE_LE_80(X,Y) (((X)->exponent < (Y)->exponent) || \
   (((X)->exponent == (Y)->exponent) && (SIGNIFICAND_LE_80(X, Y))))

/* add/subtract 1 ulp macros */

#if defined(SIZE_INT_32)
# define ADD_ULP_80(X) \
    if ((++(X)->lo_significand == 0) && \
        (++(X)->hi_significand == (((X)->exponent == 0) ? 0x80000000 : 0))) \
    { \
        (X)->hi_significand |= 0x80000000; \
        ++(X)->exponent; \
    }
# define SUB_ULP_80(X) \
    if (--(X)->lo_significand == 0xFFFFFFFF) { \
        --(X)->hi_significand; \
        if (((X)->exponent != 0) && \
            ((X)->hi_significand == 0x7FFFFFFF) && \
            (--(X)->exponent != 0)) \
        { \
            (X)->hi_significand |= 0x80000000; \
        } \
    }
#elif defined(SIZE_INT_64)
# define ADD_ULP_80(X) \
    if (++(X)->significand == (((X)->exponent == 0) ? 0x8000000000000000 : 0))) { \
        (X)->significand |= 0x8000000000000000; \
        ++(X)->exponent; \
    }
# define SUB_ULP_80(X) \
    { \
        --(X)->significand; \
        if (((X)->exponent != 0) && \
            ((X)->significand == 0x7FFFFFFFFFFFFFFF) && \
            (--(X)->exponent != 0)) \
        { \
            (X)->significand |= 0x8000000000000000; \
        } \
    }
#endif



#if (defined(_WIN32) && !defined(_WIN64))

#define FP80_DECLARE()
#define _FPC_64    0x0300
static unsigned short __wControlWord, __wNewControlWord;
#define FP80_SET() { \
        __asm { fnstcw   word ptr [__wControlWord] }   \
        __wNewControlWord = __wControlWord | _FPC_64;  \
        __asm { fldcw   word ptr [__wNewControlWord] } \
    }
#define FP80_RESET() { \
        __asm { fldcw   word ptr [__wControlWord] } \
    }
#else /* defined(_WIN32) && !defined(_WIN64) */

#define FP80_DECLARE()
#define FP80_SET()
#define FP80_RESET()

#endif  /* defined(_WIN32) && !defined(_WIN64) */


#ifdef _LIBC
# include <math.h>
#else

static const unsigned INF[] = {
    DOUBLE_HEX(7ff00000, 00000000),
    DOUBLE_HEX(fff00000, 00000000)
};

static const double _zeroo = 0.0;
static const double _bigg = 1.0e300;
static const double _ponee = 1.0;
static const double _nonee = -1.0; 

#define INVALID    (_zeroo * *((double*)&INF[0]))
#define PINF       *((double*)&INF[0]) 
#define NINF       -PINF 
#define PINF_DZ    (_ponee/_zeroo) 
#define X_TLOSS    1.41484755040568800000e+16
#endif

struct exceptionf
{
  int type;
  char *name;
  float arg1, arg2, retval;
};

# ifdef __cplusplus
struct __exception
{
  int type;
  char *name;
  double arg1, arg2, retval;
};
# else 

#  ifndef _LIBC
struct exception
{
  int type;
  char *name;
  double arg1, arg2, retval;
};
#  endif
# endif



struct exceptionl
{
  int type;
  char *name;
  long double arg1, arg2, retval;
};

#ifdef _MS_
#define	MATHERR_F	_matherrf
#define	MATHERR_D	_matherr
#else
#define	MATHERR_F	matherrf
#define	MATHERR_D	matherr
#endif

# ifdef __cplusplus
#define	EXC_DECL_D	__exception
#else
// exception is a reserved name in C++
#define	EXC_DECL_D	exception
#endif

extern int MATHERR_F(struct exceptionf*);
extern int MATHERR_D(struct EXC_DECL_D*);
extern int matherrl(struct exceptionl*);


/* Set these appropriately to make thread Safe */
#define ERRNO_RANGE  errno = ERANGE
#define ERRNO_DOMAIN errno = EDOM


// Add code to support _LIB_VERSIONIMF
#ifndef _LIBC
typedef enum
{
    _IEEE_ = -1, // IEEE-like behavior
    _SVID_,      // SysV, Rel. 4 behavior
    _XOPEN_,     // Unix98
    _POSIX_,     // Posix
    _ISOC_       // ISO C9X
} _LIB_VERSION_TYPE;


#if !defined( LIBM_BUILD )
#if defined( _DLL )
extern _LIB_VERSION_TYPE __declspec(dllimport) _LIB_VERSIONIMF;
#else
extern _LIB_VERSION_TYPE _LIB_VERSIONIMF;
#endif	/* _DLL */
#else
extern int (*pmatherrf)(struct exceptionf*);
extern int (*pmatherr)(struct EXC_DECL_D*);
extern int (*pmatherrl)(struct exceptionl*);
#endif	/* LIBM_BUILD */

// This is a run-time variable and may affect
// floating point behavior of the libm functions
#endif
