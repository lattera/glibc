/* Access functions for JISX0212 conversion.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#ifndef _JIS0212_H
#define _JIS0212_H	1

#include <gconv.h>
#include <stdint.h>


/* Struct for table with indeces in mapping table.  */
struct jisx0212_idx
{
  uint16_t start;
  uint16_t end;
  uint16_t idx;
};

/* Conversion table.  */
extern const struct jisx0212_idx jisx0212_to_ucs_idx[];
extern const uint16_t jisx0212_to_ucs[];

extern const struct jisx0212_idx jisx0212_from_ucs_idx[];
extern const char jisx0212_from_ucs[][2];


static inline wchar_t
jisx0212_to_ucs4 (const char **s, size_t avail, unsigned char offset)
{
  const struct jisx0212_idx *rp = jisx0212_to_ucs_idx;
  unsigned char ch = *(*s);
  unsigned char ch2;
  wchar_t wch = L'\0';
  int idx;

  if (ch < offset || (ch - offset) <= 0x6d || (ch - offset) > 0xea)
    return UNKNOWN_10646_CHAR;

  if (avail < 2)
    return 0;

  ch2 = (*s)[1];
  if (ch2 < offset || (ch2 - offset) <= 0x20 || (ch2 - offset) >= 0x7f)
    return UNKNOWN_10646_CHAR;

  idx = (ch - 0x21 - offset) * 94 + (ch2 - 0x21 - offset);

  while (idx < rp->start)
    ++rp;
  if (idx <= rp->end)
    wch = jisx0212_to_ucs[rp->idx + idx - rp->start];

  if (wch != L'\0')
    (*s) += 2;
  else
    wch = UNKNOWN_10646_CHAR;

  return wch;
}


static inline size_t
ucs4_to_jisx0212 (wchar_t wch, char *s, size_t avail)
{
  const struct jisx0212_idx *rp = jisx0212_from_ucs_idx;
  unsigned int ch = (unsigned int) wch;
  const char *cp = NULL;

  while (ch > rp->end)
    ++rp;
  if (ch >= rp->start)
    cp = jisx0212_from_ucs[rp->idx + ch - rp->start];

  if (cp == NULL || cp[0] == '\0')
    return UNKNOWN_10646_CHAR;

  s[0] = cp[0];
  if (cp[1] != '\0')
    {
      if (avail < 2)
	return 0;

      s[1] = cp[1];
    }

  return 2;
}

#endif /* jis0212.h */
