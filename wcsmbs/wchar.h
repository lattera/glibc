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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *      ISO Standard: 7.16.4 General wide-string utilities	<wchar.h>
 */

#ifndef _WCHAR_H

#define _WCHAR_H 1
#include <features.h>

__BEGIN_DECLS

/* Get size_t, wchar_t, uwchar_t and NULL from <stddef.h>.  */
#define __need_size_t
#define __need_wchar_t
/* #define __need_uwchar_t */
#define __need_NULL
/* __need_WCHAR_MAX */
/* __need_WCHAR_MIN */
#include <stddef.h>

/* FIXME: Should go with this or another name in stddef.h.  */
typedef unsigned int uwchar_t;

/* Conversion state information.  */
typedef int mbstate_t; /* FIXME */

/* Should come from <stddef.h> */
#define WCHAR_MIN ((wchar_t) 0)	/* FIXME */
#define WCHAR_MAX (~WCHAR_MIN)	/* FIXME */

#ifndef WEOF
# define WEOF (0xffffffffu)
#endif

/* FIXME: should this go into <stddef.h>???  */
#if 0
#define __need_wint_t
#include <stddef.h>
#else
/* Integral type unchanged by default argument promotions that can
   hold any value corresponding to members of the extended character
   set, as well as at least one value that does not correspond to any
   member of the extended character set.  */
typedef unsigned int wint_t;
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

/* Duplicate S, returning an identical malloc'd string.  */
extern wchar_t *wcsdup __P ((__const wchar_t *__s));

/* Find the first occurence of WC in WCS.  */
extern wchar_t *wcschr __P ((__const wchar_t *__wcs, wchar_t __wc));
/* Find the last occurence of WC in WCS.  */
extern wchar_t *wcsrchr __P ((__const wchar_t *__wcs, wchar_t __wc));

/* Return the length of the initial segmet of WCS which
   consists entirely of wide-characters not in REJECT.  */
extern size_t wcscspn __P ((__const wchar_t *__wcs,
			    __const wchar_t *__reject));
/* Return the length of the initial segmet of WCS which
   consists entirely of wide-characters in  ACCEPT.  */
extern size_t wcsspn __P ((__const wchar_t *__wcs, __const wchar_t *__accept));
/* Find the first occurence in WCS of any character in ACCEPT.  */
extern wchar_t *wcspbrk __P ((__const wchar_t *__wcs,
			      __const wchar_t *__accept));
/* Find the first occurence of NEEDLE in HAYSTACK.  */
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
			       size_t __N));

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

/* Return number of bytes in multibyte character pointed to by S.  */
extern size_t mbrlen __P ((__const char *__s, size_t __n, mbstate_t *ps));

/* Write wide character representation of multibyte character pointed
   to by S to PWC.  */
extern size_t mbrtowc __P ((wchar_t *__pwc, __const char *__s, size_t __n,
			    mbstate_t *__p));

/* Write multibyte representation of wide character WC to S.  */
extern size_t wcrtomb __P ((char *__s, wchar_t __wc, mbstate_t *__ps));

/* Write wide character representation of multibyte chracter string SRC
   to DST.  */
extern size_t mbsrtowcs __P ((wchar_t *__dst, __const char **__src,
			      size_t __len, mbstate_t *__ps));

/* Write multibyte character representation of wide character string
   SRC to DST.  */
extern size_t wcsrtombs __P ((char *__dst, __const wchar_t **__src,
			      size_t __len, mbstate_t *__ps));

__END_DECLS

#endif /* wchar.h  */
