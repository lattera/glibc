/* Copyright (C) 1997, 1998, 2000, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <errno.h>
#include <sys/ustat.h>
#include <sys/sysmacros.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

int
ustat (dev_t dev, struct ustat *ubuf)
{
  unsigned long long int k_dev;

  /* We must convert the value to dev_t type used by the kernel.  */
  k_dev =  dev & ((1ULL << 32) - 1);
  if (k_dev != dev)
    {
      __set_errno (EINVAL);
      return -1;
    }

  return INLINE_SYSCALL (ustat, 2, (unsigned int) k_dev, CHECK_1 (ubuf));
}
