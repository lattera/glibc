/* err.c --- 4.4BSD utility functions for error messages.
Copyright (C) 1995 Free Software Foundation, Inc.
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

#include <stdarg.h>
#include <err.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

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
  if (__progname)
    fprintf (stderr, "%s: ", __progname);
  if (format)
    vfprintf (stderr, format, ap);
  putc ('\n', stderr);
}

void
vwarn (const char *format, __gnuc_va_list ap)
{
  int error = errno;

  if (__progname)
    fprintf (stderr, "%s: ", __progname);
  if (format)
    {
      vfprintf (stderr, format, ap);
      fputs (": ", stderr);
    }
  fprintf (stderr, "%s\n", strerror (error));
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
