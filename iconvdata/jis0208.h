/* Access functions for JISX0208 conversion.
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

#ifndef _JIS0208_H
#define _JIS0208_H	1

#include <gconv.h>
#include <stdint.h>

/* Conversion table.  */
extern const uint16_t jis0208_to_ucs[];

extern const char jisx0208_from_ucs4_lat1[256][2];
extern const char jisx0208_from_ucs4_greek[0xc1][2];
extern const struct jisx0208_ucs_idx jisx0208_from_ucs_idx[];
extern const char jisx0208_from_ucs_tab[][2];


/* Struct for table with indeces in UCS mapping table.  */
struct jisx0208_ucs_idx
{
  uint16_t start;
  uint16_t end;
  uint16_t idx;
};


static inline wchar_t
jisx0208_to_ucs4 (const char **s, size_t avail, unsigned char offset)
{
  unsigned char ch = *(*s);
  unsigned char ch2;
  int idx;

  if (ch < offset || (ch - offset) <= 0x20 || (ch - offset) > 0xea)
    return UNKNOWN_10646_CHAR;

  if (avail < 2)
    return 0;

  ch2 = (*s)[1];
  if (ch2 < offset || (ch2 - offset) <= 0x20 || (ch2 - offset) >= 0x7f)
    return UNKNOWN_10646_CHAR;

  idx = (ch - 0x21 - offset) * 94 + (ch2 - 0x21 - offset);
  if (idx >= 0x1e80)
    return UNKNOWN_10646_CHAR;

  (*s) += 2;

  return jis0208_to_ucs[idx] ?: ((*s) -= 2, UNKNOWN_10646_CHAR);
}


static inline size_t
ucs4_to_jisx0208 (wchar_t wch, char *s, size_t avail)
{
  unsigned int ch = (unsigned int) wch;
  const char *cp = NULL;

  if (avail < 2)
    return 0;

  if (ch < 0x100)
    cp = jisx0208_from_ucs4_lat1[ch];
  else if (ch >= 0x391 && ch <= 0x451)
    cp = jisx0208_from_ucs4_greek[ch];
  else
    {
      const struct jisx0208_ucs_idx *rp = jisx0208_from_ucs_idx;

      while (ch > rp->end)
	++rp;
      if (ch >= rp->start)
	cp = jisx0208_from_ucs_tab[rp->idx + ch - rp->start];
    }

  if (cp == NULL || cp[0] == '\0')
    return UNKNOWN_10646_CHAR;

  s[0] = cp[0];
  s[1] = cp[1];

  return 2;
}

#endif /* jis0208.h */
