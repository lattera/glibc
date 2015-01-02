/* Copyright (C) 1991-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <stddef.h>


const char *const _sys_errlist[] =
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

const int _sys_nerr = sizeof (_sys_errlist) / sizeof (_sys_errlist[0]);
