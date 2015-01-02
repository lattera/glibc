/* Return the offset of one string within another.
   Copyright (C) 1994-2015 Free Software Foundation, Inc.
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

/* This particular implementation was written by Eric Blake, 2008.  */

#ifndef _LIBC
# include <config.h>
#endif

/* Specification of strstr.  */
#include <string.h>

#include <stdbool.h>

#ifndef _LIBC
# define __builtin_expect(expr, val)   (expr)
#endif

#define RETURN_TYPE char *
#define AVAILABLE(h, h_l, j, n_l)			\
  (!memchr ((h) + (h_l), '\0', (j) + (n_l) - (h_l))	\
   && ((h_l) = (j) + (n_l)))
#define CHECK_EOL (1)
#define RET0_IF_0(a) if (!a) goto ret0
#include "str-two-way.h"

#undef strstr

#ifndef STRSTR
#define STRSTR strstr
#endif

/* Return the first occurrence of NEEDLE in HAYSTACK.  Return HAYSTACK
   if NEEDLE is empty, otherwise NULL if NEEDLE is not found in
   HAYSTACK.  */
char *
STRSTR (const char *haystack_start, const char *needle_start)
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
    ok &= *haystack++ == *needle++;
  if (*needle)
    return NULL;
  if (ok)
    return (char *) haystack_start;

  /* Reduce the size of haystack using strchr, since it has a smaller
     linear coefficient than the Two-Way algorithm.  */
  needle_len = needle - needle_start;
  haystack = strchr (haystack_start + 1, *needle_start);
  if (!haystack || __builtin_expect (needle_len == 1, 0))
    return (char *) haystack;
  needle -= needle_len;
  haystack_len = (haystack > haystack_start + needle_len ? 1
		  : needle_len + haystack_start - haystack);

  /* Perform the search.  Abstract memory is considered to be an array
     of 'unsigned char' values, not an array of 'char' values.  See
     ISO C 99 section 6.2.6.1.  */
  if (needle_len < LONG_NEEDLE_THRESHOLD)
    return two_way_short_needle ((const unsigned char *) haystack,
				 haystack_len,
				 (const unsigned char *) needle, needle_len);
  return two_way_long_needle ((const unsigned char *) haystack, haystack_len,
			      (const unsigned char *) needle, needle_len);
}
libc_hidden_builtin_def (strstr)

#undef LONG_NEEDLE_THRESHOLD
