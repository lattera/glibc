/* ABI compatibility redirects for clock_* symbols in librt.
   Copyright (C) 2012 Free Software Foundation, Inc.
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

#include <shlib-compat.h>

/* The clock_* symbols were originally defined in librt and so
   are part of its ABI.  As of 2.17, they have moved to libc.
   So we supply definitions for librt that just redirect to
   their libc counterparts.  */

#if SHLIB_COMPAT (librt, GLIBC_2_2, GLIBC_2_17)

#include <time.h>

#ifdef HAVE_IFUNC
# define COMPAT_REDIRECT(name, proto, arglist)				      \
  __typeof (name) *name##_ifunc (void) asm (#name);			      \
  __typeof (name) *name##_ifunc (void)					      \
  {									      \
    return &__##name;							      \
  }									      \
  asm (".type " #name ", %gnu_indirect_function");
#else
# define COMPAT_REDIRECT(name, proto, arglist)				      \
  int									      \
  name proto								      \
  {									      \
    return __##name arglist;						      \
  }
#endif

COMPAT_REDIRECT (clock_getres,
                 (clockid_t clock_id, struct timespec *res),
                 (clock_id, res))
COMPAT_REDIRECT (clock_gettime,
                 (clockid_t clock_id, struct timespec *tp),
                 (clock_id, tp))
COMPAT_REDIRECT (clock_settime,
                 (clockid_t clock_id, const struct timespec *tp),
                 (clock_id, tp))
COMPAT_REDIRECT (clock_getcpuclockid,
                 (pid_t pid, clockid_t *clock_id),
                 (pid, clock_id))
COMPAT_REDIRECT (clock_nanosleep,
                 (clockid_t clock_id, int flags,
                  const struct timespec *req,
                  struct timespec *rem),
                 (clock_id, flags, req, rem))

#endif
