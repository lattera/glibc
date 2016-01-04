/* Define and initialize the `__libc_enable_secure' flag.  Generic version.
   Copyright (C) 1996-2016 Free Software Foundation, Inc.
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

/* This file is used in the static libc.  For the shared library,
   dl-sysdep.c defines and initializes __libc_enable_secure.  */

#include <unistd.h>
#include <libc-internal.h>

/* If nonzero __libc_enable_secure is already set.  */
int __libc_enable_secure_decided;
/* Safest assumption, if somehow the initializer isn't run.  */
int __libc_enable_secure = 1;

void
__libc_init_secure (void)
{
  if (__libc_enable_secure_decided == 0)
    __libc_enable_secure = (__geteuid () != __getuid ()
			    || __getegid () != __getgid ());
}
