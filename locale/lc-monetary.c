/* Define current locale data for LC_MONETARY category.
   Copyright (C) 1995, 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include "localeinfo.h"

_NL_CURRENT_DEFINE (LC_MONETARY);

const uint32_t *__monetary_conversion_rate;

void
_nl_postload_monetary (void)
{
#if BYTE_ORDER == BIG_ENDIAN
#define bo(x) x##_EB
#elif BYTE_ORDER == LITTLE_ENDIAN
#define bo(x) x##_EL
#else
#error bizarre byte order
#endif
#define paste(a,b) paste1(a,b)
#define paste1(a,b) a##b

#define current(type,x,offset) \
  ((const type *) _NL_CURRENT (LC_MONETARY, paste(_NL_MONETARY_,x)) + offset)

  __monetary_conversion_rate = current (uint32_t, bo (CONVERSION_RATE), 0);
}
