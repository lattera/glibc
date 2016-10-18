/* Linux setrlimit64 implementation (64 bits off_t).
   Copyright (C) 2010-2016 Free Software Foundation, Inc.
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
#include <shlib-compat.h>

/* Add this redirection so the strong_alias for __RLIM_T_MATCHES_RLIM64_T
   linking setlimit64 to {__}setrlimit does not throw a type error.  */
#undef settrlimit
#undef __sttrlimit
#define setrlimit setrlimit_redirect
#define __setrlimit __setrlimit_redirect
#include <sys/resource.h>
#undef setrlimit
#undef __setrlimit

/* Set the soft and hard limits for RESOURCE to *RLIMITS.
   Only the super-user can increase hard limits.
   Return 0 if successful, -1 if not (and sets errno).  */
int
__setrlimit64 (enum __rlimit_resource resource, const struct rlimit64 *rlimits)
{
  int res;

#ifdef __NR_prlimit64
  res = INLINE_SYSCALL_CALL (prlimit64, 0, resource, rlimits, NULL);
  if (res == 0 || errno != ENOSYS)
    return res;
#endif

/* The fallback code only makes sense if the platform supports
   __NR_setrlimit.  */
#ifdef __NR_setrlimit
# if !__RLIM_T_MATCHES_RLIM64_T
  struct rlimit rlimits32;

  if (rlimits->rlim_cur >= RLIM_INFINITY)
    rlimits32.rlim_cur = RLIM_INFINITY;
  else
    rlimits32.rlim_cur = rlimits->rlim_cur;
  if (rlimits->rlim_max >= RLIM_INFINITY)
    rlimits32.rlim_max = RLIM_INFINITY;
  else
    rlimits32.rlim_max = rlimits->rlim_max;
# else
#  define rlimits32 (*rlimits)
# endif

  res = INLINE_SYSCALL_CALL (setrlimit, resource, &rlimits32);
#endif

  return res;
}
weak_alias (__setrlimit64, setrlimit64)

#if __RLIM_T_MATCHES_RLIM64_T
strong_alias (__setrlimit64, __setrlimit)
weak_alias (__setrlimit64, setrlimit)
#endif
