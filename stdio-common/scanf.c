/* Copyright (C) 1991, 1995, 1996, 1997 Free Software Foundation, Inc.
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
<   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <stdarg.h>
#include <stdio.h>

/* The function `vscanf' is not defined in ISO C.  Therefore we must
   use the protected form here.  In stdio it is called `__vscanf' and
   in libio `_IO_vscanf'.  */
#ifdef USE_IN_LIBIO
# include <libioP.h>
# define VSCANF _IO_vscanf
#else
# define VSCANF __vscanf
#endif

/* Read formatted input from stdin according to the format string FORMAT.  */
/* VARARGS1 */
int
scanf (const char *format, ...)
{
  va_list arg;
  int done;

  va_start (arg, format);
  done = VSCANF (format, arg);
  va_end (arg);

  return done;
}
