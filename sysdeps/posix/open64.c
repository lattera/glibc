/* Copyright (C) 1991,1995-1997,1999,2000,2002 Free Software Foundation, Inc.
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

#include <fcntl.h>
#include <stdarg.h>
#include <bp-sym.h>
#include <sysdep-cancel.h>

/* Open FILE with access OFLAG.  If OFLAG includes O_CREAT,
   a third argument is the file protection.  */
int
__libc_open64 (const char *file, int oflag, ...)
{
  int mode = 0;

  if (oflag & O_CREAT)
    {
      va_list arg;
      va_start (arg, oflag);
      mode = va_arg (arg, int);
      va_end (arg);
    }

  if (SINGLE_THREAD_P)
    return __libc_open (file, oflag | O_LARGEFILE, mode);

  int oldtype = LIBC_CANCEL_ASYNC ();

  int result = __libc_open (file, oflag | O_LARGEFILE, mode);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}
weak_alias (__libc_open64, BP_SYM (__open64))
libc_hidden_weak (BP_SYM (__open64))
weak_alias (__libc_open64, BP_SYM (open64))
