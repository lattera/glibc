/* Copyright (C) 2018 Free Software Foundation, Inc.
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

#include <errno.h>
#include <sys/types.h>

/* Add this redirection so the strong_alias linking setrlimit64 to
   {__}setrlimit does not throw a type error.  */
#undef setrlimit
#undef __setrlimit
#define setrlimit setrlimit_redirect
#define __setrlimit __setrlimit_redirect
#include <sys/resource.h>
#undef setrlimit
#undef __setrlimit

/* RLIM64_INFINITY was supposed to be a glibc convention rather than
   anything seen by the kernel, but it ended being passed to the kernel
   through the prlimit64 syscall.  Given that a lot of binaries with
   the wrong constant value are in the wild, provide a wrapper function
   fixing the value before the syscall.  */
#define KERNEL_RLIM64_INFINITY		0xffffffffffffffffULL

int
__setrlimit64 (enum __rlimit_resource resource, const struct rlimit64 *rlimits)
{
  struct rlimit64 krlimits;

  if (rlimits->rlim_cur == RLIM64_INFINITY)
    krlimits.rlim_cur = KERNEL_RLIM64_INFINITY;
  else
    krlimits.rlim_cur = rlimits->rlim_cur;
  if (rlimits->rlim_max == RLIM64_INFINITY)
    krlimits.rlim_max = KERNEL_RLIM64_INFINITY;
  else
    krlimits.rlim_max = rlimits->rlim_max;

  return INLINE_SYSCALL_CALL (prlimit64, 0, resource, &krlimits, NULL);
}

weak_alias (__setrlimit64, setrlimit64)

strong_alias (__setrlimit64, __setrlimit)
weak_alias (__setrlimit64, setrlimit)
#ifdef SHARED
__hidden_ver1 (__setrlimit64, __GI___setrlimit, __setrlimit64);
#endif
