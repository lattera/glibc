/* Copyright (C) 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, 1996.

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

#include <errno.h>
#include <wchar.h>

#ifndef EILSEQ
#define EILSEQ EINVAL
#endif

static const wchar_t encoding_mask[] =
{
  ~0x7ff, ~0xffff, ~0x1fffff, ~0x3ffffff
};

static const unsigned char encoding_byte[] =
{
  0xc0, 0xe0, 0xf0, 0xf8, 0xfc
};

/* The state is for this UTF8 encoding not used.  */
static mbstate_t internal;

size_t
__wcrtomb (char *s, wchar_t wc, mbstate_t *ps)
{
  char fake[1];
  size_t written = 0;

  if (ps == NULL)
    ps = &internal;

  if (s == NULL)
    {
      s = fake;
      wc = L'\0';
    }

  /* Store the UTF8 representation of WC.  */
  if (wc < 0 || wc > 0x7fffffff)
    {
      /* This is no correct ISO 10646 character.  */
      __set_errno (EILSEQ);
      return (size_t) -1;
    }

  if (wc < 0x80)
    {
      /* It's a one byte sequence.  */
      if (s != NULL)
	*s = (char) wc;
      return 1;
    }

  for (written = 2; written < 6; ++written)
    if ((wc & encoding_mask[written - 2]) == 0)
      break;

  if (s != NULL)
    {
      size_t cnt = written;
      s[0] = encoding_byte[cnt - 2];

      --cnt;
      do
	{
	  s[cnt] = 0x80 | (wc & 0x3f);
	  wc >>= 6;
	}
      while (--cnt > 0);
      s[0] |= wc;
    }

  return written;
}
weak_alias (__wcrtomb, wcrtomb)
