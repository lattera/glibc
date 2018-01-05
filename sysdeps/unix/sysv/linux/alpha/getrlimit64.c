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

/* Add this redirection so the strong_alias linking getrlimit64 to
   {__}getrlimit does not throw a type error.  */
#undef getrlimit
#undef __getrlimit
#define getrlimit getrlimit_redirect
#define __getrlimit __getrlimit_redirect
#include <sys/resource.h>
#undef getrlimit
#undef __getrlimit

/* RLIM64_INFINITY was supposed to be a glibc convention rather than
   anything seen by the kernel, but it ended being passed to the kernel
   through the prlimit64 syscall.  Given that a lot of binaries with
   the wrong constant value are in the wild, provide a wrapper function
   fixing the value after the syscall.  */
#define KERNEL_RLIM64_INFINITY		0xffffffffffffffffULL

int
__getrlimit64 (enum __rlimit_resource resource, struct rlimit64 *rlimits)
{
  struct rlimit64 krlimits;

  if (INLINE_SYSCALL_CALL (prlimit64, 0, resource, NULL, &krlimits) < 0)
    return -1;

  if (krlimits.rlim_cur == KERNEL_RLIM64_INFINITY)
    rlimits->rlim_cur = RLIM64_INFINITY;
  else
    rlimits->rlim_cur = krlimits.rlim_cur;
  if (krlimits.rlim_max == KERNEL_RLIM64_INFINITY)
    rlimits->rlim_max = RLIM64_INFINITY;
  else
    rlimits->rlim_max = krlimits.rlim_max;

  return 0;
}
libc_hidden_def (__getrlimit64)
strong_alias (__getrlimit64, __GI_getrlimit)
strong_alias (__getrlimit64, __GI___getrlimit)
strong_alias (__getrlimit64, __getrlimit)
weak_alias (__getrlimit64, getrlimit)

weak_alias (__getrlimit64, getrlimit64)
libc_hidden_weak (getrlimit64)
