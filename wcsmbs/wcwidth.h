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
#include "../wctype/wchar-lookup.h"

/* Tables containing character property information.  */
extern const char *__ctype32_wctype[12];

/* Table containing width information.  */
extern const char *__ctype32_width;

static __inline int
internal_wcwidth (wint_t wc)
{
  unsigned char res;

  if (wc == L'\0')
    return 0;

  if (wctype_table_lookup (__ctype32_wctype[__ISwprint], wc) == 0)
    return -1;

  res = wcwidth_table_lookup (__ctype32_width, wc);
  return res == (unsigned char) '\xff' ? -1 : (int) res;
}
