/* Copyright (C) 1998 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <sys/syscall.h>

/*
  In Linux 2.1.x the chown functions have been changed.  A new function lchown
  was introduced.  The new chown now follows symlinks - the old chown and the
  new lchown do not follow symlinks.
  The new lchown function has the same number as the old chown had and the
  new chown has a new number.  When compiling with headers from Linux > 2.1.8x
  it's impossible to run this libc with older kernels.  In these cases libc
  has therefore to route calls to chown to the old chown function.
*/

extern int __syscall_chown (const char *__file,
			    uid_t __owner, gid_t __group);
#ifdef __NR_lchown
/* running under Linux 2.0 or < 2.1.8x */
static int __libc_old_chown;


int
__chown (const char *file, uid_t owner, gid_t group)
{
  int result;

  if (!__libc_old_chown)
    {
      int saved_errno = errno;
      result = __syscall_chown (file, owner, group);

      if (result >= 0 || errno != ENOSYS)
	return result;

      __set_errno (saved_errno);
      __libc_old_chown = 1;
    }

  return __lchown (file, owner, group);
}
#else
/* compiling under older kernels */
int
__chown (const char *file, uid_t owner, gid_t group)
{
  return __syscall_chown (file, owner, group);
}
#endif

weak_alias (__chown, chown)
