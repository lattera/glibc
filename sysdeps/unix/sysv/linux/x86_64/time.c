/* Copyright (C) 2001,02,2003,2011 Free Software Foundation, Inc.
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
/* Redefine time so that the compiler won't complain about the type
   mismatch with the IFUNC selector in strong_alias, below.  */
#undef time
#define time __redirect_time
#include <time.h>

#include <dl-vdso.h>

#define VSYSCALL_ADDR_vtime	0xffffffffff600400

/* Avoid DWARF definition DIE on ifunc symbol so that GDB can handle
   ifunc symbol properly.  */
extern __typeof (__redirect_time) __libc_time;
void *time_ifunc (void) __asm__ ("__libc_time");

void *
time_ifunc (void)
{
  PREPARE_VERSION (linux26, "LINUX_2.6", 61765110);

  /* If the vDSO is not available we fall back on the old vsyscall.  */
  return _dl_vdso_vsym ("__vdso_time", &linux26) ?: (void *) VSYSCALL_ADDR_vtime;
}
__asm (".type __libc_time, %gnu_indirect_function");

#undef time
strong_alias (__libc_time, time)
libc_hidden_ver (__libc_time, time)

#else

# include <time.h>
# include <sysdep.h>

time_t
time (time_t *t)
{
  INTERNAL_SYSCALL_DECL (err);
  return INTERNAL_SYSCALL (time, err, 1, t);
}

#endif
