/* Copyright (C) 1996-2000,2002,2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.org>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <wchar.h>
#include <locale/localeinfo.h>


/* This is the private state used if PS is NULL.  */
static mbstate_t state;

size_t
__mbsrtowcs (dst, src, len, ps)
     wchar_t *dst;
     const char **src;
     size_t len;
     mbstate_t *ps;
{
  return __mbsrtowcs_l (dst, src, len, ps ?: &state, _NL_CURRENT_LOCALE);
}
weak_alias (__mbsrtowcs, mbsrtowcs)
