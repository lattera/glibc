/* Access functions for CNS 11643, plane 2 handling.
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

#include <stdint.h>

/* Table for CNS 11643, plane 2 to UCS4 conversion.  */
extern const uint16_t __cns11643l2_to_ucs4_tab[];
extern const uint16_t __cns11643l14_to_ucs4_tab[];


static inline wchar_t
cns11643_to_ucs4 (const char **s, size_t avail, unsigned char offset)
{
  unsigned char ch = *(*s);
  unsigned char ch2;
  unsigned char ch3;
  wchar_t result;
  int idx;

  if (ch < offset || (ch - offset) <= 0x20 || (ch - offset) > 0x30)
    return UNKNOWN_10646_CHAR;

  if (avail < 3)
    return 0;

  ch2 = (*s)[1];
  if ((ch2 - offset) <= 0x20 || (ch2 - offset) >= 0x7f)
    return UNKNOWN_10646_CHAR;

  ch3 = (*s)[2];
  if ((ch3 - offset) <= 0x20 || (ch3 - offset) >= 0x7f)
    return UNKNOWN_10646_CHAR;

  idx = (ch2 - 0x21 - offset) * 94 + (ch3 - 0x21 - offset);

  if ((ch - 0x21 - offset) == 1)
    {
      if (idx > 0x2196)
	return UNKNOWN_10646_CHAR;
      result = __cns11643l1_to_ucs4_tab[idx];
    }
  else if ((ch - 0x21 - offset) == 2)
    {
      if (idx > 0x1de1)
	return UNKNOWN_10646_CHAR;
      result = __cns11643l2_to_ucs4_tab[idx];
    }
  else if ((ch - 0x21 - offset) == 0xe)
    {
      if (idx > 0x19bd)
	return UNKNOWN_10646_CHAR;
      result = __cns11643l14_to_ucs4_tab[idx];
    }
  else
    return UNKNOWN_10646_CHAR;

  if (result != L'\0')
    (*s) += 3;
  else
    result = UNKNOWN_10646_CHAR;

  return result;
}


/* Tables for the UCS4 -> CNS conversion.  */
extern const char __cns11643l1_from_ucs4_tab1[][2];
extern const char __cns11643l1_from_ucs4_tab2[][2];
extern const char __cns11643l1_from_ucs4_tab3[][2];
extern const char __cns11643l1_from_ucs4_tab4[][2];
extern const char __cns11643l1_from_ucs4_tab5[][2];
extern const char __cns11643l1_from_ucs4_tab6[][2];
extern const char __cns11643l1_from_ucs4_tab7[][2];
extern const char __cns11643l1_from_ucs4_tab8[][2];
extern const char __cns11643l1_from_ucs4_tab9[][2];
extern const char __cns11643l1_from_ucs4_tab10[][2];
extern const char __cns11643l1_from_ucs4_tab11[][2];
extern const char __cns11643l1_from_ucs4_tab12[][2];
extern const char __cns11643l1_from_ucs4_tab13[][2];
extern const char __cns11643l1_from_ucs4_tab14[][2];
extern const char __cns11643_from_ucs4_tab[][3];


static inline size_t
ucs4_to_cns11643 (wchar_t wch, char *s, size_t avail)
{
  unsigned int ch = (unsigned int) wch;
  char buf[2];
  const char *cp = buf;
  int needed = 2;

  switch (ch)
    {
    case 0xa7 ... 0xf7:
      cp = __cns11643l1_from_ucs4_tab1[ch - 0xa7];
      break;
    case 0x2c7 ... 0x2d9:
      cp = __cns11643l1_from_ucs4_tab2[ch - 0x2c7];
      break;
    case 0x391 ... 0x3c9:
      cp = __cns11643l1_from_ucs4_tab3[ch - 0x391];
      break;
    case 0x2013 ... 0x203e:
      cp = __cns11643l1_from_ucs4_tab4[ch - 0x2013];
      break;
    case 0x2103:
      cp = "\x22\x6a";
      break;
    case 0x2105:
      cp = "\x22\x22";
      break;
    case 0x2109:
      cp = "\x22\x6b";
      break;
    case 0x2160 ...0x2169:
      buf[0] = '\x24';
      buf[1] = '\x2b' + (ch - 0x2160);
      break;
    case 0x2170 ... 0x2179:
      buf[0] = '\x26';
      buf[1] = '\x35' + (ch - 0x2170);
      break;
    case 0x2190 ... 0x2199:
      cp = __cns11643l1_from_ucs4_tab5[ch - 0x2190];
      break;
    case 0x2215 ... 0x2267:
      cp = __cns11643l1_from_ucs4_tab6[ch - 0x2215];
      break;
    case 0x22a5:
      cp = "\x22\x47";
      break;
    case 0x22bf:
      cp = "\x22\x4a";
      break;
    case 0x2400 ... 0x2421:
      cp = __cns11643l1_from_ucs4_tab7[ch - 0x2400];
      break;
    case 0x2460 ... 0x247d:
      cp = __cns11643l1_from_ucs4_tab8[ch - 0x2460];
      break;
    case 0x2500 ... 0x2642:
      cp = __cns11643l1_from_ucs4_tab9[ch - 0x2500];
    case 0x3000 ... 0x3029:
      cp = __cns11643l1_from_ucs4_tab10[ch - 0x3000];
      break;
    case 0x30fb:
      cp = "\x21\x26";
      break;
    case 0x3105 ... 0x3129:
      buf[0] = '\x25';
      buf[1] = '\x26' + (ch - 0x3105);
      break;
    case 0x32a3:
      cp = "\x22\x21";
      break;
    case 0x338e ... 0x33d5:
      cp = __cns11643l1_from_ucs4_tab11[ch - 0x338e];
      break;
    case 0x4e00 ... 0x9f9c:
      cp = __cns11643l1_from_ucs4_tab12[ch - 0x4e00];

      if (cp[0] == '\0')
	{
	  /* Let's try the other planes.  */
	  needed = 3;
	  cp = __cns11643_from_ucs4_tab[ch - 0x4e00];
	}
      break;
    case 0xfe30 ... 0xfe6b:
      cp = __cns11643l1_from_ucs4_tab13[ch - 0xfe30];
      break;
    case 0xff01 ... 0xff5d:
      cp = __cns11643l1_from_ucs4_tab14[ch - 0xff01];
      break;
    case 0xffe0:
      cp = "\x22\x66";
      break;
    case 0xffe1:
      cp = "\x22\x67";
      break;
    case 0xffe5:
      cp = "\x22\x64";
      break;
    default:
      cp = "";
    }

  if (cp[0] == '\0')
    return UNKNOWN_10646_CHAR;

  if (avail < needed)
    return 0;

  s[0] = cp[0];
  s[1] = cp[1];
  if (needed == 3)
    s[2] = cp[2];

  return needed;
}
