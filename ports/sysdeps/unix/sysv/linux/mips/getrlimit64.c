/* Copyright (C) 2013 Free Software Foundation, Inc.
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

#include <sys/resource.h>

#if _MIPS_SIM == _ABIO32 || _MIPS_SIM == _ABIN32

# define getrlimit64 static __internal_getrlimit64
# undef libc_hidden_def
# define libc_hidden_def(name)
# include <sysdeps/unix/sysv/linux/getrlimit64.c>
# undef getrlimit64
# undef libc_hidden_def
# define libc_hidden_def(name) hidden_def (name)

/* RLIM64_INFINITY was supposed to be a glibc convention rather than
   anything seen by the kernel, but it ended being passed to the kernel
   through the prlimit64 syscall.  Given that a lot of binaries with
   the wrong constant value are in the wild, provide a wrapper function
   fixing the value after the syscall.  */

# define GLIBC_RLIM64_INFINITY		0x7fffffffffffffffULL
# define KERNEL_RLIM64_INFINITY		0xffffffffffffffffULL

int
getrlimit64 (enum __rlimit_resource resource,
	     struct rlimit64 *rlimits)
{
  struct rlimit64 krlimits;

  if (__internal_getrlimit64 (resource, &krlimits) < 0)
    return -1;

  if (krlimits.rlim_cur == KERNEL_RLIM64_INFINITY)
    rlimits->rlim_cur = GLIBC_RLIM64_INFINITY;
  else
    rlimits->rlim_cur = krlimits.rlim_cur;
  if (krlimits.rlim_max == KERNEL_RLIM64_INFINITY)
    rlimits->rlim_max = GLIBC_RLIM64_INFINITY;
  else
    rlimits->rlim_max = krlimits.rlim_max;

  return 0;
}

libc_hidden_def (getrlimit64)

#else /* !_ABI_O32 && !_ABI_N32 */
# include <sysdeps/unix/sysv/linux/getrlimit64.c>
#endif
