/* Resolve function pointers to VDSO functions.
   Copyright (C) 2012-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */


#ifndef _LIBC_VDSO_H
#define _LIBC_VDSO_H

#ifdef SHARED

#include <sysdep-vdso.h>

struct syscall_return_value
{
  long int value;
  long int error;
};

extern struct syscall_return_value (*VDSO_SYMBOL (gettimeofday)) (struct
								  timeval *,
								  void *)
  attribute_hidden;

extern struct syscall_return_value (*VDSO_SYMBOL (clock_gettime)) (clockid_t,
								   struct
								   timespec *);
#endif
#endif /* _LIBC_VDSO_H */
