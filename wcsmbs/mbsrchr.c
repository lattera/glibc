/* Copyright (C) 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.	 If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <mbstr.h>
#include <stdlib.h>

#define __need_wchar_t
#include <stddef.h>


/* Find the last occurence of MBC in MBS.  */
char *
mbsrchr (mbs, mbc)
    const char *mbs;
    mbchar_t mbc;
{
  const char * retval = NULL;
  int clen;
  wchar_t wc;
  wchar_t c;

  /* Reset multibyte characters to their initial state.	 */
  (void) mblen ((char *) NULL, 0);

  clen = mbtowc (&wc, (char *) &mbc, MB_CUR_MAX);
  if (clen < 0)
    /* FIXME: search character MBC is illegal.	*/
    return NULL;
  else if (clen == 0)
    wc = L'\0';

  clen = 0;
  do
    {
      mbs += clen;
      clen = mbtowc (&c, mbs, MB_CUR_MAX);
    }
  while (clen > 0 && c != wc);

  if (clen < 0)
    /* FIXME: clen < 0 means illegal character in string.  */
    return NULL;

  return (char *) (clen > 0 || (clen == 0 && wc == L'\0') ? mbs : retval);
}

