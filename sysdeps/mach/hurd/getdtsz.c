/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <hurd.h>
#include <hurd/fd.h>
#include <hurd/resource.h>

/* Return the maximum number of file descriptors the current process
   could possibly have (until it raises the resource limit).  */
int
DEFUN_VOID(__getdtablesize)
{
  int size;
  HURD_CRITICAL_BEGIN;
  __mutex_lock (&_hurd_rlimit_lock);
  size = _hurd_rlimits[RLIMIT_NOFILE].rlim_cur;
  __mutex_unlock (&_hurd_rlimit_lock);
  HURD_CRITICAL_END;
  return size;
}

weak_alias (__getdtablesize, getdtablesize)
