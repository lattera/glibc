/* Read or write system information.  Linux version.
   Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <sys/sysctl.h>

#include <sysdep.h>
#include <sys/syscall.h>

extern int __syscall__sysctl (struct __sysctl_args *args);

int
sysctl (int *name, int nlen, void *oldval, size_t *oldlenp,
	void *newval, size_t newlen)
{
  struct __sysctl_args args =
  {
    name: name,
    nlen: nlen,
    oldval: oldval,
    oldlenp: oldlenp,
    newval: newval,
    newlen: newlen
  };

  return INLINE_SYSCALL (_sysctl, 1, &args);
}
