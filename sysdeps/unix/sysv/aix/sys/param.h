/* Copyright (C) 2000 Free Software Foundation, Inc.
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

#ifndef _SYS_PARAM_H
#define _SYS_PARAM_H	1

/* This file should contain various parameter macros appropriate for the
   machine and operating system.  There is no standard set of macros; this
   file is just for compatibility with programs written for Unix that
   expect it to define things.  On Unix systems that do not have their own
   sysdep version of this file, it is generated at build time by examining
   the installed headers on the system.  */

#include <limits.h>

#define MAXSYMLINKS  1
#define MAXPATHLEN   256

/* The pagesize is 4096.  */
#define EXEC_PAGESIZE	4096

/* maximum number of supplemental groups.  */
#define NGROUPS		32

/* Macros for min/max.  */
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


/* Maximum length of hostname.  */
#define MAXHOSTNAMELEN	256

#endif	/* sys/param.h */
