/* time system call for Linux/PowerPC.
   Copyright (C) 2013 Free Software Foundation, Inc.
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
# include <bits/libc-vdso.h>
# include <dl-machine.h>

void *time_ifunc (void) asm ("time");

static time_t
time_syscall (time_t *t)
{
  struct timeval tv;
  time_t result;

  if (INLINE_VSYSCALL (gettimeofday, 2, &tv, NULL) < 0)
    result = (time_t) -1;
  else
    result = (time_t) tv.tv_sec;

  if (t != NULL)
    *t = result;
  return result;
}

void *
time_ifunc (void)
{
  /* If the vDSO is not available we fall back to the syscall.  */
  return (__vdso_time ? VDSO_IFUNC_RET (__vdso_time)
	  : time_syscall);
}
asm (".type time, %gnu_indirect_function");

/* This is doing "libc_hidden_def (time)" but the compiler won't
 * let us do it in C because it doesn't know we're defining time
 * here in this file.  */
asm (".globl __GI_time\n"
     "__GI_time = time");

#else

#include <sysdeps/posix/time.c>

#endif
