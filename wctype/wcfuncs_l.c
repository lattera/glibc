/* Copyright (C) 1996, 1997, 2000 Free Software Foundation, Inc.
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

#define	__NO_WCTYPE
#include <wctype.h>
#include <stdint.h>

#define USE_IN_EXTENDED_LOCALE_MODEL
#include "cname-lookup.h"
#include "wchar-lookup.h"

/* Provide real-function versions of all the wctype macros.  */

#define	func(name, type) \
  int name (wint_t wc, __locale_t locale)				      \
  {									      \
    if (locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_HASH_SIZE)].word != 0) \
      {									      \
	/* Old locale format.  */					      \
	const uint32_t *class32_b;					      \
	size_t idx;							      \
									      \
	idx = cname_lookup (wc, locale);				      \
	if (idx == ~((size_t) 0))					      \
	  return 0;							      \
									      \
	class32_b = (uint32_t *)					      \
	  locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_CLASS32)].string; \
									      \
	return class32_b[idx] & _ISwbit (type);				      \
      }									      \
    else								      \
      {									      \
	/* New locale format.  */					      \
	size_t i = locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_CLASS_OFFSET)].word + type; \
	const char *desc = locale->__locales[LC_CTYPE]->values[i].string;     \
	return wctype_table_lookup (desc, wc);				      \
      }									      \
  }

func (__iswalnum_l, __ISwalnum)
func (__iswalpha_l, __ISwalpha)
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
  if (locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_HASH_SIZE)].word != 0)
    {
      /* Old locale format.  */
      const int32_t *class32_tolower;
      size_t idx;

      idx = cname_lookup (wc, locale);
      if (idx == ~((size_t) 0))
	return 0;

      class32_tolower = (const int32_t *)
	locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_TOLOWER32)].string;

      return class32_tolower[idx];
    }
  else
    {
      /* New locale format.  */
      size_t i = locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_MAP_OFFSET)].word + __TOW_tolower;
      const char *desc = locale->__locales[LC_CTYPE]->values[i].string;
      return wctrans_table_lookup (desc, wc);
    }
}

wint_t
(__towupper_l) (wint_t wc, __locale_t locale)
{
  if (locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_HASH_SIZE)].word != 0)
    {
      /* Old locale format.  */
      const int32_t *class32_toupper;
      size_t idx;

      idx = cname_lookup (wc, locale);
      if (idx == ~((size_t) 0))
	return 0;

      class32_toupper = (const int32_t *)
	locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_TOUPPER32)].string;

      return class32_toupper[idx];
    }
  else
    {
      /* New locale format.  */
      size_t i = locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_MAP_OFFSET)].word + __TOW_toupper;
      const char *desc = locale->__locales[LC_CTYPE]->values[i].string;
      return wctrans_table_lookup (desc, wc);
    }
}
