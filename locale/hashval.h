/* Implement simple hashing table with string based keys.
   Copyright (C) 1994-1997, 2000, 2001, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>, October 1994.

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

#ifndef	LONGBITS
# define LONGBITS (sizeof (long int) * BITSPERBYTE)
#endif

unsigned long int compute_hashval (const void *key, size_t keylen);

unsigned long int
compute_hashval (key, keylen)
     const void *key;
     size_t keylen;
{
  size_t cnt;
  unsigned long int hval;

  /* Compute the hash value for the given string.  The algorithm
     is taken from [Aho,Sethi,Ullman], modified to reduce the number of
     collisions for short strings with very varied bit patterns.
     See http://www.clisp.org/haible/hashfunc.html.  */
  cnt = 0;
  hval = keylen;
  while (cnt < keylen)
    {
      hval = (hval << 9) | (hval >> (LONGBITS - 9));
      hval += (unsigned long int) *(((char *) key) + cnt++);
    }
  return hval != 0 ? hval : ~((unsigned long int) 0);
}
