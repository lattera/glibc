/* Copyright (C) 1991, 1992, 1995, 1996 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ctype.h>
#include "../locale/localeinfo.h"

/* Defined in locale/C-ctype.c.  */
extern const char _nl_C_LC_CTYPE_class[];
extern const char _nl_C_LC_CTYPE_class32[];
extern const char _nl_C_LC_CTYPE_toupper[];
extern const char _nl_C_LC_CTYPE_tolower[];
extern const char _nl_C_LC_CTYPE_names[];
extern const char _nl_C_LC_CTYPE_width[];

#define b(t,x,o) (((const t *) _nl_C_LC_CTYPE_##x) + o);

const unsigned short int *__ctype_b = b (unsigned short int, class, 128);
const unsigned int *__ctype32_b = b (unsigned int, class32, 0);
const int *__ctype_tolower = b (int, tolower, 128);
const int *__ctype_toupper = b (int, toupper, 128);
const unsigned int *__ctype_names = b (unsigned int, names, 0);
const unsigned char *__ctype_width = b (unsigned char, width, 0);
