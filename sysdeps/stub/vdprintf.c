/* Copyright (C) 1997 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */
 
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>

/* Write formatted output to file descriptor D according to the format string
   FORMAT, using the argument list in ARG.  */
int
vdprintf (int d, const char *format, va_list arg)
{
  __set_errno (ENOSYS);
  return 0;
}

stub_warning (vdprintf)
