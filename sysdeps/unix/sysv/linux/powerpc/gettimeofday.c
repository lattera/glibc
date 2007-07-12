/* Copyright (C) 2005 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>
#include <bp-checks.h>
#include <stddef.h>
#include <sys/time.h>
#include <time.h>
#include <hp-timing.h>

#undef __gettimeofday
#include <bits/libc-vdso.h>

/* Get the current time of day and timezone information,
   putting it into *TV and *TZ.  If TZ is NULL, *TZ is not filled.
   Returns 0 on success, -1 on errors.  */

int
__gettimeofday (tv, tz)
     struct timeval *tv;
     struct timezone *tz;
{
  return INLINE_VSYSCALL (gettimeofday, 2, CHECK_1 (tv), CHECK_1 (tz));
}

INTDEF (__gettimeofday)
weak_alias (__gettimeofday, gettimeofday)
