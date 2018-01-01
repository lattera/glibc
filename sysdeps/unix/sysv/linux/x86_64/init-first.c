/* Initialization code run first thing by the ELF startup code.  Linux/x86-64.
   Copyright (C) 2007-2018 Free Software Foundation, Inc.
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
# include <libc-vdso.h>

long int (*VDSO_SYMBOL(clock_gettime)) (clockid_t, struct timespec *)
  attribute_hidden;
long int (*VDSO_SYMBOL(getcpu)) (unsigned *, unsigned *, void *)
  attribute_hidden;

extern __typeof (clock_gettime) __syscall_clock_gettime attribute_hidden;


static inline void
__vdso_platform_setup (void)
{
  PREPARE_VERSION (linux26, "LINUX_2.6", 61765110);

  void *p = _dl_vdso_vsym ("__vdso_clock_gettime", &linux26);
  if (p == NULL)
    p = __syscall_clock_gettime;
  PTR_MANGLE (p);
  VDSO_SYMBOL(clock_gettime) = p;

  p = _dl_vdso_vsym ("__vdso_getcpu", &linux26);
  PTR_MANGLE (p);
  VDSO_SYMBOL(getcpu) = p;
}

# define VDSO_SETUP __vdso_platform_setup
#endif

#include <csu/init-first.c>
