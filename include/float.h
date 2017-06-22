#ifndef _LIBC_FLOAT_H
#define _LIBC_FLOAT_H

#define __GLIBC_INTERNAL_STARTING_HEADER_IMPLEMENTATION
#include <bits/libc-header-start.h>

#ifndef _ISOMAC
# define __STDC_WANT_IEC_60559_TYPES_EXT__
#endif

#include_next <float.h>

/* Supplement float.h macros for _Float128 for older compilers
   which do not yet support the type.  These are described in
   TS 18661-3.  */
#include <features.h>
#include <bits/floatn.h>
#if !__GNUC_PREREQ (7, 0) \
    && __HAVE_FLOAT128 && __GLIBC_USE (IEC_60559_TYPES_EXT)
# define FLT128_MANT_DIG	113
# define FLT128_DECIMAL_DIG	36
# define FLT128_DIG		33
# define FLT128_MIN_EXP		(-16381)
# define FLT128_MIN_10_EXP	(-4931)
# define FLT128_MAX_EXP		16384
# define FLT128_MAX_10_EXP	4932
# define FLT128_MAX		1.18973149535723176508575932662800702e+4932Q
# define FLT128_EPSILON		1.92592994438723585305597794258492732e-34Q
# define FLT128_MIN		3.36210314311209350626267781732175260e-4932Q
# define FLT128_TRUE_MIN	6.47517511943802511092443895822764655e-4966Q
#endif

#endif /* _LIBC_FLOAT_H */
