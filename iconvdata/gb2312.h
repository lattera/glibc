/* Access functions for GB2312 conversion.
   Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

#ifndef _GB2312_H
#define _GB2312_H	1

#include <gconv.h>
#include <stdint.h>

/* Conversion table.  */
extern const uint16_t gb2312_to_ucs[];


static inline wchar_t
gb2312_to_ucs4 (const char **s, size_t avail, unsigned char offset)
{
  unsigned char ch = *(*s);
  unsigned char ch2;
  int idx;

  if (ch < offset || (ch - offset) <= 0x20 || (ch - offset) > 0x77)
    return UNKNOWN_10646_CHAR;

  if (avail < 2)
    return 0;

  ch2 = (*s)[1];
  if ((ch2 - offset) <= 0x20 || (ch2 - offset) >= 0x7f)
    return UNKNOWN_10646_CHAR;

  idx = (ch - 0x21 - offset) * 94 + (ch2 - 0x21 - offset);
  if (idx >= 0x1ff1)
    return UNKNOWN_10646_CHAR;

  (*s) += 2;

  return gb2312_to_ucs[idx] ?: ((*s) -= 2, UNKNOWN_10646_CHAR);
}


extern const char gb2312_from_ucs4_tab1[][2];
extern const char gb2312_from_ucs4_tab2[][2];
extern const char gb2312_from_ucs4_tab3[][2];
extern const char gb2312_from_ucs4_tab4[][2];
extern const char gb2312_from_ucs4_tab5[][2];
extern const char gb2312_from_ucs4_tab6[][2];
extern const char gb2312_from_ucs4_tab7[][2];
extern const char gb2312_from_ucs4_tab8[][2];
extern const char gb2312_from_ucs4_tab9[][2];

static inline size_t
ucs4_to_gb2312 (wchar_t wch, char *s, size_t avail)
{
  unsigned int ch = (unsigned int) wch;
  char buf[2];
  const char *cp = NULL;

  if (ch < 0xa4)
    cp = NULL;
  else if (ch < 0x101)
    cp = gb2312_from_ucs4_tab1[ch - 0xa4];
  else if (ch == 0x113)
    cp = "\x28\x25";
  else if (ch == 0x11b)
    cp = "\x28\x27";
  else if (ch == 0x12b)
    cp = "\x28\x29";
  else if (ch == 0x14d)
    cp = "\x28\x2d";
  else if (ch == 0x16b)
    cp = "\x28\x31";
  else if (ch == 0x1ce)
    cp = "\x28\x23";
  else if (ch == 0x1d0)
    cp = "\x28\x2b";
  else if (ch == 0x1d2)
    cp = "\x28\x2f";
  else if (ch == 0x1d4)
    cp = "\x28\x33";
  else if (ch == 0x1d6)
    cp = "\x28\x35";
  else if (ch == 0x1d8)
    cp = "\x28\x36";
  else if (ch == 0x1da)
    cp = "\x28\x37";
  else if (ch == 0x1dc)
    cp = "\x28\x38";
  else if (ch == 0x2c7)
    cp = "\x21\x26";
  else if (ch == 0x2c9)
    cp = "\x21\x25";
  else if (ch >= 0x391 && ch <= 0x3c9)
    cp = gb2312_from_ucs4_tab2[ch - 0x391];
  else if (ch >= 0x401 && ch <= 0x451)
    cp = gb2312_from_ucs4_tab3[ch - 0x401];
  else if (ch >= 0x2015 && ch <= 0x203b)
    cp = gb2312_from_ucs4_tab4[ch - 0x2015];
  else if (ch >= 0x2103 && ch <= 0x22a5)
    cp = gb2312_from_ucs4_tab5[ch - 0x2103];
  else if (ch == 0x2313)
    cp = "\x21\x50";
  else if (ch >= 0x2460 && ch <= 0x249b)
    cp = gb2312_from_ucs4_tab6[ch - 0x2460];
  else if (ch >= 0x2500 && ch <= 0x254b)
    {
      buf[0] = '\x29';
      buf[1] = '\x24' + (ch & 256);
      cp = buf;
    }
  else if (ch == 0x25a0)
    cp = "\x21\x76";
  else if (ch == 0x25a1)
    cp = "\x21\x75";
  else if (ch == 0x25b2)
    cp = "\x21\x78";
  else if (ch == 0x25b3)
    cp = "\x21\x77";
  else if (ch == 0x25c6)
    cp = "\x21\x74";
  else if (ch == 0x25c7)
    cp = "\x21\x73";
  else if (ch == 0x25cb)
    cp = "\x21\x70";
  else if (ch == 0x25ce)
    cp = "\x21\x72";
  else if (ch == 0x25cf)
    cp = "\x21\x71";
  else if (ch == 0x2605)
    cp = "\x21\x6f";
  else if (ch == 0x2606)
    cp = "\x21\x6e";
  else if (ch == 0x2640)
    cp = "\x21\x62";
  else if (ch == 0x2642)
    cp = "\x21\x61";
  else if (ch >= 0x3000 && ch <= 0x3129)
    cp = gb2312_from_ucs4_tab7[ch - 0x3000];
  else if (ch >= 0x3220 && ch <= 0x3229)
    {
      buf[0] = '\x22';
      buf[1] = '\x65' + (ch - 0x3220);
      cp = buf;
    }
  else if (ch >= 0x4e00 && ch <= 0x9fa0)
    cp = gb2312_from_ucs4_tab8[ch - 0x4e00];
  else if (ch >= 0xff01 && ch <= 0xff5e)
    cp = gb2312_from_ucs4_tab9[ch - 0xff01];
  else if (ch == 0xffe0)
    cp = "\x21\x69";
  else if (ch == 0xffe1)
    cp = "\x21\x6a";
  else if (ch == 0xffe3)
    cp = "\x23\x7e";
  else if (ch == 0xffe5)
    cp = "\x23\x24";
  else
    return UNKNOWN_10646_CHAR;

  if (cp[1] != '\0' && avail < 2)
    return 0;

  s[0] = cp[0];
  s[1] = cp[1];

  return 2;
}

#endif	/* gb2312.h */
