/* Copyright (C) 1991,1995,1997,1998,1999,2000,2003,2005
	Free Software Foundation, Inc.
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
#include <wchar.h>

/* Write formatted output into S, according to the format string FORMAT.  */
/* VARARGS5 */
int
__swprintf_chk (wchar_t *s, size_t n, int flag, size_t s_len,
		const wchar_t *format, ...)
{
  va_list arg;
  int done;

  va_start (arg, format);
  done = __vswprintf_chk (s, n, flag, s_len, format, arg);
  va_end (arg);

  return done;
}
