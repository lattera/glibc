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

#include <ansidecl.h>
#include <stddef.h>

#ifndef HAVE_GNU_LD
#define	_sys_errlist	sys_errlist
#define	_sys_nerr	sys_nerr
#endif

CONST char *CONST _sys_errlist[] =
  {
    "Error 0",			/* 0 */
    "Argument out of function's domain", /* 1 = EDOM */
    "Result out of range",	/* 2 = ERANGE */
    "Operation not implemented", /* 3 = ENOSYS */
    "Invalid argument",		/* 4 = EINVAL */
    "Illegal seek",		/* 5 = ESPIPE */
    "Bad file descriptor",	/* 6 = EBADF */
    "Cannot allocate memory",	/* 7 = ENOMEM */
    "Permission denied",	/* 8 = EACCES */
    "Too many open files in system", /* 9 = ENFILE */
    "Too many open files",	/* 10 = EMFILE */
  };

CONST int _sys_nerr = sizeof (_sys_errlist) / sizeof (_sys_errlist[0]);
