/* Copyright (C) 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* SYS_ERRLIST cannot have Unix semantics on the Hurd, so it is easier
   just to rename it.  We also need to remap error codes to array
   indices by taking their subcode. */
#define SYS_ERRLIST _hurd_errlist
#define SYS_NERR _hurd_nerr

#include <mach/error.h>
#define ERR_REMAP(n) (err_get_code (n))

#include <sysdeps/gnu/errlist.c>

/* Oblige programs that use sys_nerr, but don't use sys_errlist. */
weak_alias (_hurd_nerr, sys_nerr)
weak_alias (_hurd_nerr, _sys_nerr)
