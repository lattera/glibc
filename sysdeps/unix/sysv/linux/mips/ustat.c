/* Copyright (C) 1997-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sys/ustat.h>
#include <sys/sysmacros.h>

#include <sysdep.h>
#include <sys/syscall.h>

int
ustat (dev_t dev, struct ustat *ubuf)
{
  unsigned long k_dev;

  /* We must convert the value to dev_t type used by the kernel.  */
  k_dev = ((major (dev) & 0xff) << 8) | (minor (dev) & 0xff);

  return INLINE_SYSCALL (ustat, 2, k_dev, ubuf);
}
