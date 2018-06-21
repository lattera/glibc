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
#include <get-rounding-mode.h>

/* Gather machine dependent _Floatn support.  */
#include <bits/floatn.h>

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

#if __FLOAT_WORD_ORDER == __BIG_ENDIAN

typedef union
{
  double value;
  struct
  {
    uint32_t msw;
    uint32_t lsw;
  } parts;
  uint64_t word;
} ieee_double_shape_type;

#endif

#if __FLOAT_WORD_ORDER == __LITTLE_ENDIAN

typedef union
{
  double value;
  struct
  {
    uint32_t lsw;
    uint32_t msw;
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
  uint32_t word;
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

/* We need to guarantee an expansion of name when building
   ldbl-128 files as another type (e.g _Float128).  */
#define mathx_hidden_def(name) hidden_def(name)

/* Get long double macros from a separate header.  */
#include <math_ldbl.h>

/* Include function declarations for each floating-point.  */
#define _Mdouble_ double
#define _MSUF_
#include <math_private_calls.h>
#undef _MSUF_
#undef _Mdouble_

#define _Mdouble_ float
#define _MSUF_ f
#define __MATH_DECLARING_FLOAT
#include <math_private_calls.h>
#undef __MATH_DECLARING_FLOAT
#undef _MSUF_
#undef _Mdouble_

#define _Mdouble_ long double
#define _MSUF_ l
#define __MATH_DECLARING_LONG_DOUBLE
#include <math_private_calls.h>
#undef __MATH_DECLARING_LONG_DOUBLE
#undef _MSUF_
#undef _Mdouble_

#if __HAVE_DISTINCT_FLOAT128
# define _Mdouble_ _Float128
# define _MSUF_ f128
# define __MATH_DECLARING_FLOATN
# include <math_private_calls.h>
# undef __MATH_DECLARING_FLOATN
# undef _MSUF_
# undef _Mdouble_
#endif

#if __HAVE_DISTINCT_FLOAT128

/* __builtin_isinf_sign is broken in GCC < 7 for float128.  */
# if ! __GNUC_PREREQ (7, 0)
#  include <ieee754_float128.h>
extern inline int
__isinff128 (_Float128 x)
{
  int64_t hx, lx;
  GET_FLOAT128_WORDS64 (hx, lx, x);
  lx |= (hx & 0x7fffffffffffffffLL) ^ 0x7fff000000000000LL;
  lx |= -lx;
  return ~(lx >> 63) & (hx >> 62);
}
# endif

extern inline _Float128
fabsf128 (_Float128 x)
{
  return __builtin_fabsf128 (x);
}
#endif



/* Prototypes for functions of the IBM Accurate Mathematical Library.  */
extern double __exp1 (double __x, double __xx);
extern double __sin (double __x);
extern double __cos (double __x);
extern int __branred (double __x, double *__a, double *__aa);
extern void __doasin (double __x, double __dx, double __v[]);
extern void __dubsin (double __x, double __dx, double __v[]);
extern void __dubcos (double __x, double __dx, double __v[]);
extern double __sin32 (double __x, double __res, double __res1);
extern double __cos32 (double __x, double __res, double __res1);
extern double __mpsin (double __x, double __dx, bool __range_reduce);
extern double __mpcos (double __x, double __dx, bool __range_reduce);
extern void __docos (double __x, double __dx, double __v[]);

/* The standards only specify one variant of the fenv.h interfaces.
   But at least for some architectures we can be more efficient if we
   know what operations are going to be performed.  Therefore we
   define additional interfaces.  By default they refer to the normal
   interfaces.  */

static __always_inline void
default_libc_feholdexcept (fenv_t *e)
{
  (void) __feholdexcept (e);
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
default_libc_fesetround (int r)
{
  (void) __fesetround (r);
}

#ifndef libc_fesetround
# define libc_fesetround  default_libc_fesetround
#endif
#ifndef libc_fesetroundf
# define libc_fesetroundf default_libc_fesetround
#endif
#ifndef libc_fesetroundl
# define libc_fesetroundl default_libc_fesetround
#endif

static __always_inline void
default_libc_feholdexcept_setround (fenv_t *e, int r)
{
  __feholdexcept (e);
  __fesetround (r);
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

#ifndef libc_feholdsetround_53bit
# define libc_feholdsetround_53bit libc_feholdsetround
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
  (void) __fesetenv (e);
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
  (void) __feupdateenv (e);
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

#ifndef libc_feresetround_53bit
# define libc_feresetround_53bit libc_feresetround
#endif

static __always_inline int
default_libc_feupdateenv_test (fenv_t *e, int ex)
{
  int ret = fetestexcept (ex);
  __feupdateenv (e);
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

/* ... and a version that also discards exceptions.  */

#ifndef libc_feresetround_noex
# define libc_feresetround_noex  libc_fesetenv
#endif
#ifndef libc_feresetround_noexf
# define libc_feresetround_noexf libc_fesetenvf
#endif
#ifndef libc_feresetround_noexl
# define libc_feresetround_noexl libc_fesetenvl
#endif

#ifndef HAVE_RM_CTX
# define HAVE_RM_CTX 0
#endif


/* Default implementation using standard fenv functions.
   Avoid unnecessary rounding mode changes by first checking the
   current rounding mode.  Note the use of __glibc_unlikely is
   important for performance.  */

static __always_inline void
default_libc_feholdsetround_ctx (struct rm_ctx *ctx, int round)
{
  ctx->updated_status = false;

  /* Update rounding mode only if different.  */
  if (__glibc_unlikely (round != get_rounding_mode ()))
    {
      ctx->updated_status = true;
      __fegetenv (&ctx->env);
      __fesetround (round);
    }
}

static __always_inline void
default_libc_feresetround_ctx (struct rm_ctx *ctx)
{
  /* Restore the rounding mode if updated.  */
  if (__glibc_unlikely (ctx->updated_status))
    __feupdateenv (&ctx->env);
}

static __always_inline void
default_libc_feholdsetround_noex_ctx (struct rm_ctx *ctx, int round)
{
  /* Save exception flags and rounding mode, and disable exception
     traps.  */
  __feholdexcept (&ctx->env);

  /* Update rounding mode only if different.  */
  if (__glibc_unlikely (round != get_rounding_mode ()))
    __fesetround (round);
}

static __always_inline void
default_libc_feresetround_noex_ctx (struct rm_ctx *ctx)
{
  /* Restore exception flags and rounding mode.  */
  __fesetenv (&ctx->env);
}

#if HAVE_RM_CTX
/* Set/Restore Rounding Modes only when necessary.  If defined, these functions
   set/restore floating point state only if the state needed within the lexical
   block is different from the current state.  This saves a lot of time when
   the floating point unit is much slower than the fixed point units.  */

# ifndef libc_feholdsetround_noex_ctx
#   define libc_feholdsetround_noex_ctx  libc_feholdsetround_ctx
# endif
# ifndef libc_feholdsetround_noexf_ctx
#   define libc_feholdsetround_noexf_ctx libc_feholdsetroundf_ctx
# endif
# ifndef libc_feholdsetround_noexl_ctx
#   define libc_feholdsetround_noexl_ctx libc_feholdsetroundl_ctx
# endif

# ifndef libc_feresetround_noex_ctx
#   define libc_feresetround_noex_ctx  libc_fesetenv_ctx
# endif
# ifndef libc_feresetround_noexf_ctx
#   define libc_feresetround_noexf_ctx libc_fesetenvf_ctx
# endif
# ifndef libc_feresetround_noexl_ctx
#   define libc_feresetround_noexl_ctx libc_fesetenvl_ctx
# endif

#else

# define libc_feholdsetround_ctx      default_libc_feholdsetround_ctx
# define libc_feresetround_ctx        default_libc_feresetround_ctx
# define libc_feholdsetround_noex_ctx default_libc_feholdsetround_noex_ctx
# define libc_feresetround_noex_ctx   default_libc_feresetround_noex_ctx

# define libc_feholdsetroundf_ctx libc_feholdsetround_ctx
# define libc_feholdsetroundl_ctx libc_feholdsetround_ctx
# define libc_feresetroundf_ctx   libc_feresetround_ctx
# define libc_feresetroundl_ctx   libc_feresetround_ctx

# define libc_feholdsetround_noexf_ctx libc_feholdsetround_noex_ctx
# define libc_feholdsetround_noexl_ctx libc_feholdsetround_noex_ctx
# define libc_feresetround_noexf_ctx   libc_feresetround_noex_ctx
# define libc_feresetround_noexl_ctx   libc_feresetround_noex_ctx

#endif

#ifndef libc_feholdsetround_53bit_ctx
#  define libc_feholdsetround_53bit_ctx libc_feholdsetround_ctx
#endif
#ifndef libc_feresetround_53bit_ctx
#  define libc_feresetround_53bit_ctx libc_feresetround_ctx
#endif

#define SET_RESTORE_ROUND_GENERIC(RM,ROUNDFUNC,CLEANUPFUNC) \
  struct rm_ctx ctx __attribute__((cleanup (CLEANUPFUNC ## _ctx))); \
  ROUNDFUNC ## _ctx (&ctx, (RM))

/* Set the rounding mode within a lexical block.  Restore the rounding mode to
   the value at the start of the block.  The exception mode must be preserved.
   Exceptions raised within the block must be set in the exception flags.
   Non-stop mode may be enabled inside the block.  */

#define SET_RESTORE_ROUND(RM) \
  SET_RESTORE_ROUND_GENERIC (RM, libc_feholdsetround, libc_feresetround)
#define SET_RESTORE_ROUNDF(RM) \
  SET_RESTORE_ROUND_GENERIC (RM, libc_feholdsetroundf, libc_feresetroundf)
#define SET_RESTORE_ROUNDL(RM) \
  SET_RESTORE_ROUND_GENERIC (RM, libc_feholdsetroundl, libc_feresetroundl)

/* Set the rounding mode within a lexical block.  Restore the rounding mode to
   the value at the start of the block.  The exception mode must be preserved.
   Exceptions raised within the block must be discarded, and exception flags
   are restored to the value at the start of the block.
   Non-stop mode must be enabled inside the block.  */

#define SET_RESTORE_ROUND_NOEX(RM) \
  SET_RESTORE_ROUND_GENERIC (RM, libc_feholdsetround_noex, \
			     libc_feresetround_noex)
#define SET_RESTORE_ROUND_NOEXF(RM) \
  SET_RESTORE_ROUND_GENERIC (RM, libc_feholdsetround_noexf, \
			     libc_feresetround_noexf)
#define SET_RESTORE_ROUND_NOEXL(RM) \
  SET_RESTORE_ROUND_GENERIC (RM, libc_feholdsetround_noexl, \
			     libc_feresetround_noexl)

/* Like SET_RESTORE_ROUND, but also set rounding precision to 53 bits.  */
#define SET_RESTORE_ROUND_53BIT(RM) \
  SET_RESTORE_ROUND_GENERIC (RM, libc_feholdsetround_53bit,	      \
			     libc_feresetround_53bit)

/* When no floating-point exceptions are defined in <fenv.h>, make
   feraiseexcept ignore its argument so that unconditional
   feraiseexcept calls do not cause errors for undefined exceptions.
   Define it to expand to a void expression so that any calls testing
   the result of feraiseexcept do produce errors.  */
#if FE_ALL_EXCEPT == 0
# define feraiseexcept(excepts) ((void) 0)
# define __feraiseexcept(excepts) ((void) 0)
#endif

/* Similarly, most <fenv.h> functions have trivial implementations in
   the absence of support for floating-point exceptions and rounding
   modes.  */

#if !FE_HAVE_ROUNDING_MODES
# if FE_ALL_EXCEPT == 0
extern inline int
fegetenv (fenv_t *__e)
{
  return 0;
}

extern inline int
__fegetenv (fenv_t *__e)
{
  return 0;
}

extern inline int
feholdexcept (fenv_t *__e)
{
  return 0;
}

extern inline int
__feholdexcept (fenv_t *__e)
{
  return 0;
}

extern inline int
fesetenv (const fenv_t *__e)
{
  return 0;
}

extern inline int
__fesetenv (const fenv_t *__e)
{
  return 0;
}

extern inline int
feupdateenv (const fenv_t *__e)
{
  return 0;
}

extern inline int
__feupdateenv (const fenv_t *__e)
{
  return 0;
}
# endif

extern inline int
fegetround (void)
{
  return FE_TONEAREST;
}

extern inline int
__fegetround (void)
{
  return FE_TONEAREST;
}

extern inline int
fesetround (int __d)
{
  return 0;
}

extern inline int
__fesetround (int __d)
{
  return 0;
}
#endif

#endif /* _MATH_PRIVATE_H_ */
