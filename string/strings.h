/* Copyright (C) 1991, 1992, 1996, 1997 Free Software Foundation, Inc.
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

#ifndef	_STRINGS_H
#define	_STRINGS_H	1

#include <features.h>
#define __need_size_t
#include <stddef.h>

__BEGIN_DECLS

/* Compare N bytes of S1 and S2 (same as memcmp).  */
extern int bcmp __P ((__const __ptr_t __s1, __const __ptr_t __s2, size_t __n));

/* Copy N bytes of SRC to DEST (like memmove, but args reversed).  */
extern void bcopy __P ((__const __ptr_t __src, __ptr_t __dest, size_t __n));

/* Set N bytes of S to 0.  */
extern void bzero __P ((__ptr_t __s, size_t __n));

/* Return the position of the first bit set in I, or 0 if none are set.
   The least-significant bit is position 1, the most-significant 32.  */
extern int ffs __P ((int __i));

/* Find the first occurrence of C in S (same as strchr).  */
extern char *index __P ((__const char *__s, int __c));

/* Find the last occurrence of C in S (same as strrchr).  */
extern char *rindex __P ((__const char *__s, int __c));

/* Compare S1 and S2, ignoring case.  */
extern int strcasecmp __P ((__const char *__s1, __const char *__s2));

/* Compare no more than N chars of S1 and S2, ignoring case.  */
extern int strncasecmp __P ((__const char *__s1, __const char *__s2,
			     size_t __n));

__END_DECLS

#endif	/* strings.h  */
