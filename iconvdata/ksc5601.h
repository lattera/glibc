/* Access functions for KS C 5601-1992 based encoding conversion.
   Copyright (C) 1998 Free Software Foundation, Inc.
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

#ifndef _KSC5601_H
#define _KSC5601_H	1

#define KSC5601_HANGUL 2350
#define KSC5601_HANJA  4888
#define KSC5601_SYMBOL  986

#include <gconv.h>
#include <stdint.h>

/* Conversion table.  */
extern const uint16_t __ksc5601_hangul_to_ucs[KSC5601_HANGUL];
extern const uint16_t __ksc5601_sym_to_ucs[];
extern const uint16_t __ksc5601_sym_from_ucs[KSC5601_SYMBOL][2];
extern const uint16_t __ksc5601_hanja_to_ucs[KSC5601_HANJA];
extern const uint16_t __ksc5601_hanja_from_ucs[KSC5601_HANJA][2];


/*
static inline wchar_t
ksc5601_to_ucs4 (char **s, size_t avail)
*/
static inline uint32_t
ksc5601_to_ucs4 (uint16_t s)
{
  unsigned char ch = s / 256;
  unsigned char ch2;
  int idx;

  /* row 94(0x7e) and row 41(0x49) are user-defined area in KS C 5601 */

  if (ch <= 0x20 || ch >= 0x7e || ch == 0x49)
    return UNKNOWN_10646_CHAR;

  ch2 = s % 256;
  if (ch2 <= 0x20 || ch2 >= 0x7f)
    return UNKNOWN_10646_CHAR;

  idx = (ch - 0x21) * 94 + (ch2 - 0x21);

  /* 1410 = 15 * 94 , 3760 = 40 * 94
     Hangul in KS C 5601 : row 16 - row 40 */

  if (idx >= 1410 && idx < 3760)
    return __ksc5601_hangul_to_ucs[idx-1410];
  else if (idx > 3854)
    /* Hanja : row 42 - row 93 : 3854 = 94 * (42-1) */
   return __ksc5601_hanja_to_ucs[idx-3854];
  else
    return __ksc5601_sym_to_ucs[idx] ?: UNKNOWN_10646_CHAR;
}

static inline size_t
ucs4_to_ksc5601_hangul (uint32_t wch, uint16_t *s)
{
  int l = 0;
  int m;
  int u = KSC5601_HANGUL - 1;
  uint32_t try;

  while (l <= u)
    {
      try = (uint32_t) __ksc5601_hangul_to_ucs[m=(l+u)/2];
      if (try > wch)
	u = m - 1;
      else if (try < wch)
	l= m + 1;
      else
	{
	  *s = (uint16_t) ((m / 94) * 256  + m % 94 + 0x3021) ;
	  return 2;
	}
    }
  return  0;
}


static inline size_t
ucs4_to_ksc5601_hanja (uint32_t wch, uint16_t *s)
{
  int l = 0;
  int m;
  int u = KSC5601_HANJA - 1;
  uint32_t try;

  while (l <= u)
    {
      m = (l + u) / 2;
      try = (uint32_t) __ksc5601_hanja_from_ucs[m][0];
      if (try > wch)
	u=m-1;
      else if (try < wch)
	l = m + 1;
      else
	{
	  *s = __ksc5601_hanja_from_ucs[m][1];
	  return 2;
	}
    }
  return 0;
}

static inline  size_t
ucs4_to_ksc5601_sym (uint32_t wch, uint16_t *s)
{
  int l = 0;
  int m;
  int u = KSC5601_SYMBOL - 1;
  uint32_t try;

  while (l <= u)
    {
      m = (l + u) / 2;
      try = __ksc5601_sym_from_ucs[m][0];
      if (try > wch)
	u = m - 1;
      else if (try < wch)
	l = m + 1;
      else
	{
	  *s = __ksc5601_sym_from_ucs[m][1];
	  return 2;
	}
    }
  return 0;
}


/*
static inline size_t
ucs4_to_ksc5601 (wchar_t wch, char **s, size_t avail)
*/

static inline size_t
ucs4_to_ksc5601 (uint32_t ch, uint16_t *s)
{
  *s = (uint16_t) UNKNOWN_10646_CHAR;  /* FIXIT */

  if (ch >= 0xac00 && ch <= 0xd7a3)
    return ucs4_to_ksc5601_hangul (ch, s);
  else if (ch >= 0x4e00 && ch <= 0x9fff || ch >= 0xf900 && ch <= 0xfa0b)
    return ucs4_to_ksc5601_hanja (ch, s);
  else
    return ucs4_to_ksc5601_sym (ch, s);
}

#endif /* ksc5601.h */
