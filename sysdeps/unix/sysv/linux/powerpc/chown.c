/* chown() compatibility.
   Copyright (C) 1998-2012 Free Software Foundation, Inc.
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

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sysdep.h>
#include <stdlib.h>

#include <kernel-features.h>

/* Consider moving to syscalls.list.  */

/*
  In Linux 2.1.x the chown functions have been changed.  A new function lchown
  was introduced.  The new chown now follows symlinks - the old chown and the
  new lchown do not follow symlinks.
  This file emulates chown() under the old kernels.
*/

int
__chown (const char *file, uid_t owner, gid_t group)
{
  return INLINE_SYSCALL (chown, 3, file, owner, group);
}
libc_hidden_def (__chown)

#include <shlib-compat.h>
versioned_symbol (libc, __chown, chown, GLIBC_2_1);
