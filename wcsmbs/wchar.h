/* Copyright (C) 1995, 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
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
 *      ISO C Standard, Amendment 1, 7.16.4
 *	General wide-string utilities	<wchar.h>
 */

#ifndef _WCHAR_H

#ifndef __need_mbstate_t
# define _WCHAR_H 1
# include <features.h>
#endif

#ifdef _WCHAR_H
/* Get FILE definition.  */
# define __need_FILE
# include <stdio.h>
/* Get va_list definition.  */
# define __need___va_list
# include <stdarg.h>

/* Get size_t, wchar_t, wint_t and NULL from <stddef.h>.  */
# define __need_size_t
# define __need_wchar_t
# define __need_NULL
#endif
#define __need_wint_t
#include <stddef.h>

/* We try to get wint_t from <stddef.h>, but not all GCC versions define it
   there.  So define it ourselves if it remains undefined.  */
#ifndef _WINT_T
/* Integral type unchanged by default argument promotions that can
   hold any value corresponding to members of the extended character
   set, as well as at least one value that does not correspond to any
   member of the extended character set.  */
# define _WINT_T
typedef unsigned int wint_t;
#endif


#ifndef __mbstate_t_defined
# define __mbstate_t_defined	1
/* Conversion state information.  */
typedef struct
{
  int count;		/* Number of bytes needed for the current character. */
  wint_t value;		/* Value so far.  */
} __mbstate_t;
#endif
#undef __need_mbstate_t


/* The rest of the file is only used if used if __need_mbstate_t is not
   defined.  */
#ifdef _WCHAR_H

/* Public type.  */
typedef __mbstate_t mbstate_t;

#ifndef WCHAR_MIN
/* These constants might also be defined in <inttypes.h>.  */
# define WCHAR_MIN (0)
# define WCHAR_MAX (0x7fffffff)
#endif

#ifndef WEOF
# define WEOF (0xffffffffu)
#endif

/* For XPG4 compliance we have to define the stuff from <wctype.h> here
   as well.  */
#if defined __USE_XOPEN && !defined __USE_UNIX98
# include <wctype.h>
#endif

/* This incomplete type is defined in <time.h> but needed here because
   of `wcsftime'.  */
struct tm;


__BEGIN_DECLS

/* Copy SRC to DEST.  */
extern wchar_t *wcscpy __P ((wchar_t *__restrict __dest,
			     __const wchar_t *__restrict __src));
/* Copy no more than N wide-characters of SRC to DEST.  */
extern wchar_t *wcsncpy __P ((wchar_t *__restrict __dest,
			      __const wchar_t *__restrict __src, size_t __n));

/* Append SRC onto DEST.  */
extern wchar_t *wcscat __P ((wchar_t *__restrict __dest,
			     __const wchar_t *__restrict __src));
/* Append no more than N wide-characters of SRC onto DEST.  */
extern wchar_t *wcsncat __P ((wchar_t *__restrict __dest,
			      __const wchar_t *__restrict __src, size_t __n));

/* Compare S1 and S2.  */
extern int wcscmp __P ((__const wchar_t *__s1, __const wchar_t *__s2));
/* Compare N wide-characters of S1 and S2.  */
extern int wcsncmp __P ((__const wchar_t *__s1, __const wchar_t *__s2,
			 size_t __n));

#ifdef __USE_GNU
/* Compare S1 and S2, ignoring case.  */
extern int wcscasecmp __P ((__const wchar_t *__s1, __const wchar_t *__s2));

/* Compare no more than N chars of S1 and S2, ignoring case.  */
extern int wcsncasecmp __P ((__const wchar_t *__s1, __const wchar_t *__s2,
                             size_t __n));

/* Similar to the two functions above but take the information from
   the provided locale and not the global locale.  */
# include <xlocale.h>

extern int __wcscasecmp_l __P ((__const wchar_t *__s1, __const wchar_t *__s2,
				__locale_t __loc));

extern int __wcsncasecmp_l __P ((__const wchar_t *__s1, __const wchar_t *__s2,
				 size_t __n, __locale_t __loc));
#endif

/* Compare S1 and S2, both interpreted as appropriate to the
   LC_COLLATE category of the current locale.  */
extern int wcscoll __P ((__const wchar_t *__s1, __const wchar_t *__s2));
/* Transform S2 into array pointed to by S1 such that if wcscmp is
   applied to two transformed strings the result is the as applying
   `wcscoll' to the original strings.  */
extern size_t wcsxfrm __P ((wchar_t *__restrict __s1,
			    __const wchar_t *__restrict __s2, size_t __n));

#ifdef __USE_GNU
/* Similar to the two functions above but take the information from
   the provided locale and not the global locale.  */

/* Compare S1 and S2, both interpreted as appropriate to the
   LC_COLLATE category of the given locale.  */
extern int __wcscoll_l __P ((__const wchar_t *__s1, __const wchar_t *__s2,
			     __locale_t __loc));
/* Transform S2 into array pointed to by S1 such that if wcscmp is
   applied to two transformed strings the result is the as applying
   `wcscoll' to the original strings.  */
extern size_t __wcsxfrm_l __P ((wchar_t *__s1, __const wchar_t *__s2,
				size_t __n, __locale_t __loc));

/* Duplicate S, returning an identical malloc'd string.  */
extern wchar_t *wcsdup __P ((__const wchar_t *__s));
#endif

/* Find the first occurrence of WC in WCS.  */
extern wchar_t *wcschr __P ((__const wchar_t *__wcs, wchar_t __wc));
/* Find the last occurrence of WC in WCS.  */
extern wchar_t *wcsrchr __P ((__const wchar_t *__wcs, wchar_t __wc));

#ifdef __USE_GNU
/* This funciton is similar to `wcschr'.  But it returns a pointer to
   the closing NUL wide character in case C is not found in S.  */
extern wchar_t *wcschrnul __P ((__const wchar_t *__s, wchar_t __wc));
#endif

/* Return the length of the initial segmet of WCS which
   consists entirely of wide characters not in REJECT.  */
extern size_t wcscspn __P ((__const wchar_t *__wcs,
			    __const wchar_t *__reject));
/* Return the length of the initial segmet of WCS which
   consists entirely of wide characters in  ACCEPT.  */
extern size_t wcsspn __P ((__const wchar_t *__wcs, __const wchar_t *__accept));
/* Find the first occurrence in WCS of any character in ACCEPT.  */
extern wchar_t *wcspbrk __P ((__const wchar_t *__wcs,
			      __const wchar_t *__accept));
/* Find the first occurrence of NEEDLE in HAYSTACK.  */
extern wchar_t *wcsstr __P ((__const wchar_t *__haystack,
			     __const wchar_t *__needle));

#if defined __USE_XOPEN && !defined __USE_UNIX98
/* Another name for `wcsstr' from XPG4.  */
extern wchar_t *wcswcs __P ((__const wchar_t *__haystack,
			     __const wchar_t *__needle));
#endif

/* Divide WCS into tokens separated by characters in DELIM.  */
extern wchar_t *wcstok __P ((wchar_t *__restrict __s,
			     __const wchar_t *__restrict __delim,
			     wchar_t **__restrict __ptr));

/* Return the number of wide characters in S.  */
extern size_t __wcslen __P ((__const wchar_t *__s));
extern size_t wcslen __P ((__const wchar_t *__s));

#ifdef __USE_GNU
/* Return the number of wide characters in S, but at most MAXLEN.  */
extern size_t wcsnlen __P ((__const wchar_t *__s, size_t __maxlen));
#endif


/* Search N wide characters of S for C.  */
extern wchar_t *wmemchr __P ((__const wchar_t *__s, wchar_t __c, size_t __n));

/* Compare N wide characters of S1 and S2.  */
extern int wmemcmp __P ((__const wchar_t *__restrict __s1,
			 __const wchar_t *__restrict __s2, size_t __n));

/* Copy N wide characters of SRC to DEST.  */
extern wchar_t *wmemcpy __P ((wchar_t *__restrict __s1,
			      __const wchar_t *__restrict __s2, size_t __n));

/* Copy N wide characters of SRC to DEST, guaranteeing
   correct behavior for overlapping strings.  */
extern wchar_t *wmemmove __P ((wchar_t *__s1, __const wchar_t *__s2,
			       size_t __n));

/* Set N wide characters of S to C.  */
extern wchar_t *wmemset __P ((wchar_t *__s, wchar_t __c, size_t __n));

#ifdef __USE_GNU
/* Copy N wide characters of SRC to DEST and return pointer to following
   wide character.  */
extern wchar_t *wmempcpy __P ((wchar_t *__restrict __s1,
			       __const wchar_t *__restrict __s2, size_t __n));
#endif


/* Determine whether C constitutes a valid (one-byte) multibyte
   character.  */
extern wint_t btowc __P ((int __c));

/* Determine whether C corresponds to a member of the extended
   character set whose multibyte representation is a single byte.  */
extern int wctob __P ((wint_t __c));

/* Determine whether PS points to an object representing the initial
   state.  */
extern int mbsinit __P ((__const mbstate_t *__ps));

/* Write wide character representation of multibyte character pointed
   to by S to PWC.  */
extern size_t mbrtowc __P ((wchar_t *__restrict __pwc,
			    __const char *__restrict __s, size_t __n,
			    mbstate_t *__p));

/* Write multibyte representation of wide character WC to S.  */
extern size_t wcrtomb __P ((char *__restrict __s, wchar_t __wc,
			    mbstate_t *__restrict __ps));

/* Return number of bytes in multibyte character pointed to by S.  */
extern size_t __mbrlen __P ((__const char *__restrict __s, size_t __n,
			     mbstate_t *__restrict __ps));
extern size_t mbrlen __P ((__const char *__restrict __s, size_t __n,
			   mbstate_t *__restrict __ps));

#if defined __OPTIMIZE__ && !defined __OPTIMIZE_SIZE__ \
    && defined __USE_EXTERN_INLINES
/* Define inline function as optimization.  */
extern __inline size_t mbrlen (__const char *__restrict __s, size_t __n,
			       mbstate_t *__restrict __ps) __THROW
{ return (__ps != NULL
	  ? mbrtowc (NULL, __s, __n, __ps) : __mbrlen (__s, __n, NULL)); }
#endif

/* Write wide character representation of multibyte character string
   SRC to DST.  */
extern size_t mbsrtowcs __P ((wchar_t *__restrict __dst,
			      __const char **__restrict __src,
			      size_t __len, mbstate_t *__restrict __ps));

/* Write multibyte character representation of wide character string
   SRC to DST.  */
extern size_t wcsrtombs __P ((char *__restrict __dst,
			      __const wchar_t **__restrict __src,
			      size_t __len, mbstate_t *__restrict __ps));


#ifdef	__USE_GNU
/* Write wide character representation of at most NMC bytes of the
   multibyte character string SRC to DST.  */
extern size_t mbsnrtowcs __P ((wchar_t *__restrict __dst,
			       __const char **__restrict __src, size_t __nmc,
			       size_t __len, mbstate_t *__restrict __ps));

/* Write multibyte character representation of at most NWC characters
   from the wide character string SRC to DST.  */
extern size_t wcsnrtombs __P ((char *__restrict __dst,
			       __const wchar_t **__restrict __src,
			       size_t __nwc, size_t __len,
			       mbstate_t *__restrict __ps));
#endif	/* use GNU */


/* The following functions are extensions found in X/Open CAE.  */
#ifdef __USE_XOPEN
/* Determine number of column positions required for C.  */
extern int wcwidth __P ((wint_t __c));

/* Determine number of column positions required for first N wide
   characters (or fewer if S ends before this) in S.  */
extern int wcswidth __P ((__const wchar_t *__s, size_t __n));
#endif	/* Use X/Open.  */


/* Convert initial portion of the wide string NPTR to `double'
   representation.  */
extern double wcstod __P ((__const wchar_t *__restrict __nptr,
			   wchar_t **__restrict __endptr));

#ifdef __USE_ISOC9X
/* Likewise for `float' and `long double' sizes of floating-point numbers.  */
extern float wcstof __P ((__const wchar_t *__restrict __nptr,
			  wchar_t **__restrict __endptr));
extern __long_double_t wcstold __P ((__const wchar_t *__restrict __nptr,
				     wchar_t **__restrict __endptr));
#endif /* C9x */


/* Convert initial portion of wide string NPTR to `long int'
   representation.  */
extern long int wcstol __P ((__const wchar_t *__restrict __nptr,
			     wchar_t **__restrict __endptr, int __base));

/* Convert initial portion of wide string NPTR to `unsigned long int'
   representation.  */
extern unsigned long int wcstoul __P ((__const wchar_t *__restrict __nptr,
				       wchar_t **__restrict __endptr,
				       int __base));

#if defined __GNUC__ && defined __USE_GNU
/* Convert initial portion of wide string NPTR to `long int'
   representation.  */
__extension__
extern long long int wcstoq __P ((__const wchar_t *__restrict __nptr,
				  wchar_t **__restrict __endptr, int __base));

/* Convert initial portion of wide string NPTR to `unsigned long long int'
   representation.  */
__extension__
extern unsigned long long int wcstouq __P ((__const wchar_t *__restrict __nptr,
					    wchar_t **__restrict __endptr,
					    int __base));
#endif /* GCC and use GNU.  */

#if defined __USE_ISOC9X || (defined __GNUC__ && defined __USE_GNU)
/* Convert initial portion of wide string NPTR to `long int'
   representation.  */
__extension__
extern long long int wcstoll __P ((__const wchar_t *__restrict __nptr,
				   wchar_t **__restrict __endptr, int __base));

/* Convert initial portion of wide string NPTR to `unsigned long long int'
   representation.  */
__extension__
extern unsigned long long int wcstoull __P ((__const wchar_t *
					     __restrict __nptr,
					     wchar_t **__restrict __endptr,
					     int __base));
#endif /* ISO C 9X or GCC and GNU.  */

#ifdef __USE_GNU
/* The concept of one static locale per category is not very well
   thought out.  Many applications will need to process its data using
   information from several different locales.  Another application is
   the implementation of the internationalization handling in the
   upcoming ISO C++ standard library.  To support this another set of
   the functions using locale data exist which have an additional
   argument.

   Attention: all these functions are *not* standardized in any form.
   This is a proof-of-concept implementation.  */

/* Structure for reentrant locale using functions.  This is an
   (almost) opaque type for the user level programs.  */
# include <xlocale.h>

/* Special versions of the functions above which take the locale to
   use as an additional parameter.  */
extern long int __wcstol_l __P ((__const wchar_t *__restrict __nptr,
				 wchar_t **__restrict __endptr, int __base,
				 __locale_t __loc));

extern unsigned long int __wcstoul_l __P ((__const wchar_t *__restrict __nptr,
					   wchar_t **__restrict __endptr,
					   int __base, __locale_t __loc));

__extension__
extern long long int __wcstoll_l __P ((__const wchar_t *__restrict __nptr,
				       wchar_t **__restrict __endptr,
				       int __base, __locale_t __loc));

__extension__
extern unsigned long long int __wcstoull_l __P ((__const wchar_t *__restrict
						 __nptr,
						 wchar_t **__restrict __endptr,
						 int __base,
						 __locale_t __loc));

extern double __wcstod_l __P ((__const wchar_t *__restrict __nptr,
			       wchar_t **__restrict __endptr,
			       __locale_t __loc));

extern float __wcstof_l __P ((__const wchar_t *__restrict __nptr,
			      wchar_t **__restrict __endptr,
			      __locale_t __loc));

extern __long_double_t __wcstold_l __P ((__const wchar_t *__restrict __nptr,
					 wchar_t **__restrict __endptr,
					 __locale_t __loc));
#endif /* GNU */


/* The internal entry points for `wcstoX' take an extra flag argument
   saying whether or not to parse locale-dependent number grouping.  */
extern double __wcstod_internal __P ((__const wchar_t *__restrict __nptr,
				      wchar_t **__restrict __endptr,
				      int __group));
extern float __wcstof_internal __P ((__const wchar_t *__restrict __nptr,
				     wchar_t **__restrict __endptr,
				     int __group));
extern __long_double_t __wcstold_internal __P ((__const wchar_t *
						__restrict __nptr,
						wchar_t **__restrict __endptr,
						int __group));

#ifndef __wcstol_internal_defined
extern long int __wcstol_internal __P ((__const wchar_t *__restrict __nptr,
					wchar_t **__restrict __endptr,
					int __base, int __group));
# define __wcstol_internal_defined	1
#endif
#ifndef __wcstoul_internal_defined
extern unsigned long int __wcstoul_internal __P ((__const wchar_t *
						  __restrict __nptr,
						  wchar_t **
						  __restrict __endptr,
						  int __base, int __group));
# define __wcstoul_internal_defined	1
#endif
#ifndef __wcstoll_internal_defined
__extension__
extern long long int __wcstoll_internal __P ((__const wchar_t *
					      __restrict __nptr,
					      wchar_t **__restrict __endptr,
					      int __base, int __group));
# define __wcstoll_internal_defined	1
#endif
#ifndef __wcstoull_internal_defined
__extension__
extern unsigned long long int __wcstoull_internal __P ((__const wchar_t *
							__restrict __nptr,
							wchar_t **
							__restrict __endptr,
							int __base,
							int __group));
# define __wcstoull_internal_defined	1
#endif


#if defined __OPTIMIZE__ && __GNUC__ >= 2
/* Define inline functions which call the internal entry points.  */

extern __inline double wcstod (__const wchar_t *__restrict __nptr,
			       wchar_t **__restrict __endptr) __THROW
{ return __wcstod_internal (__nptr, __endptr, 0); }
extern __inline long int wcstol (__const wchar_t *__restrict __nptr,
                                 wchar_t **__restrict __endptr,
				 int __base) __THROW
{ return __wcstol_internal (__nptr, __endptr, __base, 0); }
extern __inline unsigned long int wcstoul (__const wchar_t *__restrict __nptr,
                                           wchar_t **__restrict __endptr,
					   int __base) __THROW
{ return __wcstoul_internal (__nptr, __endptr, __base, 0); }

# ifdef __USE_GNU
extern __inline float wcstof (__const wchar_t *__restrict __nptr,
			      wchar_t **__restrict __endptr) __THROW
{ return __wcstof_internal (__nptr, __endptr, 0); }
extern __inline __long_double_t wcstold (__const wchar_t *__restrict __nptr,
					 wchar_t **__restrict __endptr) __THROW
{ return __wcstold_internal (__nptr, __endptr, 0); }


__extension__
extern __inline long long int wcstoq (__const wchar_t *__restrict __nptr,
				      wchar_t **__restrict __endptr,
				      int __base) __THROW
{ return __wcstoll_internal (__nptr, __endptr, __base, 0); }
__extension__
extern __inline unsigned long long int wcstouq (__const wchar_t *
						__restrict __nptr,
						wchar_t **__restrict __endptr,
						int __base) __THROW
{ return __wcstoull_internal (__nptr, __endptr, __base, 0); }
# endif /* Use GNU.  */
#endif /* Optimizing GCC >=2.  */


#ifdef	__USE_GNU
/* Copy SRC to DEST, returning the address of the terminating L'\0' in
   DEST.  */
extern wchar_t *wcpcpy __P ((wchar_t *__dest, __const wchar_t *__src));

/* Copy no more than N characters of SRC to DEST, returning the address of
   the last character written into DEST.  */
extern wchar_t *wcpncpy __P ((wchar_t *__dest, __const wchar_t *__src,
			      size_t __n));
#endif	/* use GNU */


/* Wide character I/O functions.  */
#ifdef __USE_ISOC9X

/* Select orientation for stream.  */
extern int fwide __P ((FILE *__fp, int __mode));


/* Write formatted output to STREAM.  */
extern int fwprintf __P ((FILE *__restrict __stream,
			  __const wchar_t *__restrict __format, ...))
     /* __attribute__ ((__format__ (__wprintf__, 2, 3))) */;
/* Write formatted output to stdout.  */
extern int wprintf __P ((__const wchar_t *__restrict __format, ...))
     /* __attribute__ ((__format__ (__wprintf__, 1, 2))) */;
/* Write formatted output of at most N characters to S.  */
extern int swprintf __P ((wchar_t *__restrict __s, size_t __n,
			  __const wchar_t *__restrict __format, ...))
     /* __attribute__ ((__format__ (__wprintf__, 3, 4))) */;

/* Write formatted output to S from argument list ARG.  */
extern int vfwprintf __P ((FILE *__restrict __s,
			   __const wchar_t *__restrict __format,
			   __gnuc_va_list __arg))
     /* __attribute__ ((__format__ (__wprintf__, 2, 0))) */;
/* Write formatted output to stdout from argument list ARG.  */
extern int vwprintf __P ((__const wchar_t *__restrict __format,
			  __gnuc_va_list __arg))
     /* __attribute__ ((__format__ (__wprintf__, 1, 0))) */;
/* Write formatted output of at most N character to S from argument
   list ARG.  */
extern int vswprintf __P ((wchar_t *__restrict __s, size_t __n,
			   __const wchar_t *__restrict __format,
			   __gnuc_va_list __arg))
     /* __attribute__ ((__format__ (__wprintf__, 3, 0))) */;


/* Read formatted input from STREAM.  */
extern int fwscanf __P ((FILE *__restrict __stream,
			 __const wchar_t *__restrict __format, ...))
     /* __attribute__ ((__format__ (__wscanf__, 2, 3))) */;
/* Read formatted input from stdin.  */
extern int wscanf __P ((__const wchar_t *__restrict __format, ...))
     /* __attribute__ ((__format__ (__wscanf__, 1, 2))) */;
/* Read formatted input from S.  */
extern int swscanf __P ((__const wchar_t *__restrict __s,
			 __const wchar_t *__restrict __format, ...))
     /* __attribute__ ((__format__ (__wscanf__, 2, 3))) */;

/* Read formatted input from S into argument list ARG.  */
extern int vfwscanf __P ((FILE *__restrict __s,
			  __const wchar_t *__restrict __format,
			  __gnuc_va_list __arg))
     /* __attribute__ ((__format__ (__wscanf__, 2, 0))) */;
/* Read formatted input from stdin into argument list ARG.  */
extern int vwscanf __P ((__const wchar_t *__restrict __format,
			 __gnuc_va_list __arg))
     /* __attribute__ ((__format__ (__wscanf__, 1, 0))) */;
/* Read formatted input from S into argument list ARG.  */
extern int vswscanf __P ((__const wchar_t *__restrict __s,
			  __const wchar_t *__restrict __format,
			  __gnuc_va_list __arg))
     /* __attribute__ ((__format__ (__wscanf__, 2, 0))) */;


/* Read a character from STREAM.  */
extern wint_t fgetwc __P ((FILE *__stream));
extern wint_t getwc __P ((FILE *__stream));

/* Read a character from stdin.  */
extern wint_t getwchar __P ((void));

#ifdef __USE_GNU
/* These are defined to be equivalent to the `char' functions defined
   in POSIX.1:1996.  */
extern wint_t getwc_unlocked __P ((FILE *__stream));
extern wint_t getwchar_unlocked __P ((void));

/* This is the wide character version of a GNU extension.  */
extern wint_t fgetwc_unlocked __P ((FILE *__stream));
#endif /* Use POSIX or MISC.  */


/* Write a character to STREAM.  */
extern wint_t fputwc __P ((wchar_t __wc, FILE *__stream));
extern wint_t putwc __P ((wchar_t __wc, FILE *__stream));

/* Write a character to stdout.  */
extern wint_t putwchar __P ((wchar_t __wc));

#ifdef __USE_GNU
/* Faster version when locking is not necessary.  */
extern wint_t fputwc_unlocked __P ((wchar_t __wc, FILE *__stream));

/* These are defined to be equivalent to the `char' functions defined
   in POSIX.1:1996.  */
extern wint_t putwc_unlocked __P ((wchar_t __wc, FILE *__stream));
extern wint_t putwchar_unlocked __P ((wchar_t __wc));
#endif


/* Get a newline-terminated wide character string of finite length
   from STREAM.  */
extern wchar_t *fgetws __P ((wchar_t *__restrict __ws, int __n,
			     FILE *__restrict __stream));

#ifdef __USE_GNU
/* This function does the same as `fgetws' but does not lock the stream.  */
extern wchar_t *fgetws_unlocked __P ((wchar_t *__restrict __ws, int __n,
				      FILE *__restrict __stream));
#endif


/* Write a string to STREAM.  */
extern int fputws __P ((__const wchar_t *__restrict __ws,
			FILE *__restrict __stream));

#ifdef __USE_GNU
/* This function does the same as `fputws' but does not lock the stream.  */
extern int fputws_unlocked __P ((__const wchar_t *__restrict __ws,
				 FILE *__restrict __stream));
#endif


/* Push a character back onto the input buffer of STREAM.  */
extern wint_t ungetwc __P ((wint_t __wc, FILE *__stream));


/* Format TP into S according to FORMAT.
   Write no more than MAXSIZE wide characters and return the number
   of wide characters written, or 0 if it would exceed MAXSIZE.  */
extern size_t wcsftime __P ((wchar_t *__restrict __s, size_t __maxsize,
			     __const wchar_t *__restrict __format,
			     __const struct tm *__restrict __tp));
#endif /* Use ISO C9x. */

/* The X/Open standard demands that most of the functions defined in
   the <wctype.h> header must also appear here.  This is probably
   because some X/Open members wrote their implementation before the
   ISO C standard was published and introduced the better solution.
   We have to provide these definitions for compliance reasons but we
   do this nonsense only if really necessary.  */
#if defined __USE_UNIX98 && !defined __USE_GNU
# define __need_iswxxx
# include <wctype.h>
#endif

__END_DECLS

#endif	/* _WCHAR_H defined */

#endif /* wchar.h  */
