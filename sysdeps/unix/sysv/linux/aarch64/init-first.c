/* Copyright (C) 2007-2016 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifdef SHARED
# include <dl-vdso.h>
# include <libc-vdso.h>

int (*VDSO_SYMBOL(gettimeofday)) (struct timeval *, void *) attribute_hidden;
int (*VDSO_SYMBOL(clock_gettime)) (clockid_t, struct timespec *);
int (*VDSO_SYMBOL(clock_getres)) (clockid_t, struct timespec *);

static inline void
_libc_vdso_platform_setup (void)
{
#ifdef __LP64__
  PREPARE_VERSION (linux_version, "LINUX_2.6.39", 123718537);
#else
  PREPARE_VERSION (linux_version, "LINUX_4.9", 61765625);
#endif

  void *p = _dl_vdso_vsym ("__kernel_gettimeofday", &linux_version);
  PTR_MANGLE (p);
  VDSO_SYMBOL(gettimeofday) = p;

  p = _dl_vdso_vsym ("__kernel_clock_gettime", &linux_version);
  PTR_MANGLE (p);
  VDSO_SYMBOL(clock_gettime) = p;

  p = _dl_vdso_vsym ("__kernel_clock_getres", &linux_version);
  PTR_MANGLE (p);
  VDSO_SYMBOL(clock_getres) = p;
}

# define VDSO_SETUP _libc_vdso_platform_setup
#endif

#include <csu/init-first.c>
