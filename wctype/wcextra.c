/* Additional non standardized wide character classification functions.
   Copyright (C) 1997, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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
#define __NO_WCTYPE	1
#include <wctype.h>

#include "cname-lookup.h"
#include "wchar-lookup.h"

/* If the program is compiled without optimization the following declaration
   is not visible in the header.   */
extern unsigned int *__ctype32_b;

/* This is not exported.  */
extern const char *__ctype32_wctype[12];

int
(iswblank) (wint_t wc)
{
  if (_NL_CURRENT_WORD (LC_CTYPE, _NL_CTYPE_HASH_SIZE) != 0)
    {
      /* Old locale format.  */
      size_t idx;

      idx = cname_lookup (wc);
      if (idx == ~((size_t) 0))
	return 0;

      return __ctype32_b[idx] & _ISwblank;
    }
  else
    {
      /* New locale format.  */
      return wctype_table_lookup (__ctype32_wctype[__ISwblank], wc);
    }
}
