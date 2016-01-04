/* Charset name normalization.
   Copyright (C) 2001-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 2001.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <ctype.h>
#include <locale.h>


static void
strip (char *wp, const char *s)
{
  int slash_count = 0;

  while (*s != '\0')
    {
      if (__isalnum_l (*s, _nl_C_locobj_ptr)
	  || *s == '_' || *s == '-' || *s == '.' || *s == ',' || *s == ':')
	*wp++ = __toupper_l (*s, _nl_C_locobj_ptr);
      else if (*s == '/')
	{
	  if (++slash_count == 3)
	    break;
	  *wp++ = '/';
	}
      ++s;
    }

  while (slash_count++ < 2)
    *wp++ = '/';

  *wp = '\0';
}


static inline char * __attribute__ ((unused, always_inline))
upstr (char *dst, const char *str)
{
  char *cp = dst;
  while ((*cp++ = __toupper_l (*str++, _nl_C_locobj_ptr)) != '\0')
    /* nothing */;
  return dst;
}
