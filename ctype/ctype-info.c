/* Copyright (C) 1991,92,95,96,97,99,2000,2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <ctype.h>
#include <locale/localeinfo.h>

/* Defined in locale/C-ctype.c.  */
extern const char _nl_C_LC_CTYPE_class[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_class32[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_toupper[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_tolower[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_class_upper[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_class_lower[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_class_alpha[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_class_digit[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_class_xdigit[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_class_space[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_class_print[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_class_graph[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_class_blank[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_class_cntrl[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_class_punct[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_class_alnum[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_map_toupper[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_map_tolower[] attribute_hidden;
extern const char _nl_C_LC_CTYPE_width[] attribute_hidden;

#define b(t,x,o) (((const t *) _nl_C_LC_CTYPE_##x) + o)

const unsigned short int *__ctype_b = b (unsigned short int, class, 128);
const __uint32_t *__ctype32_b = b (__uint32_t, class32, 0);
const __int32_t *__ctype_tolower = b (__int32_t, tolower, 128);
const __int32_t *__ctype_toupper = b (__int32_t, toupper, 128);
const __uint32_t *__ctype32_tolower = b (__uint32_t, tolower, 128);
const __uint32_t *__ctype32_toupper = b (__uint32_t, toupper, 128);
const char *__ctype32_wctype[12] attribute_hidden =
{
  b(char, class_upper, 32),
  b(char, class_lower, 32),
  b(char, class_alpha, 32),
  b(char, class_digit, 32),
  b(char, class_xdigit, 32),
  b(char, class_space, 32),
  b(char, class_print, 32),
  b(char, class_graph, 32),
  b(char, class_blank, 32),
  b(char, class_cntrl, 32),
  b(char, class_punct, 32),
  b(char, class_alnum, 32)
};
const char *__ctype32_wctrans[2] attribute_hidden =
{
  b(char, map_toupper, 0),
  b(char, map_tolower, 0)
};
const char *__ctype32_width attribute_hidden = b (char, width, 0);
