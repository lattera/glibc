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

#include <endian.h>
#include <string.h>
#include <wctype.h>
#include "../locale/localeinfo.h"

wctype_t
wctype (const char *property)
{
  const char *names;
  wctype_t result;

  names = _NL_CURRENT (LC_CTYPE, _NL_CTYPE_CLASS_NAMES);
  for (result = 1; result != 0; result <<= 1)
    {
      if (strcmp (property, names) == 0)
	break;

      names = strchr (names, '\0') + 1;
      if (names[0] == '\0')
	return 0;
    }

#if __BYTE_ORDER == __BIG_ENDIAN
  return result;
#else
# define SWAPU32(w) \
  (((w) << 24) | (((w) & 0xff00) << 8) | (((w) >> 8) & 0xff00) | ((w) >> 24))

# define SWAPU16(w) \
  (((w) >> 8) | ((w) << 8))

  if (sizeof (wctype_t) == 4)
    return SWAPU32 (result);
  else
    return SWAPU16 (result);
#endif
}
