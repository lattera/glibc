/* Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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

#define _WCHAR_H 1
#include <features.h>

__BEGIN_DECLS

/* Get size_t, wchar_t, wint_t and NULL from <stddef.h>.  */
#define __need_size_t
#define __need_wchar_t
#define __need_wint_t
#define __need_NULL
#include <stddef.h>


/* We try to get wint_t from <stddef.h>, but not all GCC versions define it
   there.  So define it ourselves if it remains undefined.  */
#ifndef _WINT_T
/* Integral type unchanged by default argument promotions that can
   hold any value corresponding to members of the extended character
   set, as well as at least one value that does not correspond to any
   member of the extended character set.  */
#define _WINT_T
typedef unsigned int wint_t;
#endif


/* Conversion state information.  */
typedef struct
{
  int count;		/* Number of bytes needed for the current character. */
  wint_t value;		/* Value so far.  */
} mbstate_t;

#define WCHAR_MIN ((wchar_t) 0)
#define WCHAR_MAX (~WCHAR_MIN)

#ifndef WEOF
# define WEOF (0xffffffffu)
#endif


/* Copy SRC to DEST.  */
extern wchar_t *wcscpy __P ((wchar_t *__dest, __const wchar_t *__src));
/* Copy no more than N wide-characters of SRC to DEST.  */
extern wchar_t *wcsncpy __P ((wchar_t *__dest, __const wchar_t *__src,
			      size_t __n));

/* Append SRC onto DEST.  */
extern wchar_t *wcscat __P ((wchar_t *__dest, __const wchar_t *__src));
/* Append no more than N wide-characters of SRC onto DEST.  */
extern wchar_t *wcsncat __P ((wchar_t *__dest, __const wchar_t *__src,
			      size_t __n));

/* Compare S1 and S2.  */
extern int wcscmp __P ((__const wchar_t *__s1, __const wchar_t *__s2));
/* Compare N wide-characters of S1 and S2.  */
extern int wcsncmp __P ((__const wchar_t *__s1, __const wchar_t *__s2,
			 size_t __n));

/* Compare S1 and S2, both interpreted as appropriate to the
   LC_COLLATE category of the current locale.  */
extern int wcscoll __P ((__const wchar_t *__s1, __const wchar_t *__s2));
/* Transform S2 into array pointed to by S1 such that if wcscmp is
   applied to two transformed strings the result is the as applying
   `wcscoll' to the original strings.  */
extern size_t wcsxfrm __P ((wchar_t *__s1, __const wchar_t *__s2, size_t __n));

/* Duplicate S, returning an identical malloc'd string.  */
extern wchar_t *wcsdup __P ((__const wchar_t *__s));

/* Find the first occurrence of WC in WCS.  */
extern wchar_t *wcschr __P ((__const wchar_t *__wcs, wchar_t __wc));
/* Find the last occurrence of WC in WCS.  */
extern wchar_t *wcsrchr __P ((__const wchar_t *__wcs, wchar_t __wc));

/* Return the length of the initial segmet of WCS which
   consists entirely of wide-characters not in REJECT.  */
extern size_t wcscspn __P ((__const wchar_t *__wcs,
			    __const wchar_t *__reject));
/* Return the length of the initial segmet of WCS which
   consists entirely of wide-characters in  ACCEPT.  */
extern size_t wcsspn __P ((__const wchar_t *__wcs, __const wchar_t *__accept));
/* Find the first occurrence in WCS of any character in ACCEPT.  */
extern wchar_t *wcspbrk __P ((__const wchar_t *__wcs,
			      __const wchar_t *__accept));
/* Find the first occurrence of NEEDLE in HAYSTACK.  */
extern wchar_t *wcsstr __P ((__const wchar_t *__haystack,
			     __const wchar_t *__needle));
/* Divide WCS into tokens separated by characters in DELIM.  */
extern wchar_t *wcstok __P ((wchar_t *__s, __const wchar_t *__delim,
			     wchar_t **ptr));

/* Return the number of wide-characters in S.  */
extern size_t wcslen __P ((__const wchar_t *__s));


/* Search N bytes of S for C.  */
extern wchar_t *wmemchr __P ((__const wchar_t *__s, wchar_t __c, size_t __n));

/* Compare N bytes of S1 and S2.  */
extern int wmemcmp __P ((__const wchar_t *__s1, __const wchar_t *__s2,
			 size_t __n));

/* Copy N bytes of SRC to DEST.  */
extern wchar_t *wmemcpy __P ((wchar_t *__s1, __const wchar_t *__s2,
			      size_t __n));

/* Copy N bytes of SRC to DEST, guaranteeing
   correct behavior for overlapping strings.  */
extern wchar_t *wmemmove __P ((wchar_t *__s1, __const wchar_t *__s2,
			       size_t __n));

/* Set N bytes of S to C.  */
extern wchar_t *wmemset __P ((wchar_t *__s, wchar_t __c, size_t __n));


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
extern size_t __mbrtowc __P ((wchar_t *__pwc, __const char *__s, size_t __n,
			      mbstate_t *__p));
extern size_t mbrtowc __P ((wchar_t *__pwc, __const char *__s, size_t __n,
			    mbstate_t *__p));

/* Write multibyte representation of wide character WC to S.  */
extern size_t __wcrtomb __P ((char *__s, wchar_t __wc, mbstate_t *__ps));
extern size_t wcrtomb __P ((char *__s, wchar_t __wc, mbstate_t *__ps));

/* Return number of bytes in multibyte character pointed to by S.  */
extern size_t __mbrlen __P ((__const char *__s, size_t __n, mbstate_t *__ps));
extern size_t mbrlen __P ((__const char *__s, size_t __n, mbstate_t *__ps));

#if defined (__OPTIMIZE__) \
    && (__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7))
/* Define inline function as optimization.  */
extern __inline size_t mbrlen (__const char *s, size_t n, mbstate_t *ps)
{ return ps != NULL ? __mbrtowc (NULL, s, n, ps) : __mbrlen (s, n, NULL); }
#endif

/* Write wide character representation of multibyte character string
   SRC to DST.  */
extern size_t __mbsrtowcs __P ((wchar_t *__dst, __const char **__src,
				size_t __len, mbstate_t *__ps));
extern size_t mbsrtowcs __P ((wchar_t *__dst, __const char **__src,
			      size_t __len, mbstate_t *__ps));

/* Write multibyte character representation of wide character string
   SRC to DST.  */
extern size_t __wcsrtombs __P ((char *__dst, __const wchar_t **__src,
				size_t __len, mbstate_t *__ps));
extern size_t wcsrtombs __P ((char *__dst, __const wchar_t **__src,
			      size_t __len, mbstate_t *__ps));


#ifdef	__USE_GNU
/* Write wide character representation of at most NMC bytes of the
   multibyte character string SRC to DST.  */
extern size_t __mbsnrtowcs __P ((wchar_t *__dst, __const char **__src,
				 size_t __nmc, size_t __len, mbstate_t *__ps));
extern size_t mbsnrtowcs __P ((wchar_t *__dst, __const char **__src,
			       size_t __nmc, size_t __len, mbstate_t *__ps));

/* Write multibyte character representation of at most NWC characters
   from the wide character string SRC to DST.  */
extern size_t __wcsnrtombs __P ((char *__dst, __const wchar_t **__src,
				 size_t __nwc, size_t __len, mbstate_t *__ps));
extern size_t wcsnrtombs __P ((char *__dst, __const wchar_t **__src,
			       size_t __nwc, size_t __len, mbstate_t *__ps));


/* The following functions are extensions found in X/Open CAE.  */

/* Determine number of column positions required for C.  */
extern int wcwidth __P ((wint_t __c));

/* Determine number of column positions required for first N wide
   characters (or fewer if S ends before this) in S.  */
extern int wcswidth __P ((__const wchar_t *__s, size_t __n));
#endif	/* use GNU */


/* Convert initial portion of the wide string NPTR to `double'
   representation.  */
extern double wcstod __P ((__const wchar_t *__nptr, wchar_t **__endptr));

#ifdef __USE_GNU
/* Likewise for `float' and `long double' sizes of floating-point numbers.  */
extern float wcstof __P ((__const wchar_t *__nptr, wchar_t **__endptr));
extern __long_double_t wcstold __P ((__const wchar_t *__nptr,
				     wchar_t **__endptr));
#endif /* GNU */


/* Convert initial portion of wide string NPTR to `long int'
   representation.  */
extern long int wcstol __P ((__const wchar_t *__nptr, wchar_t **__endptr,
			     int __base));

/* Convert initial portion of wide string NPTR to `unsigned long int'
   representation.  */
extern unsigned long int wcstoul __P ((__const wchar_t *__nptr,
				       wchar_t **__endptr, int __base));

#if defined (__GNUC__) && defined (__USE_GNU)
/* Convert initial portion of wide string NPTR to `long int'
   representation.  */
extern long long int wcstoq __P ((__const wchar_t *__nptr, wchar_t **__endptr,
				  int __base));

/* Convert initial portion of wide string NPTR to `unsigned long long int'
   representation.  */
extern unsigned long long int wcstouq __P ((__const wchar_t *__nptr,
					    wchar_t **__endptr, int __base));
#endif /* GCC and use GNU.  */


/* The internal entry points for `wcstoX' take an extra flag argument
   saying whether or not to parse locale-dependent number grouping.  */
extern double __wcstod_internal __P ((__const wchar_t *__nptr,
				      wchar_t **__endptr, int __group));
extern float __wcstof_internal __P ((__const wchar_t *__nptr,
				     wchar_t **__endptr, int __group));
extern __long_double_t __wcstold_internal __P ((__const wchar_t *__nptr,
						wchar_t **__endptr,
						int __group));

extern long int __wcstol_internal __P ((__const wchar_t *__nptr,
					wchar_t **__endptr, int __base,
					int __group));
extern unsigned long int __wcstoul_internal __P ((__const wchar_t *__nptr,
						  wchar_t **__endptr,
						  int __base, int __group));
#if defined __GNUC__ && defined __USE_GNU
extern long long int __wcstoq_internal __P ((__const wchar_t *__nptr,
					     wchar_t **__endptr, int __base,
					     int __group));
extern unsigned long long int __wcstouq_internal __P ((__const wchar_t *__nptr,
						       wchar_t **__endptr,
						       int __base,
						       int __group));
#endif /* GCC and use GNU.  */


#if defined (__OPTIMIZE__) && __GNUC__ >= 2
/* Define inline functions which call the internal entry points.  */

extern __inline double wcstod (__const wchar_t *__nptr, wchar_t **__endptr)
{ return __wcstod_internal (__nptr, __endptr, 0); }
extern __inline long int wcstol (__const wchar_t *__nptr,
                                 wchar_t **__endptr, int __base)
{ return __wcstol_internal (__nptr, __endptr, __base, 0); }
extern __inline unsigned long int wcstoul (__const wchar_t *__nptr,
                                           wchar_t **__endptr, int __base)
{ return __wcstoul_internal (__nptr, __endptr, __base, 0); }

#ifdef __USE_GNU
extern __inline float wcstof (__const wchar_t *__nptr, wchar_t **__endptr)
{ return __wcstof_internal (__nptr, __endptr, 0); }
extern __inline __long_double_t wcstold (__const wchar_t *__nptr,
					 wchar_t **__endptr)
{ return __wcstold_internal (__nptr, __endptr, 0); }


extern __inline long long int wcstoq (__const wchar_t *__nptr,
				      wchar_t **__endptr, int __base)
{ return __wcstoq_internal (__nptr, __endptr, __base, 0); }
extern __inline unsigned long long int wcstouq (__const wchar_t *__nptr,
						wchar_t **__endptr, int __base)
{ return __wcstouq_internal (__nptr, __endptr, __base, 0); }
#endif /* Use GNU.  */
#endif /* Optimizing GCC >=2.  */


#ifdef	__USE_GNU
/* Copy SRC to DEST, returning the address of the terminating L'\0' in
   DEST.  */
extern wchar_t *__wcpcpy __P ((wchar_t *__dest, __const wchar_t *__src));
extern wchar_t *wcpcpy __P ((wchar_t *__dest, __const wchar_t *__src));

/* Copy no more than N characters of SRC to DEST, returning the address of
   the last character written into DEST.  */
extern wchar_t *__wcpncpy __P ((wchar_t *__dest, __const wchar_t *__src,
				size_t __n));
extern wchar_t *wcpncpy __P ((wchar_t *__dest, __const wchar_t *__src,
			      size_t __n));
#endif	/* use GNU */


__END_DECLS

#endif /* wchar.h  */
