/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

#include <stdarg.h>
#include <sys/ioctl.h>

extern int kioctl (int fdes, int cmd, unsigned long int arg,
		   unsigned long int ext);

int
__ioctl (int fdes, unsigned long int cmd, ...)
{
  va_list va;
  int res;
  unsigned long int arg;
  unsigned long int ext;

  va_start (va, cmd);
  arg = va_arg (va, unsigned long int);
  ext = va_arg (va, unsigned long int);

  res = kioctl (fdes, cmd, arg, ext);

  va_end (va);

  return res;
}
strong_alias (__ioctl, ioctl)
