/* Define current locale data for LC_COLLATE category.
   Copyright (C) 1995, 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
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
#include <endian.h>


_NL_CURRENT_DEFINE (LC_COLLATE);

const int32_t *__collate_tablemb;
const unsigned char *__collate_weightmb;
const unsigned char *__collate_extramb;

/* We are called after loading LC_CTYPE data to load it into
   the variables used by the collation functions and regex.  */
void
_nl_postload_collate (void)
{
#define paste(a,b) paste1(a,b)
#define paste1(a,b) a##b
#define current(x) _NL_CURRENT (LC_COLLATE, paste(_NL_COLLATE_,x))

  __collate_tablemb = (const int32_t *) current (TABLEMB);
  __collate_weightmb = (const unsigned char *) current (WEIGHTMB);
  __collate_extramb = (const unsigned char *) current (EXTRAMB);
}
