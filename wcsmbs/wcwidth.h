/* Internal header containing implementation of wcwidth() function.
   Copyright (C) 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
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

#include <wchar.h>
#include <wctype.h>
#include "../wctype/cname-lookup.h"
#include "../wctype/wchar-lookup.h"

/* Tables containing character property information.  */
extern const char *__ctype32_wctype[12];

/* Tables containing width information.  */
extern unsigned char *__ctype_width;
extern const char *__ctype32_width;

/* If the program is compiled without optimization the following declaration
   is not visible in the header.   */
extern unsigned int *__ctype32_b;

static __inline int
internal_wcwidth (wint_t wc)
{
  if (wc == L'\0')
    return 0;

  if (_NL_CURRENT_WORD (LC_CTYPE, _NL_CTYPE_HASH_SIZE) != 0)
    {
      /* Old locale format.  */
      size_t idx;
      unsigned char res;

      idx = cname_lookup (wc);
      if (idx == ~((size_t) 0) || (__ctype32_b[idx] & _ISwprint) == 0)
	return -1;

      res = __ctype_width[idx];
      return res == (unsigned char) '\xff' ? -1 : (int) res;
    }
  else
    {
      /* New locale format.  */
      unsigned char res;

      if (wctype_table_lookup (__ctype32_wctype[__ISwprint], wc) == 0)
	return -1;

      res = wcwidth_table_lookup (__ctype32_width, wc);
      return res == (unsigned char) '\xff' ? -1 : (int) res;
    }
}
