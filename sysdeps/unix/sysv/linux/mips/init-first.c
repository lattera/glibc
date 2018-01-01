/* Initialization code run first thing by the ELF startup code.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.

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

static inline void
_libc_vdso_platform_setup (void)
{
  PREPARE_VERSION_KNOWN (linux26, LINUX_2_6);

  void *p = _dl_vdso_vsym ("__vdso_gettimeofday", &linux26);
  PTR_MANGLE (p);
  VDSO_SYMBOL (gettimeofday) = p;

  p = _dl_vdso_vsym ("__vdso_clock_gettime", &linux26);
  PTR_MANGLE (p);
  VDSO_SYMBOL (clock_gettime) = p;
}

# define VDSO_SETUP _libc_vdso_platform_setup
#endif

#include <csu/init-first.c>
