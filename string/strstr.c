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
  (((j) + (n_l) <= (h_l)) || ((h_l) += __strnlen ((void*)((h) + (h_l)), 512), \
			      (j) + (n_l) <= (h_l)))
#define CHECK_EOL (1)
#define RET0_IF_0(a) if (!a) goto ret0
#define FASTSEARCH(S,C,N) (void*) strchr ((void*)(S), (C))
#include "str-two-way.h"

#undef strstr

#ifndef STRSTR
#define STRSTR strstr
#endif

/* Return the first occurrence of NEEDLE in HAYSTACK.  Return HAYSTACK
   if NEEDLE is empty, otherwise NULL if NEEDLE is not found in
   HAYSTACK.  */
char *
STRSTR (const char *haystack, const char *needle)
{
  size_t needle_len; /* Length of NEEDLE.  */
  size_t haystack_len; /* Known minimum length of HAYSTACK.  */

  /* Handle empty NEEDLE special case.  */
  if (needle[0] == '\0')
    return (char *) haystack;

  /* Skip until we find the first matching char from NEEDLE.  */
  haystack = strchr (haystack, needle[0]);
  if (haystack == NULL || needle[1] == '\0')
    return (char *) haystack;

  /* Ensure HAYSTACK length is at least as long as NEEDLE length.
     Since a match may occur early on in a huge HAYSTACK, use strnlen
     and read ahead a few cachelines for improved performance.  */
  needle_len = strlen (needle);
  haystack_len = __strnlen (haystack, needle_len + 256);
  if (haystack_len < needle_len)
    return NULL;

  /* Check whether we have a match.  This improves performance since we avoid
     the initialization overhead of the two-way algorithm.  */
  if (memcmp (haystack, needle, needle_len) == 0)
    return (char *) haystack;

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
