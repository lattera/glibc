/* Copyright (C) 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
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

#ifdef USE_IN_EXTENDED_LOCALE_MODEL
# define STRXFRM __wcsxfrm_l
#else
# define STRXFRM wcsxfrm
#endif


#ifndef USE_IN_EXTENDED_LOCALE_MODEL
size_t
STRXFRM (wchar_t *dest, const wchar_t *src, size_t n)
#else
size_t
STRXFRM (wchar_t *dest, const wchar_t *src, size_t n, __locale_t l)
#endif
{
  if (n != 0)
    __wcpncpy (dest, src, n);

  return __wcslen (src);
}
