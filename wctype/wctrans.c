/* Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <ctype.h>
#include <string.h>
#include <wctype.h>
#include "localeinfo.h"

wctrans_t
wctrans (const char *property)
{
  const char *names;
  size_t cnt;
  unsigned int **result;

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

  if (cnt == 0)
    return (wctrans_t) __ctype_toupper;
  else if (cnt == 1)
    return (wctrans_t) __ctype_tolower;

  /* We have to search the table.  */
  result = (unsigned int **) &_NL_CURRENT (LC_CTYPE, _NL_CTYPE_WIDTH);

#if __BYTE_ORDER == _BIG_ENDIAN
  return (wctrans_t) result[1 + 2 * cnt];
#else
  return (wctrans_t) result[1 + 2 * cnt + 1];
#endif
}
