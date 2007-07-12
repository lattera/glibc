/* Copyright (C) 2004, 2005 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <mqueue.h>
#include <sysdep.h>

#ifdef __NR_mq_unlink

/* Remove message queue named NAME.  */
int
mq_unlink (const char *name)
{
  if (name[0] != '/')
    {
      __set_errno (EINVAL);
      return -1;
    }

  INTERNAL_SYSCALL_DECL (err);
  int ret = INTERNAL_SYSCALL (mq_unlink, err, 1, name + 1);

  /* While unlink can return either EPERM or EACCES, mq_unlink should
     return just EACCES.  */
  if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (ret, err), 0))
    {
      ret = INTERNAL_SYSCALL_ERRNO (ret, err);
      if (ret == EPERM)
	ret = EACCES;
      __set_errno (ret);
      ret = -1;
    }

  return ret;
}

#else
# include <rt/mq_unlink.c>
#endif
