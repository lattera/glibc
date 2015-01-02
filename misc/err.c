/* 4.4BSD utility functions for error messages.
   Copyright (C) 1995-2015 Free Software Foundation, Inc.
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

#include <stdarg.h>
#include <err.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <wchar.h>
#define flockfile(s) _IO_flockfile (s)
#define funlockfile(s) _IO_funlockfile (s)

extern char *__progname;

#define VA(call)							      \
{									      \
  va_list ap;								      \
  va_start (ap, format);						      \
  call;									      \
  va_end (ap);								      \
}

static void
convert_and_print (const char *format, __gnuc_va_list ap)
{
#define ALLOCA_LIMIT	2000
  size_t len;
  wchar_t *wformat = NULL;
  mbstate_t st;
  size_t res;
  const char *tmp;

  if (format == NULL)
    return;

  len = strlen (format) + 1;

  do
    {
      if (len < ALLOCA_LIMIT)
	wformat = (wchar_t *) alloca (len * sizeof (wchar_t));
      else
	{
	  if (wformat != NULL && len / 2 < ALLOCA_LIMIT)
	    wformat = NULL;

	  wformat = (wchar_t *) realloc (wformat, len * sizeof (wchar_t));

	  if (wformat == NULL)
	    {
	      fputws_unlocked (L"out of memory\n", stderr);
	      return;
	    }
	}

      memset (&st, '\0', sizeof (st));
      tmp =format;
    }
  while ((res = __mbsrtowcs (wformat, &tmp, len, &st)) == len);

  if (res == (size_t) -1)
    /* The string cannot be converted.  */
    wformat = (wchar_t *) L"???";

  __vfwprintf (stderr, wformat, ap);
}

void
vwarnx (const char *format, __gnuc_va_list ap)
{
  flockfile (stderr);
  if (_IO_fwide (stderr, 0) > 0)
    {
      __fwprintf (stderr, L"%s: ", __progname);
      convert_and_print (format, ap);
      putwc_unlocked (L'\n', stderr);
    }
  else
    {
      fprintf (stderr, "%s: ", __progname);
      if (format)
	vfprintf (stderr, format, ap);
      putc_unlocked ('\n', stderr);
    }
  funlockfile (stderr);
}
libc_hidden_def (vwarnx)

void
vwarn (const char *format, __gnuc_va_list ap)
{
  int error = errno;

  flockfile (stderr);
  if (_IO_fwide (stderr, 0) > 0)
    {
      __fwprintf (stderr, L"%s: ", __progname);
      if (format)
	{
	  convert_and_print (format, ap);
	  fputws_unlocked (L": ", stderr);
	}
      __set_errno (error);
      __fwprintf (stderr, L"%m\n");
    }
  else
    {
      fprintf (stderr, "%s: ", __progname);
      if (format)
	{
	  vfprintf (stderr, format, ap);
	  fputs_unlocked (": ", stderr);
	}
      __set_errno (error);
      fprintf (stderr, "%m\n");
    }
  funlockfile (stderr);
}
libc_hidden_def (vwarn)


void
warn (const char *format, ...)
{
  VA (vwarn (format, ap))
}
libc_hidden_def (warn)

void
warnx (const char *format, ...)
{
  VA (vwarnx (format, ap))
}
libc_hidden_def (warnx)

void
verr (int status, const char *format, __gnuc_va_list ap)
{
  vwarn (format, ap);
  exit (status);
}
libc_hidden_def (verr)

void
verrx (int status, const char *format, __gnuc_va_list ap)
{
  vwarnx (format, ap);
  exit (status);
}
libc_hidden_def (verrx)

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
