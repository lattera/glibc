/* Copyright (C) 1997 Free Software Foundation, Inc.
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
 *	ISO C 9X: 7.4 Integral types	<inttypes.h>
 */

#ifndef _INTTYPES_H

#define _INTTYPES_H	1
#include <features.h>

/* Exact integral types.  */

/* Signed.  */
typedef signed char int8_t;
typedef short int  int16_t;
typedef int        int32_t;
typedef long int   int64_t;

/* Unsigned.  */
typedef unsigned char       uint8_t;
typedef unsigned short int uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long int  uint64_t;


/* Largest integral types.  */
typedef long long int      intmax_t;
typedef unsigned long int uintmax_t;


/* Types for `void *' pointers.  */
typedef long int           intptr_t;
typedef unsigned long int uintptr_t;


/* Efficient types.  */
typedef long int           intfast_t;
typedef unsigned long int uintfast_t;


/* Small types.  */

/* Signed.  */
typedef signed char int_least8_t;
typedef short int  int_least16_t;
typedef int        int_least32_t;
typedef long int   int_least64_t;

/* Unsigned.  */
typedef unsigned char       uint_least8_t;
typedef unsigned short int uint_least16_t;
typedef unsigned int       uint_least32_t;
typedef unsigned long int  uint_least64_t;


/* Fast types.  */

/* Signed.  */
typedef signed char int_fast8_t;
typedef int        int_fast16_t;
typedef int        int_fast32_t;
typedef long int   int_fast64_t;

/* Unsigned.  */
typedef unsigned char      uint_fast8_t;
typedef unsigned int      uint_fast16_t;
typedef unsigned int      uint_fast32_t;
typedef unsigned long int uint_fast64_t;


/* Limits of integral types.  */

/* Minimum of signed integral types.  */
#define INT8_MIN		(-128)
#define INT16_MIN		(-32767-1)
#define INT32_MIN		(-2147483647-1)
#define INT64_MIN		(-9223372036854775807L-1)
/* Maximum of signed integral types.  */
#define INT8_MAX		(127)
#define INT16_MAX		(32767)
#define INT32_MAX		(2147483647)
#define INT64_MAX		(9223372036854775807L)

/* Maximum of unsigned integral types.  */
#define UINT8_MAX		(255U)
#define UINT16_MAX		(65535U)
#define UINT32_MAX		(4294967295U)
#define UINT64_MAX		(18446744073709551615uL)


/* Minimum of signed integral types having a minimum size.  */
#define INT_LEAST8_MIN		(-128)
#define INT_LEAST16_MIN		(-32767-1)
#define INT_LEAST32_MIN		(-2147483647-1)
#define INT_LEAST64_MIN		(-9223372036854775807L-1)
/* Maximum of signed integral types having a minimum size.  */
#define INT_LEAST8_MAX		(127)
#define INT_LEAST16_MAX		(32767)
#define INT_LEAST32_MAX		(2147483647)
#define INT_LEAST64_MAX		(9223372036854775807L)

/* Maximum of unsigned integral types having a minimum size.  */
#define UINT_LEAST8_MAX		(255U)
#define UINT_LEAST16_MAX	(65535U)
#define UINT_LEAST32_MAX	(4294967295U)
#define UINT_LEAST64_MAX	(18446744073709551615uL)


/* Minimum of fast signed integral types having a minimum size.  */
#define INT_FAST8_MIN		(-128)
#define INT_FAST16_MIN		(-2147483647-1)
#define INT_FAST32_MIN		(-2147483647-1)
#define INT_FAST64_MIN		(-9223372036854775807L-1)
/* Maximum of fast signed integral types having a minimum size.  */
#define INT_FAST8_MAX		(127)
#define INT_FAST16_MAX		(2147483647)
#define INT_FAST32_MAX		(2147483647)
#define INT_FAST64_MAX		(9223372036854775807L)

/* Maximum of fast unsigned integral types having a minimum size.  */
#define UINT_FAST8_MAX		(255U)
#define UINT_FAST16_MAX		(4294967295U)
#define UINT_FAST32_MAX		(4294967295U)
#define UINT_FAST64_MAX		(18446744073709551615uL)


/* Minimum for most efficient signed integral types.  */
#define INTFAST_MIN		(-128)
/* Maximum for most efficient signed integral types.  */
#define INTFAST_MAX		(127)

/* Maximum for most efficient unsigned integral types.  */
#define UINTFAST_MAX		(255)


/* Minimum for largest signed integral type.  */
#define INTMAX_MIN		(-9223372036854775807L-1)
/* Maximum for largest signed integral type.  */
#define INTMAX_MAX		(9223372036854775807L)

/* Maximum for largest unsigned integral type.  */
#define UINTMAX_MAX		(18446744073709551615uL)


/* Values to test for integral types holding `void *' pointer.  */
#define INTPTR_MAX		(9223372036854775807L)
#define UINTPTR_MAX		(18446744073709551615uL)


/* Signed.  */
#define INT8_C(c)	((int8_t) c)
#define INT16_C(c)	((int16_t) c)
#define INT32_C(c)	((int32_t) c)
#define INT64_C(c)	((int64_t) __CONCAT (c,l))

/* Unsigned.  */
#define UINT8_C(c)	((uint8_t) __CONCAT (c,u))
#define UINT16_C(c)	((uint16_t) __CONCAT (c,u))
#define UINT32_C(c)	((uint32_t) __CONCAT (c,u))
#define UINT64_C(c)	((uint64_t) __CONCAT (c,ul))

/* Maximal type.  */
#define INTMAX_C(c)	((intmax_t) __CONCAT (c,l))
#define UINTMAX_C(c)	((uintmax_t) __CONCAT (c,ul))


/* Macros for printing format specifiers.  */

/* Decimal notation.  */
#define PRId8		"d"
#define PRId16		"d"
#define PRId32		"d"
#define PRId64		"ld"

#define PRIdLEAST8	"d"
#define PRIdLEAST16	"d"
#define PRIdLEAST32	"d"
#define PRIdLEAST64	"ld"

#define PRIdFAST8	"d"
#define PRIdFAST16	"d"
#define PRIdFAST32	"d"
#define PRIdFAST64	"ld"


#define PRIi8		"i"
#define PRIi16		"i"
#define PRIi32		"i"
#define PRIi64		"li"

#define PRIiLEAST8	"i"
#define PRIiLEAST16	"i"
#define PRIiLEAST32	"i"
#define PRIiLEAST64	"li"

#define PRIiFAST8	"i"
#define PRIiFAST16	"i"
#define PRIiFAST32	"i"
#define PRIiFAST64	"li"

/* Octal notation.  */
#define PRIo8		"o"
#define PRIo16		"o"
#define PRIo32		"o"
#define PRIo64		"lo"

#define PRIoLEAST8	"o"
#define PRIoLEAST16	"o"
#define PRIoLEAST32	"o"
#define PRIoLEAST64	"lo"

#define PRIoFAST8	"o"
#define PRIoFAST16	"o"
#define PRIoFAST32	"o"
#define PRIoFAST64	"lo"

/* lowercase hexadecimal notation.  */
#define PRIx8		"x"
#define PRIx16		"x"
#define PRIx32		"x"
#define PRIx64		"lx"

#define PRIxLEAST8	"x"
#define PRIxLEAST16	"x"
#define PRIxLEAST32	"x"
#define PRIxLEAST64	"lx"

#define PRIxFAST8	"x"
#define PRIxFAST16	"x"
#define PRIxFAST32	"x"
#define PRIxFAST64	"lx"

/* UPPERCASE hexadecimal notation.  */
#define PRIX8		"X"
#define PRIX16		"X"
#define PRIX32		"X"
#define PRIX64		"lX"

#define PRIXLEAST8	"X"
#define PRIXLEAST16	"X"
#define PRIXLEAST32	"X"
#define PRIXLEAST64	"lX"

#define PRIXFAST8	"X"
#define PRIXFAST16	"X"
#define PRIXFAST32	"X"
#define PRIXFAST64	"lX"


/* Unsigned integers.  */
#define PRIu8		"u"
#define PRIu16		"u"
#define PRIu32		"u"
#define PRIu64		"lu"

#define PRIuLEAST8	"u"
#define PRIuLEAST16	"u"
#define PRIuLEAST32	"u"
#define PRIuLEAST64	"lu"

#define PRIuFAST8	"u"
#define PRIuFAST16	"u"
#define PRIuFAST32	"u"
#define PRIuFAST64	"lu"


/* Macros for printing `intmax_t' and `uintmax_t'.  */
#define PRIdMAX		"ld"
#define PRIoMAX		"lo"
#define PRIxMAX		"lx"
#define PRIuMAX		"lu"


/* Macros for printing `intfast_t' and `uintfast_t'.  */
#define PRIdFAST	"ld"
#define PRIoFAST	"lo"
#define PRIxFAST	"lx"
#define PRIuFAST	"lu"


/* Macros for printing `intptr_t' and `uintptr_t'.  */
#define PRIdPTR		"ld"
#define PRIoPTR		"lo"
#define PRIxPTR		"lx"
#define PRIuPTR		"lu"


/* Macros for printing format specifiers.  */

/* Decimal notation.  */
#define SCNd16		"hd"
#define SCNd32		"d"
#define SCNd64		"ld"

#define SCNi16		"hi"
#define SCNi32		"i"
#define SCNi64		"li"

/* Octal notation.  */
#define SCNo16		"ho"
#define SCNo32		"o"
#define SCNo64		"lo"

/* Hexadecimal notation.  */
#define SCNx16		"hx"
#define SCNx32		"x"
#define SCNx64		"lx"


/* Macros for scaning `intfast_t' and `uintfast_t'.  */
#define SCNdFAST	"ld"
#define SCNiFAST	"li"
#define SCNoFAST	"lo"
#define SCNxFAST	"lx"

/* Macros for scaning `intptr_t' and `uintptr_t'.  */
#define SCNdPTR		"ld"
#define SCNiPTR		"li"
#define SCNoPTR		"lo"
#define SCNxPTR		"lx"

#endif /* inttypes.h */
