/* Copyright (C) 1991, 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <errno.h>
#include <sys/time.h>
#include <hurd.h>

/* Adjust the current time of day by the amount in DELTA.
   If OLDDELTA is not NULL, it is filled in with the amount
   of time adjustment remaining to be done from the last `__adjtime' call.
   This call is restricted to the super-user.  */
int
DEFUN(__adjtime, (delta, olddelta),
      CONST struct timeval *delta AND
      struct timeval *olddelta)
{
  error_t err;
  mach_port_t hostpriv;

  hostpriv = __pid2task (-1);
  if (hostpriv == MACH_PORT_NULL)
    return -1;
  err = __host_adjust_time (hostpriv, delta, olddelta);
  __mach_port_deallocate (__mach_task_self (), hostpriv);
  if (err)
    return __hurd_fail (err);
  return 0;
}

weak_alias (__adjtime, adjtime)
