/* timespec_get -- returns the calendar time based on a given time base.
   Linux/x86 version.
   Copyright (C) 2015 Free Software Foundation, Inc.
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

#include <libc-vdso.h>

#ifdef SHARED
# define INTERNAL_GETTIME(id, tp) \
  ({ long int (*f) (clockid_t, struct timespec *) = __vdso_clock_gettime; \
  PTR_DEMANGLE (f);							  \
  (*f) (id, tp); })
#endif

#include <sysdeps/unix/sysv/linux/timespec_get.c>
