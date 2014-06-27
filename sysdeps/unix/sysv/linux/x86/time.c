/* time -- Get number of seconds since Epoch.  Linux/x86 version.
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

#include <time.h>

#ifdef SHARED

#include <dl-vdso.h>

void *time_ifunc (void) __asm__ ("time");

void *
time_ifunc (void)
{
  PREPARE_VERSION_KNOWN (linux26, LINUX_2_6);

  return _dl_vdso_vsym ("__vdso_time", &linux26) ?: TIME_FALLBACK;
}
asm (".type time, %gnu_indirect_function");

libc_ifunc_hidden_def(time)

#else

# include <sysdep.h>

time_t
time (time_t *t)
{
  INTERNAL_SYSCALL_DECL (err);
  return INTERNAL_SYSCALL (time, err, 1, t);
}

#endif
