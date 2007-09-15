/* Copyright (C) 2004, 2005, 2007 Free Software Foundation, Inc.
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
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <sysdep.h>

#ifdef __NR_mq_open

/* Establish connection between a process and a message queue NAME and
   return message queue descriptor or (mqd_t) -1 on error.  OFLAG determines
   the type of access used.  If O_CREAT is on OFLAG, the third argument is
   taken as a `mode_t', the mode of the created message queue, and the fourth
   argument is taken as `struct mq_attr *', pointer to message queue
   attributes.  If the fourth argument is NULL, default attributes are
   used.  */
mqd_t
__mq_open (const char *name, int oflag, ...)
{
  if (name[0] != '/')
    {
      __set_errno (EINVAL);
      return -1;
    }

  mode_t mode = 0;
  struct mq_attr *attr = NULL;
  if (oflag & O_CREAT)
    {
      va_list ap;

      va_start (ap, oflag);
      mode = va_arg (ap, mode_t);
      attr = va_arg (ap, struct mq_attr *);
      va_end (ap);
    }

  return INLINE_SYSCALL (mq_open, 4, name + 1, oflag, mode, attr);
}
strong_alias (__mq_open, mq_open);

mqd_t
__mq_open_2 (const char *name, int oflag)
{
  if (oflag & O_CREAT)
    __fortify_fail ("invalid mq_open call: O_CREAT without mode and attr");

  return __mq_open (name, oflag);
}
#else
# include <rt/mq_open.c>
#endif
