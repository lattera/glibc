/* Define current locale data for LC_COLLATE category.
Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include "localeinfo.h"
#include <endian.h>


extern const u_int32_t _nl_C_LC_COLLATE_symbol_hash[];
extern const char _nl_C_LC_COLLATE_symbol_strings[];
extern const u_int32_t _nl_C_LC_COLLATE_symbol_classes[];


_NL_CURRENT_DEFINE (LC_COLLATE);

const u_int32_t *__collate_table = NULL;
const u_int32_t *__collate_extra = NULL;

const u_int32_t *__collate_element_hash = NULL;
const char *__collate_element_strings = NULL;
const wchar_t *__collate_element_values = NULL;

const u_int32_t *__collate_symbol_hash = _nl_C_LC_COLLATE_symbol_hash;
const char *__collate_symbol_strings = _nl_C_LC_COLLATE_symbol_strings;
const u_int32_t *__collate_symbol_classes = _nl_C_LC_COLLATE_symbol_classes;


/* We are called after loading LC_CTYPE data to load it into
   the variables used by the collation functions and regex.  */
void
_nl_postload_collate (void)
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

#define current(x)							      \
  ((const unsigned int *) _NL_CURRENT (LC_COLLATE, paste(_NL_COLLATE_,x)))

  __collate_table = current (bo (TABLE));
  __collate_extra = current (bo (EXTRA));

  __collate_element_hash = current (bo (ELEM_HASH));
  __collate_element_strings = (const char *) current (ELEM_STR_POOL);
  __collate_element_values = (const wchar_t *) current (bo (ELEM_VAL));

  __collate_symbol_hash = current (bo (SYMB_HASH));
  __collate_symbol_strings = (const char *) current (SYMB_STR_POOL);
  __collate_symbol_classes = current (bo (SYMB_CLASS));
}
