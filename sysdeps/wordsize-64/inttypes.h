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
 *	ISO C 9X: 7.4 Integral types	<inttypes.h>
 */

#ifndef _INTTYPES_H
#define _INTTYPES_H	1

#include <features.h>
/* Get the type definitions.  */
#include <stdint.h>


/* The ISO C 9X standard specifies that these macros must only be
   defined if explicitly requested.  */
#if !defined __cplusplus || defined __STDC_FORMAT_MACROS

/* Macros for printing format specifiers.  */

/* Decimal notation.  */
# define PRId8		"d"
# define PRId16		"d"
# define PRId32		"d"
# define PRId64		"ld"

# define PRIdLEAST8	"d"
# define PRIdLEAST16	"d"
# define PRIdLEAST32	"d"
# define PRIdLEAST64	"ld"

# define PRIdFAST8	"d"
# define PRIdFAST16	"d"
# define PRIdFAST32	"d"
# define PRIdFAST64	"ld"


# define PRIi8		"i"
# define PRIi16		"i"
# define PRIi32		"i"
# define PRIi64		"li"

# define PRIiLEAST8	"i"
# define PRIiLEAST16	"i"
# define PRIiLEAST32	"i"
# define PRIiLEAST64	"li"

# define PRIiFAST8	"i"
# define PRIiFAST16	"i"
# define PRIiFAST32	"i"
# define PRIiFAST64	"li"

/* Octal notation.  */
# define PRIo8		"o"
# define PRIo16		"o"
# define PRIo32		"o"
# define PRIo64		"lo"

# define PRIoLEAST8	"o"
# define PRIoLEAST16	"o"
# define PRIoLEAST32	"o"
# define PRIoLEAST64	"lo"

# define PRIoFAST8	"o"
# define PRIoFAST16	"o"
# define PRIoFAST32	"o"
# define PRIoFAST64	"lo"

/* lowercase hexadecimal notation.  */
# define PRIx8		"x"
# define PRIx16		"x"
# define PRIx32		"x"
# define PRIx64		"lx"

# define PRIxLEAST8	"x"
# define PRIxLEAST16	"x"
# define PRIxLEAST32	"x"
# define PRIxLEAST64	"lx"

# define PRIxFAST8	"x"
# define PRIxFAST16	"x"
# define PRIxFAST32	"x"
# define PRIxFAST64	"lx"

/* UPPERCASE hexadecimal notation.  */
# define PRIX8		"X"
# define PRIX16		"X"
# define PRIX32		"X"
# define PRIX64		"lX"

# define PRIXLEAST8	"X"
# define PRIXLEAST16	"X"
# define PRIXLEAST32	"X"
# define PRIXLEAST64	"lX"

# define PRIXFAST8	"X"
# define PRIXFAST16	"X"
# define PRIXFAST32	"X"
# define PRIXFAST64	"lX"


/* Unsigned integers.  */
# define PRIu8		"u"
# define PRIu16		"u"
# define PRIu32		"u"
# define PRIu64		"lu"

# define PRIuLEAST8	"u"
# define PRIuLEAST16	"u"
# define PRIuLEAST32	"u"
# define PRIuLEAST64	"lu"

# define PRIuFAST8	"u"
# define PRIuFAST16	"u"
# define PRIuFAST32	"u"
# define PRIuFAST64	"lu"


/* Macros for printing `intmax_t' and `uintmax_t'.  */
# define PRIdMAX	"ld"
# define PRIoMAX	"lo"
# define PRIxMAX	"lx"
# define PRIuMAX	"lu"


/* Macros for printing `intptr_t' and `uintptr_t'.  */
# define PRIdPTR	"ld"
# define PRIoPTR	"lo"
# define PRIxPTR	"lx"
# define PRIuPTR	"lu"


/* Macros for printing format specifiers.  */

/* Decimal notation.  */
# define SCNd8		"hhd"
# define SCNd16		"hd"
# define SCNd32		"d"
# define SCNd64		"ld"

# define SCNi8		"hhi"
# define SCNi16		"hi"
# define SCNi32		"i"
# define SCNi64		"li"

# define SCNu8		"hhu"
# define SCNu16		"hu"
# define SCNu32		"u"
# define SCNu64		"lu"

/* Octal notation.  */
# define SCNo8		"hho"
# define SCNo16		"ho"
# define SCNo32		"o"
# define SCNo64		"lo"

/* Hexadecimal notation.  */
# define SCNx8		"hhx"
# define SCNx16		"hx"
# define SCNx32		"x"
# define SCNx64		"lx"


/* Macros for scaning `intfast_t' and `uintfast_t'.  */
# define SCNdFAST	"ld"
# define SCNiFAST	"li"
# define SCNoFAST	"lo"
# define SCNxFAST	"lx"

/* Macros for scaning `intptr_t' and `uintptr_t'.  */
# define SCNdPTR	"ld"
# define SCNiPTR	"li"
# define SCNoPTR	"lo"
# define SCNxPTR	"lx"

#endif	/* C++ && format macros */


__BEGIN_DECLS

/* Like `strtol' but convert to `intmax_t'.  */
extern intmax_t strtoimax __P ((__const char *__restrict __nptr,
				char **__restrict __endptr, int __base));

/* Like `strtoul' but convert to `uintmax_t'.  */
extern uintmax_t strtoumax __P ((__const char * __restrict __nptr,
				 char ** __restrict __endptr, int __base));

/* Like `wcstol' but convert to `intmax_t'.  */
extern intmax_t wcstoimax __P ((__const wchar_t * __restrict __nptr,
				wchar_t **__restrict __endptr, int __base));

/* Like `wcstoul' but convert to `uintmax_t'.  */
extern uintmax_t wcstoumax __P ((__const wchar_t * __restrict __nptr,
				 wchar_t ** __restrict __endptr, int __base));

#ifdef __USE_EXTERN_INLINES

/* Like `strtol' but convert to `intmax_t'.  */
# ifndef __strtol_internal_defined
extern long int __strtol_internal __P ((__const char *__restrict __nptr,
					char **__restrict __endptr,
					int __base, int __group));
#  define __strtol_internal_defined	1
# endif
extern __inline intmax_t
strtoimax (__const char *__restrict nptr, char **__restrict endptr, int base)
{
  return __strtol_internal (nptr, endptr, base, 0);
}

/* Like `strtoul' but convert to `uintmax_t'.  */
# ifndef __strtoul_internal_defined
extern unsigned long int __strtoul_internal __P ((__const char *
						  __restrict __nptr,
						  char ** __restrict __endptr,
						  int __base, int __group));
#  define __strtoul_internal_defined	1
# endif
extern __inline uintmax_t
strtoumax (__const char *__restrict nptr, char **__restrict endptr, int base)
{
  return __strtoul_internal (nptr, endptr, base, 0);
}

/* Like `wcstol' but convert to `intmax_t'.  */
# ifndef __wcstol_internal_defined
extern long int __wcstol_internal __P ((__const wchar_t * __restrict __nptr,
					wchar_t **__restrict __endptr,
					int __base, int __group));
#  define __wcstol_internal_defined	1
# endif
extern __inline intmax_t
wcstoimax (__const wchar_t *__restrict nptr, wchar_t **__restrict endptr,
	   int base)
{
  return __wcstol_internal (nptr, endptr, base, 0);
}


/* Like `wcstoul' but convert to `uintmax_t'.  */
# ifndef __wcstoul_internal_defined
extern unsigned long int __wcstoul_internal __P ((__const wchar_t *
						  __restrict __nptr,
						  wchar_t **
						  __restrict __endptr,
						  int __base, int __group));
#  define __wcstoul_internal_defined	1
# endif
extern __inline uintmax_t
wcstoumax (__const wchar_t *__restrict nptr, wchar_t **__restrict endptr,
	   int base)
{
  return __wcstoul_internal (nptr, endptr, base, 0);
}
#endif	/* Use extern inlines.  */

__END_DECLS

#endif /* inttypes.h */
