/* Copyright (C) 1991, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Torbjorn Granlund (tege@sics.se).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <string.h>
#include <memcopy.h>

/* Set N bytes of S to 0.  */
void
bzero (s, len)
     void *s;
     size_t len;
{
  long int dstp = (long int) s;
  const op_t zero = 0;

  if (len >= 8)
    {
      size_t xlen;

      /* There are at least some bytes to zero.  No need to test
	 for LEN == 0 in this alignment loop.  */
      while (dstp % OPSIZ != 0)
	{
	  ((byte *) dstp)[0] = 0;
	  dstp += 1;
	  len -= 1;
	}

      /* Write 8 op_t per iteration until less than 8 op_t remain.  */
      xlen = len / (OPSIZ * 8);
      while (xlen != 0)
	{
	  ((op_t *) dstp)[0] = zero;
	  ((op_t *) dstp)[1] = zero;
	  ((op_t *) dstp)[2] = zero;
	  ((op_t *) dstp)[3] = zero;
	  ((op_t *) dstp)[4] = zero;
	  ((op_t *) dstp)[5] = zero;
	  ((op_t *) dstp)[6] = zero;
	  ((op_t *) dstp)[7] = zero;
	  dstp += 8 * OPSIZ;
	  xlen -= 1;
	}
      len %= OPSIZ * 8;

      /* Write 1 op_t per iteration until less than op_t remain.  */
      xlen = len / OPSIZ;
      while (xlen != 0)
	{
	  ((op_t *) dstp)[0] = zero;
	  dstp += OPSIZ;
	  xlen -= 1;
	}
      len %= OPSIZ;
    }

  /* Write the last few bytes.  */
  while (len != 0)
    {
      ((byte *) dstp)[0] = 0;
      dstp += 1;
      len -= 1;
    }
}
