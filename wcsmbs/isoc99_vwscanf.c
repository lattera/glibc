/* Copyright (C) 1991, 1997, 2006, 2007 Free Software Foundation, Inc.
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

#include <libioP.h>
#include <stdio.h>
#include <wchar.h>

/* Read formatted input from STDIN according to the format string FORMAT.  */
/* VARARGS2 */
int
__isoc99_vwscanf (const wchar_t *format, _IO_va_list args)
{
  int done;

  _IO_acquire_lock_clear_flags2 (stdin);
  stdin->_flags2 |= _IO_FLAGS2_SCANF_STD;
  done = _IO_vfwscanf (stdin, format, args, NULL);
  _IO_release_lock (stdin);
  return done;
}
