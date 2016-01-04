/* Return the offset of one string within another.
   Copyright (C) 1994-2016 Free Software Foundation, Inc.
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

/*
 * My personal strstr() implementation that beats most other algorithms.
 * Until someone tells me otherwise, I assume that this is the
 * fastest implementation of strstr() in C.
 * I deliberately chose not to comment it.  You should have at least
 * as much fun trying to understand it, as I had to write it :-).
 *
 * Stephen R. van den Berg, berg@pool.informatik.rwth-aachen.de	*/

#if HAVE_CONFIG_H
# include <config.h>
#endif

/* Specification.  */
#include <string.h>

#include <ctype.h>
#include <stdbool.h>
#include <strings.h>

#define TOLOWER(Ch) tolower (Ch)

/* Two-Way algorithm.  */
#define RETURN_TYPE char *
#define AVAILABLE(h, h_l, j, n_l)			\
  (!memchr ((h) + (h_l), '\0', (j) + (n_l) - (h_l))	\
   && ((h_l) = (j) + (n_l)))
#define CHECK_EOL (1)
#define RET0_IF_0(a) if (!a) goto ret0
#define CANON_ELEMENT(c) TOLOWER (c)
#define CMP_FUNC(p1, p2, l)				\
  __strncasecmp ((const char *) (p1), (const char *) (p2), l)
#include "str-two-way.h"

#undef strcasestr
#undef __strcasestr

#ifndef STRCASESTR
#define STRCASESTR __strcasestr
#endif


/* Find the first occurrence of NEEDLE in HAYSTACK, using
   case-insensitive comparison.  This function gives unspecified
   results in multibyte locales.  */
char *
STRCASESTR (const char *haystack_start, const char *needle_start)
{
  const char *haystack = haystack_start;
  const char *needle = needle_start;
  size_t needle_len; /* Length of NEEDLE.  */
  size_t haystack_len; /* Known minimum length of HAYSTACK.  */
  bool ok = true; /* True if NEEDLE is prefix of HAYSTACK.  */

  /* Determine length of NEEDLE, and in the process, make sure
     HAYSTACK is at least as long (no point processing all of a long
     NEEDLE if HAYSTACK is too short).  */
  while (*haystack && *needle)
    {
      ok &= (TOLOWER ((unsigned char) *haystack)
	     == TOLOWER ((unsigned char) *needle));
      haystack++;
      needle++;
    }
  if (*needle)
    return NULL;
  if (ok)
    return (char *) haystack_start;
  needle_len = needle - needle_start;
  haystack = haystack_start + 1;
  haystack_len = needle_len - 1;

  /* Perform the search.  Abstract memory is considered to be an array
     of 'unsigned char' values, not an array of 'char' values.  See
     ISO C 99 section 6.2.6.1.  */
  if (needle_len < LONG_NEEDLE_THRESHOLD)
    return two_way_short_needle ((const unsigned char *) haystack,
				 haystack_len,
				 (const unsigned char *) needle_start,
				 needle_len);
  return two_way_long_needle ((const unsigned char *) haystack, haystack_len,
			      (const unsigned char *) needle_start,
			      needle_len);
}

#undef LONG_NEEDLE_THRESHOLD

#ifndef NO_ALIAS
weak_alias (__strcasestr, strcasestr)
#endif
