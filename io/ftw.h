/* Copyright (C) 1992 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *	SVID ftw.h
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

/* Call a function on every element in a directory tree.  */
extern int ftw __P ((__const char *__dir,
		     int (*__func) (__const char *__file,
				    struct stat *__status,
				    int __flag),
		     int __descriptors));

#endif	/* ftw.h */
