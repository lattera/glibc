/* Copyright (C) 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
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

#include <ctype.h>
#include <inttypes.h>
#include <string.h>
#include <wctype.h>
#include "../locale/localeinfo.h"

/* These are not exported.  */
extern const uint32_t *__ctype32_toupper;
extern const uint32_t *__ctype32_tolower;

wctrans_t
wctrans (const char *property)
{
  const char *names;
  size_t cnt;

  names = _NL_CURRENT (LC_CTYPE, _NL_CTYPE_MAP_NAMES);
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

  if (_NL_CURRENT_WORD (LC_CTYPE, _NL_CTYPE_HASH_SIZE) != 0)
    {
      /* Old locale format.  */
      if (cnt == 0)
	return (wctrans_t) __ctype32_toupper;
      else if (cnt == 1)
	return (wctrans_t) __ctype32_tolower;

      /* We have to search the table.  */
      return (wctrans_t) (const int32_t *) _NL_CURRENT (LC_CTYPE, _NL_NUM_LC_CTYPE + cnt - 2);
    }
  else
    {
      /* New locale format.  */
      size_t i = _NL_CURRENT_WORD (LC_CTYPE, _NL_CTYPE_MAP_OFFSET) + cnt;
      return (wctrans_t) _nl_current_LC_CTYPE->values[i].string;
    }
}
