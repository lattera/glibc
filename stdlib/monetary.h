/* Header file for monetary value formatting functions.
   Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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

#ifndef	_MONETARY_H
#define	_MONETARY_H	1

#include <features.h>

/* Get needed types.  */
#include <sys/types.h>

__BEGIN_DECLS

/* Formatting a monetary value according to the current locale.  */
extern ssize_t strfmon __P ((char *__restrict __s, size_t __maxsize,
			     __const char *__restrict __format, ...));

#ifdef __USE_GNU
# include <xlocale.h>

/* Formatting a monetary value according to the current locale.  */
extern ssize_t __strfmon_l __P ((char *__restrict __s, size_t __maxsize,
				 __locale_t loc,
				 __const char *__restrict __format, ...));
#endif

__END_DECLS

#endif	/* monetary.h */
