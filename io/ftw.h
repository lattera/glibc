/* Copyright (C) 1992, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ian Lance Taylor (ian@airs.com).

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
 *	X/Open Portability Guide 4.2: ftw.h
 */

#ifndef _FTW_H

#define	_FTW_H	1
#include <features.h>

#include <statbuf.h>

/* The FLAG argument to the user function passed to ftw.  */
#define FTW_F	0		/* Regular file.  */
#define FTW_D	1		/* Directory.  */
#define FTW_DNR	2		/* Unreadable directory.  */
#define FTW_NS	3		/* Unstatable file.  */

__BEGIN_DECLS

/* Call a function on every element in a directory tree.  */
extern int ftw __P ((__const char *__dir,
		     int (*__func) (__const char *__file,
				    __const struct stat *__status,
				    int __flag),
		     int __descriptors));

__END_DECLS

#endif	/* ftw.h */
