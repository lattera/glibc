/* Copyright (C) 2001, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2001

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
#include <string.h>
#include <unistd.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

int
__gethostname (char *name, size_t len)
{
  int result;

  result = INLINE_SYSCALL (gethostname, 2, CHECK_N (name, len), len);

  if (result == 0
      /* See whether the string is terminated.  If not we will return
	 an error.  */
      && memchr (name, '\0', len) == NULL)
    {
      __set_errno (EOVERFLOW);
      result = -1;
    }

  return result;
}

weak_alias (__gethostname, gethostname)
