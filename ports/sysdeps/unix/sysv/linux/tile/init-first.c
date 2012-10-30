/* Copyright (C) 2012 Free Software Foundation, Inc.
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
#include <bits/libc-vdso.h>

long int (*__vdso_gettimeofday) (struct timeval *, void *) attribute_hidden;

static inline void
_libc_vdso_platform_setup (void)
{
  PREPARE_VERSION (linux26, "LINUX_2.6", 61765110);
  __vdso_gettimeofday = _dl_vdso_vsym ("__vdso_gettimeofday", &linux26);
}

#define VDSO_SETUP _libc_vdso_platform_setup
#endif

#include <csu/init-first.c>
