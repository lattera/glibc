/* Copyright (C) 2008 Free Software Foundation, Inc.
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

#ifdef SHARED
# include <dl-vdso.h>
# undef __gettimeofday
# undef __clock_gettime
# undef __clock_getres
# include <bits/libc-vdso.h>

long int (*__vdso_gettimeofday) (struct timeval *, void *) attribute_hidden;

long int (*__vdso_clock_gettime) (clockid_t, struct timespec *)
  __attribute__ ((nocommon));
strong_alias (__vdso_clock_gettime, __GI___vdso_clock_gettime attribute_hidden)

long int (*__vdso_clock_getres) (clockid_t, struct timespec *)
  __attribute__ ((nocommon));
strong_alias (__vdso_clock_getres, __GI___vdso_clock_getres attribute_hidden)


static inline void
_libc_vdso_platform_setup (void)
{
  PREPARE_VERSION (linux2629, "LINUX_2.6.29", 123718585);

  __vdso_gettimeofday = _dl_vdso_vsym ("__kernel_gettimeofday", &linux2629);
  __vdso_clock_gettime = _dl_vdso_vsym ("__kernel_clock_gettime", &linux2629);
  __vdso_clock_getres = _dl_vdso_vsym ("__kernel_clock_getres", &linux2629);
}

# define VDSO_SETUP _libc_vdso_platform_setup
#endif

#include "../init-first.c"
