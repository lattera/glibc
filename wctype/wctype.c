/* Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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

#include <endian.h>
#include <string.h>
#include <wctype.h>
#include <locale/localeinfo.h>

wctype_t
__wctype (const char *property)
{
  const char *names;
  wctype_t result;
  size_t proplen = strlen (property);

  names = _NL_CURRENT (LC_CTYPE, _NL_CTYPE_CLASS_NAMES);
  for (result = 1; result != 0; result <<= 1)
    {
      size_t nameslen = strlen (names);

      if (proplen == nameslen && memcmp (property, names, proplen) == 0)
	break;

      names += nameslen + 1;
      if (names[0] == '\0')
	return 0;
    }

#if __BYTE_ORDER == __BIG_ENDIAN
  return result;
#else
# define SWAPU32(w) \
  (((w) << 24) | (((w) & 0xff00) << 8) | (((w) >> 8) & 0xff00) | ((w) >> 24))

  return SWAPU32 (result);
#endif
}
weak_alias (__wctype, wctype)
