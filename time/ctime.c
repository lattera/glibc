/* Copyright (C) 1991, 1996 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#undef	__OPTIMIZE__	/* Avoid inline `ctime' function.  */
#include <time.h>

#undef	ctime


/* Return a string as returned by asctime which
   is the representation of *T in that form.  */
char *
ctime (const time_t *t)
{
  static char buf[64];		/* POSIX.1 suggests at least 26 bytes.  */
  struct tm tm;
  struct tm *tp = __localtime_r (t, &tm);
  if (tp == NULL)
    return NULL;
  return __asctime_r (tp, buf);
}
