/* Copyright (C) 1991 Free Software Foundation, Inc.
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

#ifndef	_VARARGS_H

#define	_VARARGS_H	1
#include <features.h>

#ifdef	__GNUC__

#define va_alist  __builtin_va_alist
#define va_dcl    int __builtin_va_alist;
#define va_list   char *

#ifdef __sparc__
#define va_start(AP) 						\
 (__builtin_saveregs (),					\
  AP = ((void *) &__builtin_va_alist))
#else
#define va_start(AP)  AP=(char *) &__builtin_va_alist
#endif
#define va_end(AP)

#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#define va_arg(AP, TYPE)					\
 (*((TYPE *) (AP += __va_rounded_size (TYPE),			\
	      AP - __va_rounded_size (TYPE))))

#else	/* Not GCC.  */

/* Implement varargs on top of our stdarg implementation.  */

#include <stdarg.h>

#define	va_alist	__va_fakearg
#define	va_dcl		int __va_fakearg;

#undef	va_start
#define	va_start(ap)	(__va_start((ap), __va_fakearg), \
			 (ap) -= sizeof(__va_fakearg))

#endif	/* GCC.  */

#endif	/* varargs.h  */
