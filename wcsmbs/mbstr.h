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
not, write to the, Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef _MBSTRING_H

#define _MBSTRING_H 1
#include <features.h>
#include <limits.h>

#define __need_size_t
#include <stddef.h>

__BEGIN_DECLS

/* This data type should be large enough to contain MB_CUR_MAX bytes.  */
typedef unsigned int mbchar_t;


/* Copy SRC to DEST.  */
extern char *mbscpy __P ((char *__dest, __const char *__src));
/* Copy no more than N multi-byte characters of SRC to DEST.  */
extern char *mbsncpy __P ((char *__dest, __const char *__src, size_t __n));

/* Append SRC onto DEST.  */
extern char *mbscat __P ((char *__dest, __const char *__src));
/* Append no more than N characters from SRC onto DEST.  */
extern char *mbsncat __P ((char *__dest, __const char *__src, size_t __n));

/* Compare S1 and S2.  */
extern int mbscmp __P ((__const char *__s1, __const char *__s2));
/* Compare N characters of S1 and S2.  */
extern int mbsncmp __P ((__const char *__s1, __const char *__s2, size_t __n));

/* Duplicate MBS, returning an identical malloc'd string.  */
extern char *mbsdup __P ((__const char *__s));

/* Find the first occurence of MBC in MBS.  */
extern char *mbschr __P ((__const char *__mbs, mbchar_t mbc));
/* Find the last occurence of MBC in MBS.  */
extern char *mbsrchr __P ((__const char *__mbs, mbchar_t mbc));

/* Return the length of MBS.  */
extern size_t mbslen __P ((__const char *__mbs));


/* Advance pointer to multibyte string by one character.  */
extern char *mbsadvance __P ((__const char *__mbs));

/* Return first character in MBS.  */
extern mbchar_t mbstomb __P ((__const char *__mbs));

__END_DECLS

#endif /* mbstring.h */
