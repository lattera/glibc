/* Declarations for math functions.
   Copyright (C) 1991-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/*
 *	ISO C99 Standard: 7.12 Mathematics	<math.h>
 */

#ifndef	_MATH_H
#define	_MATH_H	1

#define __GLIBC_INTERNAL_STARTING_HEADER_IMPLEMENTATION
#include <bits/libc-header-start.h>

__BEGIN_DECLS

/* Get definitions of __intmax_t and __uintmax_t.  */
#include <bits/types.h>

/* Get machine-dependent vector math functions declarations.  */
#include <bits/math-vector.h>

/* Get machine-dependent HUGE_VAL value (returned on overflow).
   On all IEEE754 machines, this is +Infinity.  */
#include <bits/huge_val.h>
#ifdef __USE_ISOC99
# include <bits/huge_valf.h>
# include <bits/huge_vall.h>

/* Get machine-dependent INFINITY value.  */
# include <bits/inf.h>

/* Get machine-dependent NAN value (returned for some domain errors).  */
# include <bits/nan.h>
#endif /* __USE_ISOC99 */

#if __GLIBC_USE (IEC_60559_BFP_EXT)
/* Signaling NaN macros, if supported.  */
# if __GNUC_PREREQ (3, 3)
#  define SNANF (__builtin_nansf (""))
#  define SNAN (__builtin_nans (""))
#  define SNANL (__builtin_nansl (""))
# endif
#endif

/* Get __GLIBC_FLT_EVAL_METHOD.  */
#include <bits/flt-eval-method.h>

#ifdef __USE_ISOC99
/* Define the following typedefs.

    float_t	floating-point type at least as wide as `float' used
		to evaluate `float' expressions
    double_t	floating-point type at least as wide as `double' used
		to evaluate `double' expressions
*/
# if __GLIBC_FLT_EVAL_METHOD == 0 || __GLIBC_FLT_EVAL_METHOD == 16
typedef float float_t;
typedef double double_t;
# elif __GLIBC_FLT_EVAL_METHOD == 1
typedef double float_t;
typedef double double_t;
# elif __GLIBC_FLT_EVAL_METHOD == 2
typedef long double float_t;
typedef long double double_t;
# elif __GLIBC_FLT_EVAL_METHOD == 32
typedef _Float32 float_t;
typedef double double_t;
# elif __GLIBC_FLT_EVAL_METHOD == 33
typedef _Float32x float_t;
typedef _Float32x double_t;
# elif __GLIBC_FLT_EVAL_METHOD == 64
typedef _Float64 float_t;
typedef _Float64 double_t;
# elif __GLIBC_FLT_EVAL_METHOD == 65
typedef _Float64x float_t;
typedef _Float64x double_t;
# elif __GLIBC_FLT_EVAL_METHOD == 128
typedef _Float128 float_t;
typedef _Float128 double_t;
# elif __GLIBC_FLT_EVAL_METHOD == 129
typedef _Float128x float_t;
typedef _Float128x double_t;
# else
#  error "Unknown __GLIBC_FLT_EVAL_METHOD"
# endif
#endif

/* Define macros for the return values of ilogb and llogb, based on
   __FP_LOGB0_IS_MIN and __FP_LOGBNAN_IS_MIN.

    FP_ILOGB0	Expands to a value returned by `ilogb (0.0)'.
    FP_ILOGBNAN	Expands to a value returned by `ilogb (NAN)'.
    FP_LLOGB0	Expands to a value returned by `llogb (0.0)'.
    FP_LLOGBNAN	Expands to a value returned by `llogb (NAN)'.

*/

#include <bits/fp-logb.h>
#ifdef __USE_ISOC99
# if __FP_LOGB0_IS_MIN
#  define FP_ILOGB0	(-2147483647 - 1)
# else
#  define FP_ILOGB0	(-2147483647)
# endif
# if __FP_LOGBNAN_IS_MIN
#  define FP_ILOGBNAN	(-2147483647 - 1)
# else
#  define FP_ILOGBNAN	2147483647
# endif
#endif
#if __GLIBC_USE (IEC_60559_BFP_EXT)
# if __WORDSIZE == 32
#  define __FP_LONG_MAX 0x7fffffffL
# else
#  define __FP_LONG_MAX 0x7fffffffffffffffL
# endif
# if __FP_LOGB0_IS_MIN
#  define FP_LLOGB0	(-__FP_LONG_MAX - 1)
# else
#  define FP_LLOGB0	(-__FP_LONG_MAX)
# endif
# if __FP_LOGBNAN_IS_MIN
#  define FP_LLOGBNAN	(-__FP_LONG_MAX - 1)
# else
#  define FP_LLOGBNAN	__FP_LONG_MAX
# endif
#endif

/* Get the architecture specific values describing the floating-point
   evaluation.  The following symbols will get defined:

    FP_FAST_FMA
    FP_FAST_FMAF
    FP_FAST_FMAL
		If defined it indicates that the `fma' function
		generally executes about as fast as a multiply and an add.
		This macro is defined only iff the `fma' function is
		implemented directly with a hardware multiply-add instructions.
*/

#include <bits/fp-fast.h>

#if __GLIBC_USE (IEC_60559_BFP_EXT)
/* Rounding direction macros for fromfp functions.  */
enum
  {
    FP_INT_UPWARD =
# define FP_INT_UPWARD 0
      FP_INT_UPWARD,
    FP_INT_DOWNWARD =
# define FP_INT_DOWNWARD 1
      FP_INT_DOWNWARD,
    FP_INT_TOWARDZERO =
# define FP_INT_TOWARDZERO 2
      FP_INT_TOWARDZERO,
    FP_INT_TONEARESTFROMZERO =
# define FP_INT_TONEARESTFROMZERO 3
      FP_INT_TONEARESTFROMZERO,
    FP_INT_TONEAREST =
# define FP_INT_TONEAREST 4
      FP_INT_TONEAREST,
  };
#endif

/* The file <bits/mathcalls.h> contains the prototypes for all the
   actual math functions.  These macros are used for those prototypes,
   so we can easily declare each function as both `name' and `__name',
   and can declare the float versions `namef' and `__namef'.  */

#define __SIMD_DECL(function) __CONCAT (__DECL_SIMD_, function)

#define __MATHCALL_VEC(function, suffix, args) 	\
  __SIMD_DECL (__MATH_PRECNAME (function, suffix)) \
  __MATHCALL (function, suffix, args)

#define __MATHDECL_VEC(type, function,suffix, args) \
  __SIMD_DECL (__MATH_PRECNAME (function, suffix)) \
  __MATHDECL(type, function,suffix, args)

#define __MATHCALL(function,suffix, args)	\
  __MATHDECL (_Mdouble_,function,suffix, args)
#define __MATHDECL(type, function,suffix, args) \
  __MATHDECL_1(type, function,suffix, args); \
  __MATHDECL_1(type, __CONCAT(__,function),suffix, args)
#define __MATHCALLX(function,suffix, args, attrib)	\
  __MATHDECLX (_Mdouble_,function,suffix, args, attrib)
#define __MATHDECLX(type, function,suffix, args, attrib) \
  __MATHDECL_1(type, function,suffix, args) __attribute__ (attrib); \
  __MATHDECL_1(type, __CONCAT(__,function),suffix, args) __attribute__ (attrib)
#define __MATHDECL_1(type, function,suffix, args) \
  extern type __MATH_PRECNAME(function,suffix) args __THROW

#define _Mdouble_		double
#define __MATH_PRECNAME(name,r)	__CONCAT(name,r)
#define __MATH_DECLARING_DOUBLE  1
#define _Mdouble_BEGIN_NAMESPACE __BEGIN_NAMESPACE_STD
#define _Mdouble_END_NAMESPACE   __END_NAMESPACE_STD
#include <bits/mathcalls.h>
#undef	_Mdouble_
#undef _Mdouble_BEGIN_NAMESPACE
#undef _Mdouble_END_NAMESPACE
#undef	__MATH_PRECNAME
#undef __MATH_DECLARING_DOUBLE

#ifdef __USE_ISOC99


/* Include the file of declarations again, this time using `float'
   instead of `double' and appending f to each function name.  */

# ifndef _Mfloat_
#  define _Mfloat_		float
# endif
# define _Mdouble_		_Mfloat_
# define __MATH_PRECNAME(name,r) name##f##r
# define __MATH_DECLARING_DOUBLE  0
# define _Mdouble_BEGIN_NAMESPACE __BEGIN_NAMESPACE_C99
# define _Mdouble_END_NAMESPACE   __END_NAMESPACE_C99
# include <bits/mathcalls.h>
# undef	_Mdouble_
# undef _Mdouble_BEGIN_NAMESPACE
# undef _Mdouble_END_NAMESPACE
# undef	__MATH_PRECNAME
# undef __MATH_DECLARING_DOUBLE

# if !(defined __NO_LONG_DOUBLE_MATH && defined _LIBC) \
     || defined __LDBL_COMPAT \
     || defined _LIBC_TEST
#  ifdef __LDBL_COMPAT

#   ifdef __USE_ISOC99
extern float __nldbl_nexttowardf (float __x, long double __y)
				  __THROW __attribute__ ((__const__));
#    ifdef __REDIRECT_NTH
extern float __REDIRECT_NTH (nexttowardf, (float __x, long double __y),
			     __nldbl_nexttowardf)
     __attribute__ ((__const__));
extern double __REDIRECT_NTH (nexttoward, (double __x, long double __y),
			      nextafter) __attribute__ ((__const__));
extern long double __REDIRECT_NTH (nexttowardl,
				   (long double __x, long double __y),
				   nextafter) __attribute__ ((__const__));
#    endif
#   endif

#   undef __MATHDECL_1
#   define __MATHDECL_2(type, function,suffix, args, alias) \
  extern type __REDIRECT_NTH(__MATH_PRECNAME(function,suffix), \
			     args, alias)
#   define __MATHDECL_1(type, function,suffix, args) \
  __MATHDECL_2(type, function,suffix, args, __CONCAT(function,suffix))
#  endif

/* Include the file of declarations again, this time using `long double'
   instead of `double' and appending l to each function name.  */

#  ifndef _Mlong_double_
#   define _Mlong_double_	long double
#  endif
#  define _Mdouble_		_Mlong_double_
#  define __MATH_PRECNAME(name,r) name##l##r
#  define __MATH_DECLARING_DOUBLE  0
#  define _Mdouble_BEGIN_NAMESPACE __BEGIN_NAMESPACE_C99
#  define _Mdouble_END_NAMESPACE   __END_NAMESPACE_C99
#  define __MATH_DECLARE_LDOUBLE   1
#  include <bits/mathcalls.h>
#  undef _Mdouble_
#  undef _Mdouble_BEGIN_NAMESPACE
#  undef _Mdouble_END_NAMESPACE
#  undef __MATH_PRECNAME
#  undef __MATH_DECLARING_DOUBLE

# endif /* !(__NO_LONG_DOUBLE_MATH && _LIBC) || __LDBL_COMPAT */

#endif	/* Use ISO C99.  */
#undef	__MATHDECL_1
#undef	__MATHDECL
#undef	__MATHCALL


#if defined __USE_MISC || defined __USE_XOPEN
/* This variable is used by `gamma' and `lgamma'.  */
extern int signgam;
#endif


/* Depending on the type of TG_ARG, call an appropriately suffixed
   version of FUNC with arguments (including parentheses) ARGS.
   Suffixed functions may not exist for long double if it has the same
   format as double, or for other types with the same format as float,
   double or long double.  The behavior is undefined if the argument
   does not have a real floating type.  The definition may use a
   conditional expression, so all suffixed versions of FUNC must
   return the same type (FUNC may include a cast if necessary rather
   than being a single identifier).  */
#ifdef __NO_LONG_DOUBLE_MATH
# define __MATH_TG(TG_ARG, FUNC, ARGS)					\
  (sizeof (TG_ARG) == sizeof (float) ? FUNC ## f ARGS : FUNC ARGS)
#else
# define __MATH_TG(TG_ARG, FUNC, ARGS)		\
  (sizeof (TG_ARG) == sizeof (float)		\
   ? FUNC ## f ARGS				\
   : sizeof (TG_ARG) == sizeof (double)		\
   ? FUNC ARGS					\
   : FUNC ## l ARGS)
#endif

/* ISO C99 defines some generic macros which work on any data type.  */
#ifdef __USE_ISOC99

/* All floating-point numbers can be put in one of these categories.  */
enum
  {
    FP_NAN =
# define FP_NAN 0
      FP_NAN,
    FP_INFINITE =
# define FP_INFINITE 1
      FP_INFINITE,
    FP_ZERO =
# define FP_ZERO 2
      FP_ZERO,
    FP_SUBNORMAL =
# define FP_SUBNORMAL 3
      FP_SUBNORMAL,
    FP_NORMAL =
# define FP_NORMAL 4
      FP_NORMAL
  };

/* GCC bug 66462 means we cannot use the math builtins with -fsignaling-nan,
   so disable builtins if this is enabled.  When fixed in a newer GCC,
   the __SUPPORT_SNAN__ check may be skipped for those versions.  */

/* Return number of classification appropriate for X.  */
# if __GNUC_PREREQ (4,4) && !defined __SUPPORT_SNAN__			      \
     && !defined __OPTIMIZE_SIZE__
#  define fpclassify(x) __builtin_fpclassify (FP_NAN, FP_INFINITE,	      \
     FP_NORMAL, FP_SUBNORMAL, FP_ZERO, x)
# else
#  define fpclassify(x) __MATH_TG ((x), __fpclassify, (x))
# endif

/* Return nonzero value if sign of X is negative.  */
# if __GNUC_PREREQ (4,0)
#  define signbit(x) __MATH_TG ((x), __builtin_signbit, (x))
# else
#  define signbit(x) __MATH_TG ((x), __signbit, (x))
# endif

/* Return nonzero value if X is not +-Inf or NaN.  */
# if __GNUC_PREREQ (4,4) && !defined __SUPPORT_SNAN__
#  define isfinite(x) __builtin_isfinite (x)
# else
#  define isfinite(x) __MATH_TG ((x), __finite, (x))
# endif

/* Return nonzero value if X is neither zero, subnormal, Inf, nor NaN.  */
# if __GNUC_PREREQ (4,4) && !defined __SUPPORT_SNAN__
#  define isnormal(x) __builtin_isnormal (x)
# else
#  define isnormal(x) (fpclassify (x) == FP_NORMAL)
# endif

/* Return nonzero value if X is a NaN.  We could use `fpclassify' but
   we already have this functions `__isnan' and it is faster.  */
# if __GNUC_PREREQ (4,4) && !defined __SUPPORT_SNAN__
#  define isnan(x) __builtin_isnan (x)
# else
#  define isnan(x) __MATH_TG ((x), __isnan, (x))
# endif

/* Return nonzero value if X is positive or negative infinity.  */
# if __GNUC_PREREQ (4,4) && !defined __SUPPORT_SNAN__
#  define isinf(x) __builtin_isinf_sign (x)
# else
#  define isinf(x) __MATH_TG ((x), __isinf, (x))
# endif

/* Bitmasks for the math_errhandling macro.  */
# define MATH_ERRNO	1	/* errno set by math functions.  */
# define MATH_ERREXCEPT	2	/* Exceptions raised by math functions.  */

/* By default all functions support both errno and exception handling.
   In gcc's fast math mode and if inline functions are defined this
   might not be true.  */
# ifndef __FAST_MATH__
#  define math_errhandling	(MATH_ERRNO | MATH_ERREXCEPT)
# endif

#endif /* Use ISO C99.  */

#if __GLIBC_USE (IEC_60559_BFP_EXT)
# include <bits/iscanonical.h>

/* Return nonzero value if X is a signaling NaN.  */
# define issignaling(x) __MATH_TG ((x), __issignaling, (x))

/* Return nonzero value if X is subnormal.  */
# define issubnormal(x) (fpclassify (x) == FP_SUBNORMAL)

/* Return nonzero value if X is zero.  */
# ifndef __cplusplus
#  ifdef __SUPPORT_SNAN__
#   define iszero(x) (fpclassify (x) == FP_ZERO)
#  else
#   define iszero(x) (((__typeof (x)) (x)) == 0)
#  endif
# else	/* __cplusplus */
extern "C++" {
template <class __T> inline bool
iszero (__T __val)
{
#  ifdef __SUPPORT_SNAN__
  return fpclassify (__val) == FP_ZERO;
#  else
  return __val == 0;
#  endif
}
} /* extern C++ */
# endif	/* __cplusplus */
#endif /* Use IEC_60559_BFP_EXT.  */

#ifdef	__USE_MISC
/* Support for various different standard error handling behaviors.  */
typedef enum
{
  _IEEE_ = -1,	/* According to IEEE 754/IEEE 854.  */
  _SVID_,	/* According to System V, release 4.  */
  _XOPEN_,	/* Nowadays also Unix98.  */
  _POSIX_,
  _ISOC_	/* Actually this is ISO C99.  */
} _LIB_VERSION_TYPE;

/* This variable can be changed at run-time to any of the values above to
   affect floating point error handling behavior (it may also be necessary
   to change the hardware FPU exception settings).  */
extern _LIB_VERSION_TYPE _LIB_VERSION;
#endif


#ifdef __USE_MISC
/* In SVID error handling, `matherr' is called with this description
   of the exceptional condition.

   We have a problem when using C++ since `exception' is a reserved
   name in C++.  */
# ifdef __cplusplus
struct __exception
# else
struct exception
# endif
  {
    int type;
    char *name;
    double arg1;
    double arg2;
    double retval;
  };

# ifdef __cplusplus
extern int matherr (struct __exception *__exc) throw ();
# else
extern int matherr (struct exception *__exc);
# endif

# define X_TLOSS	1.41484755040568800000e+16

/* Types of exceptions in the `type' field.  */
# define DOMAIN		1
# define SING		2
# define OVERFLOW	3
# define UNDERFLOW	4
# define TLOSS		5
# define PLOSS		6

/* SVID mode specifies returning this large value instead of infinity.  */
# define HUGE		3.40282347e+38F

#else	/* !Misc.  */

# ifdef __USE_XOPEN
/* X/Open wants another strange constant.  */
#  define MAXFLOAT	3.40282347e+38F
# endif

#endif	/* Misc.  */


/* Some useful constants.  */
#if defined __USE_MISC || defined __USE_XOPEN
# define M_E		2.7182818284590452354	/* e */
# define M_LOG2E	1.4426950408889634074	/* log_2 e */
# define M_LOG10E	0.43429448190325182765	/* log_10 e */
# define M_LN2		0.69314718055994530942	/* log_e 2 */
# define M_LN10		2.30258509299404568402	/* log_e 10 */
# define M_PI		3.14159265358979323846	/* pi */
# define M_PI_2		1.57079632679489661923	/* pi/2 */
# define M_PI_4		0.78539816339744830962	/* pi/4 */
# define M_1_PI		0.31830988618379067154	/* 1/pi */
# define M_2_PI		0.63661977236758134308	/* 2/pi */
# define M_2_SQRTPI	1.12837916709551257390	/* 2/sqrt(pi) */
# define M_SQRT2	1.41421356237309504880	/* sqrt(2) */
# define M_SQRT1_2	0.70710678118654752440	/* 1/sqrt(2) */
#endif

/* The above constants are not adequate for computation using `long double's.
   Therefore we provide as an extension constants with similar names as a
   GNU extension.  Provide enough digits for the 128-bit IEEE quad.  */
#ifdef __USE_GNU
# define M_El		2.718281828459045235360287471352662498L /* e */
# define M_LOG2El	1.442695040888963407359924681001892137L /* log_2 e */
# define M_LOG10El	0.434294481903251827651128918916605082L /* log_10 e */
# define M_LN2l		0.693147180559945309417232121458176568L /* log_e 2 */
# define M_LN10l	2.302585092994045684017991454684364208L /* log_e 10 */
# define M_PIl		3.141592653589793238462643383279502884L /* pi */
# define M_PI_2l	1.570796326794896619231321691639751442L /* pi/2 */
# define M_PI_4l	0.785398163397448309615660845819875721L /* pi/4 */
# define M_1_PIl	0.318309886183790671537767526745028724L /* 1/pi */
# define M_2_PIl	0.636619772367581343075535053490057448L /* 2/pi */
# define M_2_SQRTPIl	1.128379167095512573896158903121545172L /* 2/sqrt(pi) */
# define M_SQRT2l	1.414213562373095048801688724209698079L /* sqrt(2) */
# define M_SQRT1_2l	0.707106781186547524400844362104849039L /* 1/sqrt(2) */
#endif


/* When compiling in strict ISO C compatible mode we must not use the
   inline functions since they, among other things, do not set the
   `errno' variable correctly.  */
#if defined __STRICT_ANSI__ && !defined __NO_MATH_INLINES
# define __NO_MATH_INLINES	1
#endif

#if defined __USE_ISOC99 && __GNUC_PREREQ(2,97)
/* ISO C99 defines some macros to compare number while taking care for
   unordered numbers.  Many FPUs provide special instructions to support
   these operations.  Generic support in GCC for these as builtins went
   in before 3.0.0, but not all cpus added their patterns.  We define
   versions that use the builtins here, and <bits/mathinline.h> will
   undef/redefine as appropriate for the specific GCC version in use.  */
# define isgreater(x, y)	__builtin_isgreater(x, y)
# define isgreaterequal(x, y)	__builtin_isgreaterequal(x, y)
# define isless(x, y)		__builtin_isless(x, y)
# define islessequal(x, y)	__builtin_islessequal(x, y)
# define islessgreater(x, y)	__builtin_islessgreater(x, y)
# define isunordered(u, v)	__builtin_isunordered(u, v)
#endif

/* Get machine-dependent inline versions (if there are any).  */
#ifdef __USE_EXTERN_INLINES
# include <bits/mathinline.h>
#endif

/* Define special entry points to use when the compiler got told to
   only expect finite results.  */
#if defined __FINITE_MATH_ONLY__ && __FINITE_MATH_ONLY__ > 0
# include <bits/math-finite.h>
#endif

#ifdef __USE_ISOC99
/* If we've still got undefined comparison macros, provide defaults.  */

/* Return nonzero value if X is greater than Y.  */
# ifndef isgreater
#  define isgreater(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      !isunordered (__x, __y) && __x > __y; }))
# endif

/* Return nonzero value if X is greater than or equal to Y.  */
# ifndef isgreaterequal
#  define isgreaterequal(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      !isunordered (__x, __y) && __x >= __y; }))
# endif

/* Return nonzero value if X is less than Y.  */
# ifndef isless
#  define isless(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      !isunordered (__x, __y) && __x < __y; }))
# endif

/* Return nonzero value if X is less than or equal to Y.  */
# ifndef islessequal
#  define islessequal(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      !isunordered (__x, __y) && __x <= __y; }))
# endif

/* Return nonzero value if either X is less than Y or Y is less than X.  */
# ifndef islessgreater
#  define islessgreater(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      !isunordered (__x, __y) && (__x < __y || __y < __x); }))
# endif

/* Return nonzero value if arguments are unordered.  */
# ifndef isunordered
#  define isunordered(u, v) \
  (__extension__							      \
   ({ __typeof__(u) __u = (u); __typeof__(v) __v = (v);			      \
      fpclassify (__u) == FP_NAN || fpclassify (__v) == FP_NAN; }))
# endif

#endif

#if __GLIBC_USE (IEC_60559_BFP_EXT)
/* An expression whose type has the widest of the evaluation formats
   of X and Y (which are of floating-point types).  */
# if __FLT_EVAL_METHOD__ == 2 || __FLT_EVAL_METHOD__ > 64
#  define __MATH_EVAL_FMT2(x, y) ((x) + (y) + 0.0L)
# elif __FLT_EVAL_METHOD__ == 1 || __FLT_EVAL_METHOD__ > 32
#  define __MATH_EVAL_FMT2(x, y) ((x) + (y) + 0.0)
# else
#  define __MATH_EVAL_FMT2(x, y) ((x) + (y))
# endif

/* Return X == Y but raising "invalid" and setting errno if X or Y is
   a NaN.  */
# define iseqsig(x, y) \
  __MATH_TG (__MATH_EVAL_FMT2 (x, y), __iseqsig, ((x), (y)))
#endif

__END_DECLS


#endif /* math.h  */
