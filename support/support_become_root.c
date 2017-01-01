/* Acquire root privileges.
   Copyright (C) 2016-2017 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <support/namespace.h>

#include <sched.h>
#include <stdio.h>
#include <unistd.h>

bool
support_become_root (void)
{
#ifdef CLONE_NEWUSER
  if (unshare (CLONE_NEWUSER | CLONE_NEWNS) == 0)
    /* Even if we do not have UID zero, we have extended privileges at
       this point.  */
    return true;
#endif
  if (setuid (0) != 0)
    {
      printf ("warning: could not become root outside namespace (%m)\n");
      return false;
    }
  return true;
}
