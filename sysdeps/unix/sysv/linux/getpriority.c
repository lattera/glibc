/* getpriority for Linux.
   Copyright (C) 1996, 1998 Free Software Foundation, Inc.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <sys/resource.h>

#include <sysdep.h>
#include <sys/syscall.h>

extern int __syscall_getpriority (int, int);

/* The return value of __syscall_getpriority is biased by this value
   to avoid returning negative values.  */
#define PZERO 20

/* Return the highest priority of any process specified by WHICH and WHO
   (see above); if WHO is zero, the current process, process group, or user
   (as specified by WHO) is used.  A lower priority number means higher
   priority.  Priorities range from PRIO_MIN to PRIO_MAX.  */

int
getpriority (enum __priority_which which, int who)
{
  int res;

  res = INLINE_SYSCALL (getpriority, 2, (int) which, who);
  if (res >= 0)
    res = PZERO - res;
  return res;
}
