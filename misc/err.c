/* err.c --- 4.4BSD utility functions for error messages.
   Copyright (C) 1995, 1996, 1998 Free Software Foundation, Inc.
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

#include <stdarg.h>
#include <err.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#ifdef USE_IN_LIBIO
# define flockfile(s) _IO_flockfile (s)
# define funlockfile(s) _IO_funlockfile (s)
#endif

extern char *__progname;

#define VA(call)							      \
{									      \
  va_list ap;								      \
  va_start (ap, format);						      \
  call;									      \
  va_end (ap);								      \
}

void
vwarnx (const char *format, __gnuc_va_list ap)
{
  flockfile (stderr);
  if (__progname)
    fprintf (stderr, "%s: ", __progname);
  if (format)
    vfprintf (stderr, format, ap);
  putc_unlocked ('\n', stderr);
  funlockfile (stderr);
}

void
vwarn (const char *format, __gnuc_va_list ap)
{
  int error = errno;

  flockfile (stderr);
  if (__progname)
    fprintf (stderr, "%s: ", __progname);
  if (format)
    {
      vfprintf (stderr, format, ap);
      fputs_unlocked (": ", stderr);
    }
  __set_errno (error);
  fprintf (stderr, "%m\n");
  funlockfile (stderr);
}


void
warn (const char *format, ...)
{
  VA (vwarn (format, ap))
}

void
warnx (const char *format, ...)
{
  VA (vwarnx (format, ap))
}

void
verr (int status, const char *format, __gnuc_va_list ap)
{
  vwarn (format, ap);
  exit (status);
}

void
verrx (int status, const char *format, __gnuc_va_list ap)
{
  vwarnx (format, ap);
  exit (status);
}

void
err (int status, const char *format, ...)
{
  VA (verr (status, format, ap))
}

void
errx (int status, const char *format, ...)
{
  VA (verrx (status, format, ap))
}
