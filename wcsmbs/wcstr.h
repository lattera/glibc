/* Copyright (C) 1995 Free Software Foundation, Inc.
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
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef _WCSTRING_H

#define _WCSTRING_H 1
#include <features.h>

__BEGIN_DECLS

/* Get size_t, wchar_t, uwchar_t and NULL from <stddef.h>.  */
#define __need_size_t
#define __need_wchar_t
/* #define __need_uwchar_t */
#define __need_NULL
#include <stddef.h>

/* FIXME: Should go with this or another name in stddef.h.  */
typedef unsigned int uwchar_t;


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
extern wchar_t *wcswcs __P ((__const wchar_t *__haystack,
			     __const wchar_t *__needle));
/* Divide WCS into tokens separated by characters in DELIM.  */
extern wchar_t *wcstok __P ((wchar_t *__s, __const wchar_t *__delim));

/* Return the number of wide-characters in S.  */
extern size_t wcslen __P ((__const wchar_t *__s));

__END_DECLS

#endif /* wcstring.h */
