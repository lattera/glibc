/* Copyright (C) 1998,2002 Free Software Foundation, Inc.
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

/* sys_errlist cannot have Unix semantics on the Hurd, so it is easier just
   to rename it.  We also need to remap error codes to array indices by
   taking their subcode. */
#define _sys_errlist_internal	_hurd_errlist
#define _sys_nerr_internal	_hurd_nerr
#define ERRLIST_NO_COMPAT	1

#include <mach/error.h>
#define ERR_REMAP(n) (err_get_code (n))

#include <sysdeps/gnu/errlist.c>
