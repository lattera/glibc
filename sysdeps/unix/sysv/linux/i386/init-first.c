/* Initialization code run first thing by the ELF startup code.  Linux/i386.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#ifdef SHARED
# include <time.h>
# include <sysdep.h>
# include <dl-vdso.h>
# include <sysdep-vdso.h>

long int (*VDSO_SYMBOL (clock_gettime)) (clockid_t, struct timespec *)
  attribute_hidden;

static long int
clock_gettime_syscall (clockid_t id, struct timespec *tp)
{
  INTERNAL_SYSCALL_DECL (err);
  return INTERNAL_SYSCALL (clock_gettime, err, 2, id, tp);
}

static inline void
__vdso_platform_setup (void)
{
  PREPARE_VERSION_KNOWN (linux26, LINUX_2_6);

  void *p = _dl_vdso_vsym ("__vdso_clock_gettime", &linux26);
  if (p == NULL)
    p = clock_gettime_syscall;
  PTR_MANGLE (p);
  VDSO_SYMBOL (clock_gettime) = p;
}

# define VDSO_SETUP __vdso_platform_setup
#endif

#include <csu/init-first.c>
