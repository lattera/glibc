//
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

// History: 02/02/2000 Initial version
//          2/28/2000 added tags for logb and nextafter
//          3/22/2000 Changes to support _LIB_VERSION variable
//                    and filled some enum gaps. Added support for C99.
//          5/31/2000 added prototypes for __libm_frexp_4l/8l
//          8/10/2000 Changed declaration of _LIB_VERSION to work for library
//                    builds and other application builds (precompiler directives).
//          8/11/2000 Added pointers-to-matherr-functions declarations to allow
//                    for user-defined matherr functions in the dll build.
//         12/07/2000 Added scalbn error_types values.
//

#ifndef ASSEMBLER
#include <math.h>

float __libm_frexp_4f( float x, int*  exp);
float __libm_frexp_8f( float x, int*  exp);
double __libm_frexp_4( double x, int*  exp);
double __libm_frexp_8( double x, int*  exp);
long double __libm_frexp_4l( long double x, int*  exp);
long double __libm_frexp_8l( long double x, int*  exp);
void __libm_sincos_pi4(double,double*,double*,int);
void __libm_y0y1(double , double *, double *);
void __libm_j0j1(double , double *, double *);
double __libm_lgamma_kernel(double,int*,int,int);
double __libm_j0(double);
double __libm_j1(double);
double __libm_jn(int,double);
double __libm_y0(double);
double __libm_y1(double);
double __libm_yn(int,double);

extern double rint(double);
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

extern long lround(double);
extern long lroundf(float);
extern long lroundl(long double);

#if !(defined(SIZE_INT_32) || defined(SIZE_INT_64))
    #error integer size not established; define SIZE_INT_32 or SIZE_INT_64
#endif

struct fp64 { /*/ sign:1 exponent:11 significand:52 (implied leading 1)*/
  unsigned lo_significand:32;
  unsigned hi_significand:20;
  unsigned exponent:11;
  unsigned sign:1;
};

#define HI_SIGNIFICAND_LESS(X, HI) ((X)->hi_significand < 0x ## HI)
#define f64abs(x) ((x) < 0.0 ? -(x) : (x))

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
  scalbnf_overflow, scalbnf_underflow            /* 178, 179 */
} error_types;

void __libm_error_support(void*,void*,void*,error_types);

#define BIAS_64  1023
#define EXPINF_64  2047

#define DOUBLE_HEX(HI, LO) 0x ## LO, 0x ## HI

#if 0
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
#define        MATHERR_F       _matherrf
#define        MATHERR_D       _matherr
#else
#define        MATHERR_F       matherrf
#define        MATHERR_D       matherr
#endif

# ifdef __cplusplus
#define        EXC_DECL_D      __exception
#else
// exception is a reserved name in C++
#define        EXC_DECL_D      exception
#endif

extern int MATHERR_F(struct exceptionf*);
extern int MATHERR_D(struct EXC_DECL_D*);
extern int matherrl(struct exceptionl*);


/* Set these appropriately to make thread Safe */
#define ERRNO_RANGE  errno = ERANGE
#define ERRNO_DOMAIN errno = EDOM


// Add code to support _LIB_VERSION
#ifndef _LIBC
typedef enum
{
    _IEEE_ = -1, // IEEE-like behavior
    _SVID_,      // SysV, Rel. 4 behavior
    _XOPEN_,     // Unix98
    _POSIX_,     // Posix
    _ISOC_       // ISO C9X
} _LIB_VERSION_TYPE;

extern _LIB_VERSION_TYPE _LIB_VERSION;
#endif

// This is a run-time variable and may effect
// floating point behavior of the libm functions

#endif	/* ASSEMBLER */

/* Support for compatible assembler handling.  */
#if !defined L && defined _LIBC
#define L(name) .L##name
#endif
#ifdef __ELF__
#define ASM_SIZE_DIRECTIVE(name) .size name,.-name
#define ASM_TYPE_DIRECTIVE(name,T) .type name,T
#else
#define ASM_SIZE_DIRECTIVE(name)
#define ASM_TYPE_DIRECTIVE(name,T)
#endif
