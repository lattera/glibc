/* Copyright (C) 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
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

#include <string.h>
#include <wctype.h>
#include "../locale/localeinfo.h"

wctrans_t
__wctrans_l (const char *property, __locale_t locale)
{
  const char *names;
  size_t cnt;

  names = locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_MAP_NAMES)].string;
  cnt = 0;
  while (names[0] != '\0')
    {
      if (strcmp (property, names) == 0)
	break;

      names = strchr (names, '\0') + 1;
      ++cnt;
    }

  if (names[0] == '\0')
    return 0;

  if (locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_HASH_SIZE)].word == 0)
    {
      /* Old locale format.  */
      if (cnt == __TOW_toupper)
	return (wctrans_t) locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_TOUPPER32)].string;
      else if (cnt == __TOW_tolower)
	return (wctrans_t) locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_TOLOWER32)].string;

      /* We have to search the table.  */
      return (wctrans_t) (const int32_t *) locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_NUM_LC_CTYPE + cnt - 2)].string;
    }
  else
    {
      /* New locale format.  */
      size_t i = locale->__locales[LC_CTYPE]->values[_NL_ITEM_INDEX (_NL_CTYPE_MAP_OFFSET)].word + cnt;
      return (wctrans_t) locale->__locales[LC_CTYPE]->values[i].string;
    }
}
