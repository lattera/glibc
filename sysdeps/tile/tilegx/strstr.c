/* Copyright (C) 2013-2015 Free Software Foundation, Inc.
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

/* Specification of strstr.  */
#include <string.h>

#include <stdbool.h>
#include "string-endian.h"

#define RETURN_TYPE char *
#define AVAILABLE(h, h_l, j, n_l)			\
  (!memchr ((h) + (h_l), '\0', (j) + (n_l) - (h_l))	\
   && ((h_l) = (j) + (n_l)))
#include "str-two-way.h"
typeof(two_way_short_needle) two_way_short_needle __attribute__((unused));

#undef strstr

#ifndef STRSTR
#define STRSTR strstr
#endif

#ifndef STRSTR2
#define STRSTR2 strstr2
#endif

#ifndef STRCHR
#define STRCHR strchr
#endif

#ifndef STRSTR_SCAN
#define STRSTR_SCAN strstr_scan
#endif

#ifndef TOLOWER
# define TOLOWER(Ch) (Ch)
#endif

#ifdef USE_AS_STRCASESTR

static uint64_t
vec_tolower (uint64_t cc)
{
  /* For Uppercases letters, add 32 to convert to lower case.  */
  uint64_t less_than_eq_Z = __insn_v1cmpltui (cc, 'Z' + 1);
  uint64_t less_than_A =  __insn_v1cmpltui (cc, 'A');
  uint64_t is_upper = __insn_v1cmpne (less_than_eq_Z, less_than_A);
  return __insn_v1add (cc,__insn_v1shli (is_upper, 5));
}

/* There is no strcasechr() defined, but needed for 1 byte case
   of strcasestr(), so create it here.  */

static char *
strcasechr (const char *s, int c)
{
  int z, g;

  c = tolower (c);

  /* Get an aligned pointer.  */
  const uintptr_t s_int = (uintptr_t) s;
  const uint64_t *p = (const uint64_t *) (s_int & -8);

  /* Create eight copies of the byte for which we are looking.  */
  const uint64_t goal = copy_byte(c);

  /* Read the first aligned word, but force bytes before the string to
     match neither zero nor goal (we make sure the high bit of each byte
     is 1, and the low 7 bits are all the opposite of the goal byte).  */
  const uint64_t before_mask = MASK (s_int);
  uint64_t v =
    (vec_tolower (*p) | before_mask) ^ (goal & __insn_v1shrui (before_mask, 1));

  uint64_t zero_matches, goal_matches;
  while (1)
    {
      /* Look for a terminating '\0'.  */
      zero_matches = __insn_v1cmpeqi (v, 0);

      /* Look for the goal byte.  */
      goal_matches = __insn_v1cmpeq (v, goal);

      if (__builtin_expect ((zero_matches | goal_matches) != 0, 0))
        break;

      v = vec_tolower (*++p);
    }

  z = CFZ (zero_matches);
  g = CFZ (goal_matches);

  /* If we found c before '\0' we got a match. Note that if c == '\0'
     then g == z, and we correctly return the address of the '\0'
     rather than NULL.  */
  return (g <= z) ? ((char *) p) + (g >> 3) : NULL;
}

# define vec_load(p) vec_tolower (*(p))
# define STRCHR strcasechr
# define CMP_FUNC __strncasecmp

#else

# define vec_load(p) (*(p))
# define STRCHR strchr
# define CMP_FUNC memcmp

#endif


/* Compare 2-character needle using SIMD.  */
static char *
STRSTR2 (const char *haystack_start, const char *needle)
{
  int z, g;

  __insn_prefetch (haystack_start + 64);

  /* Get an aligned pointer.  */
  const uintptr_t s_int = (uintptr_t) haystack_start;
  const uint64_t *p = (const uint64_t *) (s_int & -8);

  /* Create eight copies of the first byte for which we are looking.  */
  const uint64_t byte1 = copy_byte (TOLOWER (*needle));
  /* Create eight copies of the second byte for which we are looking.  */
  const uint64_t byte2 = copy_byte (TOLOWER (*(needle + 1)));

  /* Read the first aligned word, but force bytes before the string to
     match neither zero nor goal (we make sure the high bit of each byte
     is 1, and the low 7 bits are all the opposite of the goal byte).  */
  const uint64_t before_mask = MASK (s_int);
  uint64_t v =
    (vec_load (p) | before_mask) ^ (byte1 & __insn_v1shrui (before_mask, 1));

  uint64_t zero_matches, goal_matches;
  while (1)
    {
      /* Look for a terminating '\0'.  */
      zero_matches = __insn_v1cmpeqi (v, 0);
      uint64_t byte1_matches = __insn_v1cmpeq (v, byte1);
      if (__builtin_expect (zero_matches != 0, 0))
	{
	  /* This is the last vector.  Don't worry about matches
	     crossing into the next vector.  Shift the second byte
	     back 1 byte to align it with the first byte, then and to
	     check for both matching.  Each vector has a 1 in the LSB
	     of the byte if there was match.  */
	  uint64_t byte2_matches = __insn_v1cmpeq (v, byte2);
	  goal_matches = byte1_matches & STRSHIFT (byte2_matches, 8);
	  break;
	}
      else
	{
	  /* This is not the last vector, so load the next vector now.
	     And compare byte2 to the 8-bytes starting 1 byte shifted from v,
	     which goes 1-byte into the next vector.  */
	  uint64_t v2 = vec_load (p + 1);
	  if (byte1_matches)
	    {
	      /* 8-bytes starting 1 byte into v.  */
	      v = __insn_dblalign (v, v2, (void*)1);
	      uint64_t byte2_matches_shifted = __insn_v1cmpeq (v, byte2);
	      goal_matches = byte1_matches & byte2_matches_shifted;
	      if (__builtin_expect (goal_matches != 0, 0))
		break;
	    }
	  __insn_prefetch (p + 4);
	  /* Move to next vector.  */
	  v = v2;
	  p++;
	}
    }

  z = CFZ (zero_matches);
  g = CFZ (goal_matches);

  /* If we found the match before '\0' we got a true match. Note that
     if c == '\0' then g == z, and we correctly return the address of
     the '\0' rather than NULL.  */
  return (g <= z) ? ((char *) p) + (g >> 3) : NULL;
}

/* Scan for NEEDLE, using the first two characters as a filter.  */
static char *
STRSTR_SCAN (const char *haystack, const char *needle,
	     unsigned int needle_len)
{
  char *match;
  while (1)
    {
      match = STRSTR2 (haystack, needle);
      if (match == NULL)
	return NULL;
      /* Found first two characters of needle, check for remainder.  */
      if (CMP_FUNC (match + 2, needle + 2, needle_len - 2) == 0)
	return match;
      /* Move past the previous match. Could be +2 instead of +1 if
         first two characters are different, but that tested slower.  */
      haystack = match + 1;
    }
}

/* Return the first occurrence of NEEDLE in HAYSTACK.  Return HAYSTACK
   if NEEDLE is empty, otherwise NULL if NEEDLE is not found in
   HAYSTACK.  */
char *
STRSTR (const char *haystack_start, const char *needle_start)
{
  const char *haystack = haystack_start;
  const char *needle = needle_start;
  __insn_prefetch (haystack);
  size_t needle_len = strlen (needle_start); /* Length of NEEDLE.  */
  size_t haystack_len; /* Known minimum length of HAYSTACK.  */

  if (needle_len <= 2)
    {
      if (needle_len == 1)
	return STRCHR (haystack_start, *needle_start);
      if (needle_len == 0)
	return (char *) haystack_start;
      else
	return STRSTR2 (haystack_start, needle_start);
    }

  /* Fail if NEEDLE is longer than HAYSTACK.  */
  if (__strnlen (haystack, needle_len) < needle_len)
    return NULL;

  /* Perform the search.  Abstract memory is considered to be an array
     of 'unsigned char' values, not an array of 'char' values.  See
     ISO C 99 section 6.2.6.1.  */
  if (needle_len < 40)
    return STRSTR_SCAN (haystack_start, needle_start, needle_len);
  else
    {
      /* Reduce the size of haystack using STRSTR2, since it has a smaller
	 linear coefficient than the Two-Way algorithm.  */
      haystack = STRSTR2 (haystack_start, needle_start);
      if (haystack == NULL)
	return NULL;
      needle = needle_start;
      haystack_len = (haystack > haystack_start + needle_len ? 1
		      : needle_len + haystack_start - haystack);

      return two_way_long_needle ((const unsigned char *) haystack,
				  haystack_len,
				  (const unsigned char *) needle, needle_len);
    }
}
#ifndef USE_AS_STRCASESTR
libc_hidden_builtin_def (STRSTR)
#endif

#undef LONG_NEEDLE_THRESHOLD
