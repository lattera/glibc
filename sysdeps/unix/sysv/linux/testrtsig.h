/* Test whether RT signals are really available.
   Copyright (C) 1997, 1999, 2006 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <string.h>
#include <sys/utsname.h>

#include <kernel-features.h>

static int
kernel_has_rtsig (void)
{
#if __ASSUME_REALTIME_SIGNALS
  return 1;
#else
  struct utsname name;

  return uname (&name) == 0 && __strverscmp (name.release, "2.1.70") >= 0;
#endif
}
