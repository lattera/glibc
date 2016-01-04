/* Copyright (C) 2011-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <string.h>
#include <stdint.h>
#include "string-endian.h"

void *
__memchr (const void *s, int c, size_t n)
{
  const uint64_t *last_word_ptr;
  const uint64_t *p;
  const char *last_byte_ptr;
  uintptr_t s_int;
  uint64_t goal, before_mask, v, bits;
  char *ret;

  if (__builtin_expect (n == 0, 0))
    {
      /* Don't dereference any memory if the array is empty. */
      return NULL;
    }

  /* Get an aligned pointer. */
  s_int = (uintptr_t) s;
  p = (const uint64_t *) (s_int & -8);

  /* Create eight copies of the byte for which we are looking. */
  goal = copy_byte(c);

  /* Read the first word, but munge it so that bytes before the array
     will not match goal.  */
  before_mask = MASK (s_int);
  v = (*p | before_mask) ^ (goal & before_mask);

  /* Compute the address of the last byte. */
  last_byte_ptr = (const char *) s + n - 1;

  /* Compute the address of the word containing the last byte. */
  last_word_ptr = (const uint64_t *) ((uintptr_t) last_byte_ptr & -8);

  while ((bits = __insn_v1cmpeq (v, goal)) == 0)
    {
      if (__builtin_expect (p == last_word_ptr, 0))
        {
          /* We already read the last word in the array, so give up.  */
          return NULL;
        }
      v = *++p;
    }

  /* We found a match, but it might be in a byte past the end
     of the array.  */
  ret = ((char *) p) + (CFZ (bits) >> 3);
  return (ret <= last_byte_ptr) ? ret : NULL;
}
weak_alias (__memchr, memchr)
libc_hidden_builtin_def (memchr)
