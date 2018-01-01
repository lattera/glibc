/* Return the offset of one string within another.
   Copyright (C) 1994-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#if HAVE_CONFIG_H
# include <config.h>
#endif

/* Specification.  */
#include <string.h>

#include <ctype.h>
#include <stdbool.h>
#include <strings.h>

#define USE_AS_STRCASESTR
#define STRSTR __strcasestr
#define STRSTR2 strcasestr2
#define STRCHR strcasechr
#define STRSTR_SCAN strcasestr_scan

#undef strcasestr
#undef __strcasestr

#ifndef STRCASESTR
#define STRCASESTR __strcasestr
#endif

#define TOLOWER(Ch) (isupper (Ch) ? tolower (Ch) : (Ch))

#define CANON_ELEMENT(c) TOLOWER (c)
#define CMP_FUNC(p1, p2, l)				\
  __strncasecmp ((const char *) (p1), (const char *) (p2), l)

#include "strstr.c"

#ifndef NO_ALIAS
weak_alias (__strcasestr, strcasestr)
#endif
