/* Open a file by name.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <nacl-interfaces.h>


/* Open FILE with access OFLAG.  If OFLAG includes O_CREAT,
   a third argument is the file protection.  */
int
__libc_open (const char *file, int oflag, ...)
{
  mode_t mode = 0;

  if (oflag & O_CREAT)
    {
      va_list arg;
      va_start (arg, oflag);
      mode = va_arg (arg, mode_t);
      va_end (arg);
    }

  int fd;
  return NACL_CALL (__nacl_irt_dev_filename.open (file, oflag, mode, &fd), fd);
}
libc_hidden_def (__libc_open)
weak_alias (__libc_open, __open)
libc_hidden_weak (__open)
weak_alias (__libc_open, open)

/* open64 is just an alias.  */
strong_alias (__libc_open, __libc_open64)
strong_alias (__libc_open64, __open64)
libc_hidden_def (__open64)
weak_alias (__libc_open64, open64)
