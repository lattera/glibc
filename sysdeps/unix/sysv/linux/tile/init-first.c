/* Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

#ifdef SHARED
#include <dl-vdso.h>
#include <libc-vdso.h>

struct syscall_return_value (*VDSO_SYMBOL(gettimeofday)) (struct timeval *,
							  void *)
  attribute_hidden;

struct syscall_return_value (*VDSO_SYMBOL(clock_gettime)) (clockid_t,
                                                           struct timespec *)
  __attribute__ ((nocommon));


static inline void
_libc_vdso_platform_setup (void)
{
  PREPARE_VERSION (linux26, "LINUX_2.6", 61765110);

  void *p = _dl_vdso_vsym ("__vdso_gettimeofday", &linux26);
  PTR_MANGLE (p);
  VDSO_SYMBOL (gettimeofday) = p;

  p = _dl_vdso_vsym ("__vdso_clock_gettime", &linux26);
  PTR_MANGLE (p);
  VDSO_SYMBOL (clock_gettime) = p;
}

#define VDSO_SETUP _libc_vdso_platform_setup
#endif

#include <csu/init-first.c>
