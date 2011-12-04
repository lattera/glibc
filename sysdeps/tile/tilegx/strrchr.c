/* Copyright (C) 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <string.h>
#include <stdint.h>
#include "string-endian.h"

char *
strrchr (const char *s, int c)
{
  /* Get an aligned pointer. */
  const uintptr_t s_int = (uintptr_t) s;
  const uint64_t *p = (const uint64_t *) (s_int & -8);

  /* Create eight copies of the byte for which we are looking. */
  const uint64_t goal = 0x0101010101010101ULL * (uint8_t) c;

  /* Read the first aligned word, but force bytes before the string to
     match neither zero nor goal (we make sure the high bit of each byte
     is 1, and the low 7 bits are all the opposite of the goal byte).  */
  const uint64_t before_mask = MASK (s_int);
  uint64_t v = (*p | before_mask) ^ (goal & __insn_v1shrui (before_mask, 1));
  const char *found = NULL;
  uint64_t zero_matches, goal_matches;
  while (1)
    {
      /* Look for a terminating '\0'. */
      zero_matches = __insn_v1cmpeqi (v, 0);

      /* Look for the goal byte. */
      goal_matches = __insn_v1cmpeq (v, goal);

      /* If we found the goal, record the last offset. */
      if (__builtin_expect (goal_matches != 0, 0))
        {
          if (__builtin_expect (zero_matches != 0, 0))
            {
              /* Clear any goal after the first zero. */
              int first_nul = CFZ (zero_matches);
              goal_matches &= NULMASK (first_nul);
            }
          if (__builtin_expect (goal_matches != 0, 1))
            found = ((char *) p) + 7 - (REVCZ (goal_matches) >> 3);
        }

      if (__builtin_expect (zero_matches != 0, 0))
        return (char *) found;

      v = *++p;
    }
}
weak_alias (strrchr, rindex)
libc_hidden_builtin_def (strrchr)
