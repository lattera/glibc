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


extern const u_int32_t _nl_C_LC_COLLATE_symbol_hash[];
extern const char _nl_C_LC_COLLATE_symbol_strings[];
extern const u_int32_t _nl_C_LC_COLLATE_symbol_classes[];


_NL_CURRENT_DEFINE (LC_COLLATE);

const u_int32_t *__collate_tablewc;
const u_int32_t *__collate_extrawc;

const u_int32_t *__collate_element_hash;
const char *__collate_element_strings;
const uint32_t *__collate_element_values;

const u_int32_t *__collate_symbol_hash = _nl_C_LC_COLLATE_symbol_hash;
const char *__collate_symbol_strings = _nl_C_LC_COLLATE_symbol_strings;
const u_int32_t *__collate_symbol_classeswc = _nl_C_LC_COLLATE_symbol_classes;


/* We are called after loading LC_CTYPE data to load it into
   the variables used by the collation functions and regex.  */
void
_nl_postload_collate (void)
{
#define paste(a,b) paste1(a,b)
#define paste1(a,b) a##b

#define current(x)							      \
  ((const unsigned int *) _NL_CURRENT (LC_COLLATE, paste(_NL_COLLATE_,x)))

  __collate_tablewc = current (TABLEWC);
  __collate_extrawc = current (EXTRAWC);

  __collate_element_hash = current (ELEM_HASH);
  __collate_element_strings = (const char *) current (ELEM_STR_POOL);
  __collate_element_values = (const uint32_t *) current (ELEM_VAL);

  __collate_symbol_hash = current (SYMB_HASH);
  __collate_symbol_strings = (const char *) current (SYMB_STR_POOL);
  __collate_symbol_classeswc = current (SYMB_CLASSWC);
}
