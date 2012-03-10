/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */

/*
 * from: @(#)fdlibm.h 5.1 93/09/24
 */

#ifndef _MATH_PRIVATE_H_
#define _MATH_PRIVATE_H_

#include <endian.h>
#include <stdint.h>
#include <sys/types.h>
#include <fenv.h>

/* The original fdlibm code used statements like:
	n0 = ((*(int*)&one)>>29)^1;		* index of high word *
	ix0 = *(n0+(int*)&x);			* high word of x *
	ix1 = *((1-n0)+(int*)&x);		* low word of x *
   to dig two 32 bit words out of the 64 bit IEEE floating point
   value.  That is non-ANSI, and, moreover, the gcc instruction
   scheduler gets it wrong.  We instead use the following macros.
   Unlike the original code, we determine the endianness at compile
   time, not at run time; I don't see much benefit to selecting
   endianness at run time.  */

/* A union which permits us to convert between a double and two 32 bit
   ints.  */

#if __FLOAT_WORD_ORDER == BIG_ENDIAN

typedef union
{
  double value;
  struct
  {
    u_int32_t msw;
    u_int32_t lsw;
  } parts;
  uint64_t word;
} ieee_double_shape_type;

#endif

#if __FLOAT_WORD_ORDER == LITTLE_ENDIAN

typedef union
{
  double value;
  struct
  {
    u_int32_t lsw;
    u_int32_t msw;
  } parts;
  uint64_t word;
} ieee_double_shape_type;

#endif

/* Get two 32 bit ints from a double.  */

#define EXTRACT_WORDS(ix0,ix1,d)				\
do {								\
  ieee_double_shape_type ew_u;					\
  ew_u.value = (d);						\
  (ix0) = ew_u.parts.msw;					\
  (ix1) = ew_u.parts.lsw;					\
} while (0)

/* Get the more significant 32 bit int from a double.  */

#ifndef GET_HIGH_WORD
# define GET_HIGH_WORD(i,d)					\
do {								\
  ieee_double_shape_type gh_u;					\
  gh_u.value = (d);						\
  (i) = gh_u.parts.msw;						\
} while (0)
#endif

/* Get the less significant 32 bit int from a double.  */

#ifndef GET_LOW_WORD
# define GET_LOW_WORD(i,d)					\
do {								\
  ieee_double_shape_type gl_u;					\
  gl_u.value = (d);						\
  (i) = gl_u.parts.lsw;						\
} while (0)
#endif

/* Get all in one, efficient on 64-bit machines.  */
#ifndef EXTRACT_WORDS64
# define EXTRACT_WORDS64(i,d)					\
do {								\
  ieee_double_shape_type gh_u;					\
  gh_u.value = (d);						\
  (i) = gh_u.word;						\
} while (0)
#endif

/* Set a double from two 32 bit ints.  */
#ifndef INSERT_WORDS
# define INSERT_WORDS(d,ix0,ix1)				\
do {								\
  ieee_double_shape_type iw_u;					\
  iw_u.parts.msw = (ix0);					\
  iw_u.parts.lsw = (ix1);					\
  (d) = iw_u.value;						\
} while (0)
#endif

/* Get all in one, efficient on 64-bit machines.  */
#ifndef INSERT_WORDS64
# define INSERT_WORDS64(d,i)					\
do {								\
  ieee_double_shape_type iw_u;					\
  iw_u.word = (i);						\
  (d) = iw_u.value;						\
} while (0)
#endif

/* Set the more significant 32 bits of a double from an int.  */
#ifndef SET_HIGH_WORD
#define SET_HIGH_WORD(d,v)					\
do {								\
  ieee_double_shape_type sh_u;					\
  sh_u.value = (d);						\
  sh_u.parts.msw = (v);						\
  (d) = sh_u.value;						\
} while (0)
#endif

/* Set the less significant 32 bits of a double from an int.  */
#ifndef SET_LOW_WORD
# define SET_LOW_WORD(d,v)					\
do {								\
  ieee_double_shape_type sl_u;					\
  sl_u.value = (d);						\
  sl_u.parts.lsw = (v);						\
  (d) = sl_u.value;						\
} while (0)
#endif

/* A union which permits us to convert between a float and a 32 bit
   int.  */

typedef union
{
  float value;
  u_int32_t word;
} ieee_float_shape_type;

/* Get a 32 bit int from a float.  */
#ifndef GET_FLOAT_WORD
# define GET_FLOAT_WORD(i,d)					\
do {								\
  ieee_float_shape_type gf_u;					\
  gf_u.value = (d);						\
  (i) = gf_u.word;						\
} while (0)
#endif

/* Set a float from a 32 bit int.  */
#ifndef SET_FLOAT_WORD
# define SET_FLOAT_WORD(d,i)					\
do {								\
  ieee_float_shape_type sf_u;					\
  sf_u.word = (i);						\
  (d) = sf_u.value;						\
} while (0)
#endif

/* Get long double macros from a separate header.  */
#include <math_ldbl.h>

/* ieee style elementary functions */
extern double __ieee754_sqrt (double);
extern double __ieee754_acos (double);
extern double __ieee754_acosh (double);
extern double __ieee754_log (double);
extern double __ieee754_atanh (double);
extern double __ieee754_asin (double);
extern double __ieee754_atan2 (double,double);
extern double __ieee754_exp (double);
extern double __ieee754_exp2 (double);
extern double __ieee754_exp10 (double);
extern double __ieee754_cosh (double);
extern double __ieee754_fmod (double,double);
extern double __ieee754_pow (double,double);
extern double __ieee754_lgamma_r (double,int *);
extern double __ieee754_gamma_r (double,int *);
extern double __ieee754_lgamma (double);
extern double __ieee754_gamma (double);
extern double __ieee754_log10 (double);
extern double __ieee754_log2 (double);
extern double __ieee754_sinh (double);
extern double __ieee754_hypot (double,double);
extern double __ieee754_j0 (double);
extern double __ieee754_j1 (double);
extern double __ieee754_y0 (double);
extern double __ieee754_y1 (double);
extern double __ieee754_jn (int,double);
extern double __ieee754_yn (int,double);
extern double __ieee754_remainder (double,double);
extern int32_t __ieee754_rem_pio2 (double,double*);
extern double __ieee754_scalb (double,double);

/* fdlibm kernel function */
extern double __kernel_standard (double,double,int);
extern float __kernel_standard_f (float,float,int);
extern double __kernel_sin (double,double,int);
extern double __kernel_cos (double,double);
extern double __kernel_tan (double,double,int);
extern int    __kernel_rem_pio2 (double*,double*,int,int,int, const int32_t*);

/* internal functions.  */
extern double __copysign (double x, double __y);

extern inline double __copysign (double x, double y)
{ return __builtin_copysign (x, y); }

/* ieee style elementary float functions */
extern float __ieee754_sqrtf (float);
extern float __ieee754_acosf (float);
extern float __ieee754_acoshf (float);
extern float __ieee754_logf (float);
extern float __ieee754_atanhf (float);
extern float __ieee754_asinf (float);
extern float __ieee754_atan2f (float,float);
extern float __ieee754_expf (float);
extern float __ieee754_exp2f (float);
extern float __ieee754_exp10f (float);
extern float __ieee754_coshf (float);
extern float __ieee754_fmodf (float,float);
extern float __ieee754_powf (float,float);
extern float __ieee754_lgammaf_r (float,int *);
extern float __ieee754_gammaf_r (float,int *);
extern float __ieee754_lgammaf (float);
extern float __ieee754_gammaf (float);
extern float __ieee754_log10f (float);
extern float __ieee754_log2f (float);
extern float __ieee754_sinhf (float);
extern float __ieee754_hypotf (float,float);
extern float __ieee754_j0f (float);
extern float __ieee754_j1f (float);
extern float __ieee754_y0f (float);
extern float __ieee754_y1f (float);
extern float __ieee754_jnf (int,float);
extern float __ieee754_ynf (int,float);
extern float __ieee754_remainderf (float,float);
extern int32_t __ieee754_rem_pio2f (float,float*);
extern float __ieee754_scalbf (float,float);


/* float versions of fdlibm kernel functions */
extern float __kernel_sinf (float,float,int);
extern float __kernel_cosf (float,float);
extern float __kernel_tanf (float,float,int);
extern int   __kernel_rem_pio2f (float*,float*,int,int,int, const int32_t*);

/* internal functions.  */
extern float __copysignf (float x, float __y);

extern inline float __copysignf (float x, float y)
{ return __builtin_copysignf (x, y); }

/* ieee style elementary long double functions */
extern long double __ieee754_sqrtl (long double);
extern long double __ieee754_acosl (long double);
extern long double __ieee754_acoshl (long double);
extern long double __ieee754_logl (long double);
extern long double __ieee754_atanhl (long double);
extern long double __ieee754_asinl (long double);
extern long double __ieee754_atan2l (long double,long double);
extern long double __ieee754_expl (long double);
extern long double __ieee754_exp2l (long double);
extern long double __ieee754_exp10l (long double);
extern long double __ieee754_coshl (long double);
extern long double __ieee754_fmodl (long double,long double);
extern long double __ieee754_powl (long double,long double);
extern long double __ieee754_lgammal_r (long double,int *);
extern long double __ieee754_gammal_r (long double,int *);
extern long double __ieee754_lgammal (long double);
extern long double __ieee754_gammal (long double);
extern long double __ieee754_log10l (long double);
extern long double __ieee754_log2l (long double);
extern long double __ieee754_sinhl (long double);
extern long double __ieee754_hypotl (long double,long double);
extern long double __ieee754_j0l (long double);
extern long double __ieee754_j1l (long double);
extern long double __ieee754_y0l (long double);
extern long double __ieee754_y1l (long double);
extern long double __ieee754_jnl (int,long double);
extern long double __ieee754_ynl (int,long double);
extern long double __ieee754_remainderl (long double,long double);
extern int   __ieee754_rem_pio2l (long double,long double*);
extern long double __ieee754_scalbl (long double,long double);

/* long double versions of fdlibm kernel functions */
extern long double __kernel_sinl (long double,long double,int);
extern long double __kernel_cosl (long double,long double);
extern long double __kernel_tanl (long double,long double,int);
extern void __kernel_sincosl (long double,long double,
			      long double *,long double *, int);
extern int   __kernel_rem_pio2l (long double*,long double*,int,int,
				 int,const int*);

#ifndef NO_LONG_DOUBLE
/* prototypes required to compile the ldbl-96 support without warnings */
extern int __finitel (long double);
extern int __ilogbl (long double);
extern int __isinfl (long double);
extern int __isnanl (long double);
extern long double __atanl (long double);
extern long double __copysignl (long double, long double);
extern long double __expm1l (long double);
extern long double __floorl (long double);
extern long double __frexpl (long double, int *);
extern long double __ldexpl (long double, int);
extern long double __log1pl (long double);
extern long double __nanl (const char *);
extern long double __rintl (long double);
extern long double __scalbnl (long double, int);
extern long double __sqrtl (long double x);
extern long double fabsl (long double x);
extern void __sincosl (long double, long double *, long double *);
extern long double __logbl (long double x);
extern long double __significandl (long double x);

extern inline long double __copysignl (long double x, long double y)
{ return __builtin_copysignl (x, y); }

#endif

/* Prototypes for functions of the IBM Accurate Mathematical Library.  */
extern double __exp1 (double __x, double __xx, double __error);
extern double __sin (double __x);
extern double __cos (double __x);
extern int __branred (double __x, double *__a, double *__aa);
extern void __doasin (double __x, double __dx, double __v[]);
extern void __dubsin (double __x, double __dx, double __v[]);
extern void __dubcos (double __x, double __dx, double __v[]);
extern double __halfulp (double __x, double __y);
extern double __sin32 (double __x, double __res, double __res1);
extern double __cos32 (double __x, double __res, double __res1);
extern double __mpsin (double __x, double __dx);
extern double __mpcos (double __x, double __dx);
extern double __mpsin1 (double __x);
extern double __mpcos1 (double __x);
extern double __slowexp (double __x);
extern double __slowpow (double __x, double __y, double __z);
extern void __docos (double __x, double __dx, double __v[]);

#ifndef math_opt_barrier
# define math_opt_barrier(x) \
({ __typeof (x) __x = (x); __asm ("" : "+m" (__x)); __x; })
# define math_force_eval(x) \
({ __typeof (x) __x = (x); __asm __volatile__ ("" : : "m" (__x)); })
#endif


/* The standards only specify one variant of the fenv.h interfaces.
   But at least for some architectures we can be more efficient if we
   know what operations are going to be performed.  Therefore we
   define additional interfaces.  By default they refer to the normal
   interfaces.  */

static __always_inline void
default_libc_feholdexcept (fenv_t *e)
{
  (void) feholdexcept (e);
}

#ifndef libc_feholdexcept
# define libc_feholdexcept  default_libc_feholdexcept
#endif
#ifndef libc_feholdexceptf
# define libc_feholdexceptf default_libc_feholdexcept
#endif
#ifndef libc_feholdexceptl
# define libc_feholdexceptl default_libc_feholdexcept
#endif

static __always_inline void
default_libc_feholdexcept_setround (fenv_t *e, int r)
{
  feholdexcept (e);
  fesetround (r);
}

#ifndef libc_feholdexcept_setround
# define libc_feholdexcept_setround  default_libc_feholdexcept_setround
#endif
#ifndef libc_feholdexcept_setroundf
# define libc_feholdexcept_setroundf default_libc_feholdexcept_setround
#endif
#ifndef libc_feholdexcept_setroundl
# define libc_feholdexcept_setroundl default_libc_feholdexcept_setround
#endif

#ifndef libc_feholdexcept_setround_53bit
# define libc_feholdexcept_setround_53bit libc_feholdexcept_setround
#endif

#ifndef libc_fetestexcept
# define libc_fetestexcept  fetestexcept
#endif
#ifndef libc_fetestexceptf
# define libc_fetestexceptf fetestexcept
#endif
#ifndef libc_fetestexceptl
# define libc_fetestexceptl fetestexcept
#endif

static __always_inline void
default_libc_fesetenv (fenv_t *e)
{
  (void) fesetenv (e);
}

#ifndef libc_fesetenv
# define libc_fesetenv  default_libc_fesetenv
#endif
#ifndef libc_fesetenvf
# define libc_fesetenvf default_libc_fesetenv
#endif
#ifndef libc_fesetenvl
# define libc_fesetenvl default_libc_fesetenv
#endif

static __always_inline void
default_libc_feupdateenv (fenv_t *e)
{
  (void) feupdateenv (e);
}

#ifndef libc_feupdateenv
# define libc_feupdateenv  default_libc_feupdateenv
#endif
#ifndef libc_feupdateenvf
# define libc_feupdateenvf default_libc_feupdateenv
#endif
#ifndef libc_feupdateenvl
# define libc_feupdateenvl default_libc_feupdateenv
#endif

#ifndef libc_feupdateenv_53bit
# define libc_feupdateenv_53bit libc_feupdateenv
#endif

static __always_inline int
default_libc_feupdateenv_test (fenv_t *e, int ex)
{
  int ret = fetestexcept (ex);
  feupdateenv (e);
  return ret;
}

#ifndef libc_feupdateenv_test
# define libc_feupdateenv_test  default_libc_feupdateenv_test
#endif
#ifndef libc_feupdateenv_testf
# define libc_feupdateenv_testf default_libc_feupdateenv_test
#endif
#ifndef libc_feupdateenv_testl
# define libc_feupdateenv_testl default_libc_feupdateenv_test
#endif

/* Save and set the rounding mode.  The use of fenv_t to store the old mode
   allows a target-specific version of this function to avoid converting the
   rounding mode from the fpu format.  By default we have no choice but to
   manipulate the entire env.  */

#ifndef libc_feholdsetround
# define libc_feholdsetround  libc_feholdexcept_setround
#endif
#ifndef libc_feholdsetroundf
# define libc_feholdsetroundf libc_feholdexcept_setroundf
#endif
#ifndef libc_feholdsetroundl
# define libc_feholdsetroundl libc_feholdexcept_setroundl
#endif

/* ... and the reverse.  */

#ifndef libc_feresetround
# define libc_feresetround  libc_feupdateenv
#endif
#ifndef libc_feresetroundf
# define libc_feresetroundf libc_feupdateenvf
#endif
#ifndef libc_feresetroundl
# define libc_feresetroundl libc_feupdateenvl
#endif

/* ... and a version that may also discard exceptions.  */

#ifndef libc_feresetround_noex
# define libc_feresetround_noex  libc_fesetenv
#endif
#ifndef libc_feresetround_noexf
# define libc_feresetround_noexf libc_fesetenvf
#endif
#ifndef libc_feresetround_noexl
# define libc_feresetround_noexl libc_fesetenvl
#endif

/* Save and restore the rounding mode within a lexical block.  */

#define SET_RESTORE_ROUND(RM) \
  fenv_t __libc_save_rm __attribute__((cleanup(libc_feresetround)));	\
  libc_feholdsetround (&__libc_save_rm, (RM))
#define SET_RESTORE_ROUNDF(RM) \
  fenv_t __libc_save_rm __attribute__((cleanup(libc_feresetroundf)));	\
  libc_feholdsetroundf (&__libc_save_rm, (RM))
#define SET_RESTORE_ROUNDL(RM) \
  fenv_t __libc_save_rm __attribute__((cleanup(libc_feresetroundl)));	\
  libc_feholdsetroundl (&__libc_save_rm, (RM))

/* Save and restore the rounding mode within a lexical block, and also
   the set of exceptions raised within the block may be discarded.  */

#define SET_RESTORE_ROUND_NOEX(RM) \
  fenv_t __libc_save_rm __attribute__((cleanup(libc_feresetround_noex))); \
  libc_feholdsetround (&__libc_save_rm, (RM))
#define SET_RESTORE_ROUND_NOEXF(RM) \
  fenv_t __libc_save_rm __attribute__((cleanup(libc_feresetround_noexf))); \
  libc_feholdsetroundf (&__libc_save_rm, (RM))
#define SET_RESTORE_ROUND_NOEXL(RM) \
  fenv_t __libc_save_rm __attribute__((cleanup(libc_feresetround_noexl))); \
  libc_feholdsetroundl (&__libc_save_rm, (RM))

/* Like SET_RESTORE_ROUND, but also set rounding precision to 53 bits.  */
#define SET_RESTORE_ROUND_53BIT(RM) \
  fenv_t __libc_save_rm __attribute__((cleanup(libc_feupdateenv_53bit))); \
  libc_feholdexcept_setround_53bit (&__libc_save_rm, (RM))

#define __nan(str) \
  (__builtin_constant_p (str) && str[0] == '\0' ? NAN : __nan (str))
#define __nanf(str) \
  (__builtin_constant_p (str) && str[0] == '\0' ? NAN : __nan (str))
#define __nanl(str) \
  (__builtin_constant_p (str) && str[0] == '\0' ? NAN : __nan (str))

#endif /* _MATH_PRIVATE_H_ */
