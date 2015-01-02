/* Copyright (C) 2005-2015 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <sys/param.h>
#ifdef HAVE_INLINED_SYSCALLS
# include <errno.h>
# include <sysdep.h>
#endif


ssize_t
__readlink_chk (const char *path, void *buf, size_t len, size_t buflen)
{
  if (len > buflen)
    __chk_fail ();

#ifdef HAVE_INLINED_SYSCALLS
  return INLINE_SYSCALL (readlink, 3, path, buf, len);
#else
  return __readlink (path, buf, len);
#endif
}
