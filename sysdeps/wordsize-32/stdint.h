/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/*
 *	ISO C 9X: 7.4 Integer types	<stdint.h>
 */

#ifndef _STDINT_H
#define _STDINT_H	1

#include <features.h>
#define __need_wchar_t
#include <stddef.h>

/* Exact integral types.  */

/* Signed.  */

/* There is some amount of overlap with <sys/types.h> as known by inet code */
#ifndef __int8_t_defined
# define __int8_t_defined
typedef signed char    int8_t;
typedef short int     int16_t;
typedef int           int32_t;
__extension__
typedef long long int int64_t;
#endif

/* Unsigned.  */
typedef unsigned char           uint8_t;
typedef unsigned short int     uint16_t;
typedef unsigned int           uint32_t;
__extension__
typedef unsigned long long int uint64_t;


/* Small types.  */

/* Signed.  */
typedef signed char    int_least8_t;
typedef short int     int_least16_t;
typedef int           int_least32_t;
__extension__
typedef long long int int_least64_t;

/* Unsigned.  */
typedef unsigned char           uint_least8_t;
typedef unsigned short int     uint_least16_t;
typedef unsigned int           uint_least32_t;
__extension__
typedef unsigned long long int uint_least64_t;


/* Fast types.  */

/* Signed.  */
typedef signed char    int_fast8_t;
typedef int           int_fast16_t;
typedef int           int_fast32_t;
__extension__
typedef long long int int_fast64_t;

/* Unsigned.  */
typedef unsigned char           uint_fast8_t;
typedef unsigned int           uint_fast16_t;
typedef unsigned int           uint_fast32_t;
__extension__
typedef unsigned long long int uint_fast64_t;


/* Types for `void *' pointers.  */
#ifndef intptr_t
typedef int           intptr_t;
# define intptr_t intptr_t
#endif
typedef unsigned int uintptr_t;


/* Largest integral types.  */
__extension__ typedef long long int           intmax_t;
__extension__ typedef unsigned long long int uintmax_t;


/* The ISO C 9X standard specifies that these macros must only be
   defined if explicitly requested.  */
#if !defined __cplusplus || defined __STDC_LIMIT_MACROS

/* Limits of integral types.  */

/* Minimum of signed integral types.  */
# define INT8_MIN		(-128)
# define INT16_MIN		(-32767-1)
# define INT32_MIN		(-2147483647-1)
# define INT64_MIN		(-9223372036854775807LL-1)
/* Maximum of signed integral types.  */
# define INT8_MAX		(127)
# define INT16_MAX		(32767)
# define INT32_MAX		(2147483647)
# define INT64_MAX		(9223372036854775807LL)

/* Maximum of unsigned integral types.  */
# define UINT8_MAX		(255U)
# define UINT16_MAX		(65535U)
# define UINT32_MAX		(4294967295U)
# define UINT64_MAX		(18446744073709551615uLL)


/* Minimum of signed integral types having a minimum size.  */
# define INT_LEAST8_MIN		(-128)
# define INT_LEAST16_MIN	(-32767-1)
# define INT_LEAST32_MIN	(-2147483647-1)
# define INT_LEAST64_MIN	(-9223372036854775807LL-1)
/* Maximum of signed integral types having a minimum size.  */
# define INT_LEAST8_MAX		(127)
# define INT_LEAST16_MAX	(32767)
# define INT_LEAST32_MAX	(2147483647)
# define INT_LEAST64_MAX	(9223372036854775807LL)

/* Maximum of unsigned integral types having a minimum size.  */
# define UINT_LEAST8_MAX	(255U)
# define UINT_LEAST16_MAX	(65535U)
# define UINT_LEAST32_MAX	(4294967295U)
# define UINT_LEAST64_MAX	(18446744073709551615uLL)


/* Minimum of fast signed integral types having a minimum size.  */
# define INT_FAST8_MIN		(-128)
# define INT_FAST16_MIN		(-2147483647-1)
# define INT_FAST32_MIN		(-2147483647-1)
# define INT_FAST64_MIN		(-9223372036854775807LL-1)
/* Maximum of fast signed integral types having a minimum size.  */
# define INT_FAST8_MAX		(127)
# define INT_FAST16_MAX		(2147483647)
# define INT_FAST32_MAX		(2147483647)
# define INT_FAST64_MAX		(9223372036854775807LL)

/* Maximum of fast unsigned integral types having a minimum size.  */
# define UINT_FAST8_MAX		(255U)
# define UINT_FAST16_MAX	(4294967295U)
# define UINT_FAST32_MAX	(4294967295U)
# define UINT_FAST64_MAX	(18446744073709551615uLL)


/* Values to test for integral types holding `void *' pointer.  */
# define INTPTR_MIN		(-2147483647-1)
# define INTPTR_MAX		(2147483647)
# define UINTPTR_MAX		(4294967295U)


/* Minimum for largest signed integral type.  */
# define INTMAX_MIN		(-9223372036854775807LL-1)
/* Maximum for largest signed integral type.  */
# define INTMAX_MAX		(9223372036854775807LL)

/* Maximum for largest unsigned integral type.  */
# define UINTMAX_MAX		(18446744073709551615uLL)


/* Limits of other integer types.  */

/* Limits of `ptrdiff_t' type.  */
# define PTRDIFF_MIN	(-2147483647-1)
# define PTRDIFF_MAX	(2147483647)

/* Limits of `sig_atomic_t'.  */
# define SIG_ATOMIC_MIN	(-2147483647-1)
# define SIG_ATOMIC_MAX	(2147483647)

/* Limit of `size_t' type.  */
# define SIZE_MAX	(4294967295U)

/* Limits of `wchar_t'.  */
# ifndef WCHAR_MIN
/* These constants might also be defined in <wchar.h>.  */
#  define WCHAR_MIN	(-2147483647-1)
#  define WCHAR_MAX	(2147483647)
# endif

/* Limits of `wint_t'.  */
# define WINT_MIN	(0)
# define WINT_MAX	(4294967295U)

#endif	/* C++ && limit macros */


/* The ISO C 9X standard specifies that these macros must only be
   defined if explicitly requested.  */
#if !defined __cplusplus || defined __STDC_CONSTANT_MACROS

/* Signed.  */
# define INT8_C(c)	((int8_t) c)
# define INT16_C(c)	((int16_t) c)
# define INT32_C(c)	((int32_t) c)
# define INT64_C(c)	((int64_t) __CONCAT (c,ll))

/* Unsigned.  */
# define UINT8_C(c)	((uint8_t) __CONCAT (c,u))
# define UINT16_C(c)	((uint16_t) __CONCAT (c,u))
# define UINT32_C(c)	((uint32_t) __CONCAT (c,u))
# define UINT64_C(c)	((uint64_t) __CONCAT (c,ull))

/* Maximal type.  */
# define INTMAX_C(c)	((intmax_t) __CONCAT (c,ll))
# define UINTMAX_C(c)	((uintmax_t) __CONCAT (c,ull))

#endif	/* C++ && constant macros */

#endif /* stdint.h */
