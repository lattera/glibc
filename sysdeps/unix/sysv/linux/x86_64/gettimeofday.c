/* Copyright (C) 2001 Free Software Foundation, Inc.
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

#include <asm/vsyscall.h>
#include <time.h>
#include <sys/time.h>


/* Get the current time of day and timezone information,
   putting it into *TV and *TZ.  If TZ is NULL, *TZ is not filled.
   Returns 0 on success, -1 on errors.  */
int
__gettimeofday (struct timeval *tv, struct timezone *tz)
{
  /* We're using a virtual syscall here.  */
  int (*__vgettimeofday)(struct timeval *, struct timezone *)
    = (int (*)(struct timeval *, struct timezone *)) VSYSCALL_ADDR (__NR_vgettimeofday);


  return __vgettimeofday (tv, tz);
}

weak_alias (__gettimeofday, gettimeofday)
