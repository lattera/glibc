/* Copyright (C) 1991, 1994 Free Software Foundation, Inc.
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

/* This file defines the `errno' constants.  */

#if !defined(__Emath_defined) && (defined(_ERRNO_H) || defined(__need_Emath))
#undef	__need_Emath
#define	__Emath_defined	1

#define	EDOM	1
#define	ERANGE	2
#endif

#ifdef	_ERRNO_H
#define	ENOSYS	3
#define	EINVAL	4
#define	ESPIPE	5
#define	EBADF	6
#define	ENOMEM	7
#define	EACCES	8
#define ENFILE  9
#define EMFILE  10
#endif
