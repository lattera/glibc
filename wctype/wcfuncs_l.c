/* Copyright (C) 1996, 1997, 2000, 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <wctype.h>
#include <stdint.h>
#include <locale.h>
#include <locale/localeinfo.h>

#define USE_IN_EXTENDED_LOCALE_MODEL
#include "wchar-lookup.h"

/* Provide real-function versions of all the wctype macros.  */

#define	func(name, type) \
  int name (wint_t wc, __locale_t locale)				      \
  {									      \
    size_t i = locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_CLASS_OFFSET)].word + type; \
    const char *desc = locale->__locales[LC_CTYPE]->values[i].string;	      \
    return wctype_table_lookup (desc, wc);				      \
  }

func (__iswalnum_l, __ISwalnum)
func (__iswalpha_l, __ISwalpha)
func (__iswblank_l, __ISwblank)
func (__iswcntrl_l, __ISwcntrl)
func (__iswdigit_l, __ISwdigit)
func (__iswlower_l, __ISwlower)
func (__iswgraph_l, __ISwgraph)
func (__iswprint_l, __ISwprint)
func (__iswpunct_l, __ISwpunct)
func (__iswspace_l, __ISwspace)
func (__iswupper_l, __ISwupper)
func (__iswxdigit_l, __ISwxdigit)

wint_t
(__towlower_l) (wint_t wc, __locale_t locale)
{
  size_t i = locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_MAP_OFFSET)].word + __TOW_tolower;
  const char *desc = locale->__locales[LC_CTYPE]->values[i].string;
  return wctrans_table_lookup (desc, wc);
}

wint_t
(__towupper_l) (wint_t wc, __locale_t locale)
{
  size_t i = locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_MAP_OFFSET)].word + __TOW_toupper;
  const char *desc = locale->__locales[LC_CTYPE]->values[i].string;
  return wctrans_table_lookup (desc, wc);
}
