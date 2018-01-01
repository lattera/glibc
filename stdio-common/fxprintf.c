/* Copyright (C) 2005-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.org>.

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <libioP.h>

static int
locked_vfxprintf (FILE *fp, const char *fmt, va_list ap)
{
  if (_IO_fwide (fp, 0) <= 0)
    return _IO_vfprintf (fp, fmt, ap);

  /* We must convert the narrow format string to a wide one.
     Each byte can produce at most one wide character.  */
  wchar_t *wfmt;
  mbstate_t mbstate;
  int res;
  int used_malloc = 0;
  size_t len = strlen (fmt) + 1;

  if (__glibc_unlikely (len > SIZE_MAX / sizeof (wchar_t)))
    {
      __set_errno (EOVERFLOW);
      return -1;
    }
  if (__libc_use_alloca (len * sizeof (wchar_t)))
    wfmt = alloca (len * sizeof (wchar_t));
  else if ((wfmt = malloc (len * sizeof (wchar_t))) == NULL)
    return -1;
  else
    used_malloc = 1;

  memset (&mbstate, 0, sizeof mbstate);
  res = __mbsrtowcs (wfmt, &fmt, len, &mbstate);

  if (res != -1)
    res = _IO_vfwprintf (fp, wfmt, ap);

  if (used_malloc)
    free (wfmt);

  return res;
}

int
__fxprintf (FILE *fp, const char *fmt, ...)
{
  if (fp == NULL)
    fp = stderr;

  va_list ap;
  va_start (ap, fmt);
  _IO_flockfile (fp);

  int res = locked_vfxprintf (fp, fmt, ap);

  _IO_funlockfile (fp);
  va_end (ap);
  return res;
}

int
__fxprintf_nocancel (FILE *fp, const char *fmt, ...)
{
  if (fp == NULL)
    fp = stderr;

  va_list ap;
  va_start (ap, fmt);
  _IO_flockfile (fp);
  int save_flags2 = ((_IO_FILE *)fp)->_flags2;
  ((_IO_FILE *)fp)->_flags2 |= _IO_FLAGS2_NOTCANCEL;

  int res = locked_vfxprintf (fp, fmt, ap);

  ((_IO_FILE *)fp)->_flags2 = save_flags2;
  _IO_funlockfile (fp);
  va_end (ap);
  return res;
}
