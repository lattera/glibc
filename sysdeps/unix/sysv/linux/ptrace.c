/* Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>

int
ptrace (int request, int pid, int addr, int data)
{
  long int ret;
  long int res;
  if (request > 0 && request < 4)
    (long int *) data = &ret;

  res = __syscall_ptrace (request, pid, addr, data);

  if (res >= 0)
    {
      if (request > 0 && request < 4)
	{
	  errno = 0;
	  return (ret);
	}
      return (int) res;
    }

  errno = -res;
  return -1;
}
