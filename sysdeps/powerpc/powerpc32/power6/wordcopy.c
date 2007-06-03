/* _memcopy.c -- subroutines for memory copy functions.
   Copyright (C) 1991, 1996, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Torbjorn Granlund (tege@sics.se).
   Updated for POWER6 by Steven Munroe (sjmunroe@us.ibm.com).

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

/* BE VERY CAREFUL IF YOU CHANGE THIS CODE...!  */

#include <stddef.h>
#include <memcopy.h>

/* _wordcopy_fwd_aligned -- Copy block beginning at SRCP to
   block beginning at DSTP with LEN `op_t' words (not LEN bytes!).
   Both SRCP and DSTP should be aligned for memory operations on `op_t's.  */

void
_wordcopy_fwd_aligned (dstp, srcp, len)
     long int dstp;
     long int srcp;
     size_t len;
{
  op_t a0, a1;

  if (len & 1)
  {
    ((op_t *) dstp)[0] = ((op_t *) srcp)[0];
    
    if (len == 1)
      return;
    srcp += OPSIZ;
    dstp += OPSIZ;
    len -= 1;
  }

  do
    {
      a0 = ((op_t *) srcp)[0];
      a1 = ((op_t *) srcp)[1];
      ((op_t *) dstp)[0] = a0;
      ((op_t *) dstp)[1] = a1;

      srcp += 2 * OPSIZ;
      dstp += 2 * OPSIZ;
      len -= 2;
    }
  while (len != 0);
}

/* _wordcopy_fwd_dest_aligned -- Copy block beginning at SRCP to
   block beginning at DSTP with LEN `op_t' words (not LEN bytes!).
   DSTP should be aligned for memory operations on `op_t's, but SRCP must
   *not* be aligned.  */

void
_wordcopy_fwd_dest_aligned (dstp, srcp, len)
     long int dstp;
     long int srcp;
     size_t len;
{
  op_t a0, a1, a2;
  int sh_1, sh_2;
  int align;

  /* Calculate how to shift a word read at the memory operation
     aligned srcp to make it aligned for copy.  */

  align = srcp % OPSIZ;
  sh_1 = 8 * (srcp % OPSIZ);
  sh_2 = 8 * OPSIZ - sh_1;

  /* Make SRCP aligned by rounding it down to the beginning of the `op_t'
     it points in the middle of.  */
  srcp &= -OPSIZ;
  a0 = ((op_t *) srcp)[0];

  if (len & 1)
  {
    a1 = ((op_t *) srcp)[1];
    ((op_t *) dstp)[0] = MERGE (a0, sh_1, a1, sh_2);
    
    if (len == 1)
      return;
    
    a0 = a1;
    srcp += OPSIZ;
    dstp += OPSIZ;
    len -= 1;
  }

  switch (align)
    {
    case 1:
      do
        {
          a1 = ((op_t *) srcp)[1];
          a2 = ((op_t *) srcp)[2];
          ((op_t *) dstp)[0] = MERGE (a0, 8, a1, (32-8));
          ((op_t *) dstp)[1] = MERGE (a1, 8, a2, (32-8));
          a0 = a2;
    
          srcp += 2 * OPSIZ;
          dstp += 2 * OPSIZ;
          len -= 2;
        }
      while (len != 0);
      break;
    case 2:
      do
        {
          a1 = ((op_t *) srcp)[1];
          a2 = ((op_t *) srcp)[2];
          ((op_t *) dstp)[0] = MERGE (a0, 16, a1, (32-16));
          ((op_t *) dstp)[1] = MERGE (a1, 16, a2, (32-16));
          a0 = a2;
    
          srcp += 2 * OPSIZ;
          dstp += 2 * OPSIZ;
          len -= 2;
        }
      while (len != 0);
      break;
    case 3:
      do
        {
          a1 = ((op_t *) srcp)[1];
          a2 = ((op_t *) srcp)[2];
          ((op_t *) dstp)[0] = MERGE (a0, 24, a1, (32-24));
          ((op_t *) dstp)[1] = MERGE (a1, 24, a2, (32-24));
          a0 = a2;
    
          srcp += 2 * OPSIZ;
          dstp += 2 * OPSIZ;
          len -= 2;
        }
      while (len != 0);
      break;
    }

}

/* _wordcopy_bwd_aligned -- Copy block finishing right before
   SRCP to block finishing right before DSTP with LEN `op_t' words
   (not LEN bytes!).  Both SRCP and DSTP should be aligned for memory
   operations on `op_t's.  */

void
_wordcopy_bwd_aligned (dstp, srcp, len)
     long int dstp;
     long int srcp;
     size_t len;
{
  op_t a0, a1;

  if (len & 1)
  {
    srcp -= OPSIZ;
    dstp -= OPSIZ;
    ((op_t *) dstp)[0] = ((op_t *) srcp)[0];
    
    if (len == 1)
      return;
    len -= 1;
  }

  do
    {
      srcp -= 2 * OPSIZ;
      dstp -= 2 * OPSIZ;

      a1 = ((op_t *) srcp)[1];
      a0 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[1] = a1;
      ((op_t *) dstp)[0] = a0;

      len -= 2;
    }
  while (len != 0);
}

/* _wordcopy_bwd_dest_aligned -- Copy block finishing right
   before SRCP to block finishing right before DSTP with LEN `op_t'
   words (not LEN bytes!).  DSTP should be aligned for memory
   operations on `op_t', but SRCP must *not* be aligned.  */

void
_wordcopy_bwd_dest_aligned (dstp, srcp, len)
     long int dstp;
     long int srcp;
     size_t len;
{
  op_t a0, a1, a2;
  int sh_1, sh_2;
  int align;

  /* Calculate how to shift a word read at the memory operation
     aligned srcp to make it aligned for copy.  */

  align = srcp % OPSIZ;
  sh_1 = 8 * (srcp % OPSIZ);
  sh_2 = 8 * OPSIZ - sh_1;

  /* Make srcp aligned by rounding it down to the beginning of the op_t
     it points in the middle of.  */
  srcp &= -OPSIZ;
  a2 = ((op_t *) srcp)[0];

  if (len & 1)
  {
    srcp -= OPSIZ;
    dstp -= OPSIZ;
    a1 = ((op_t *) srcp)[0];
    ((op_t *) dstp)[0] = MERGE (a1, sh_1, a2, sh_2);

    if (len == 1)
      return;

    a2 = a1;
    len -= 1;
  }

  switch (align)
    {
    case 1:
      do
        {
          srcp -= 2 * OPSIZ;
          dstp -= 2 * OPSIZ;
    
          a1 = ((op_t *) srcp)[1];
          a0 = ((op_t *) srcp)[0];
          ((op_t *) dstp)[1] = MERGE (a1, 8, a2, (32-8));
          ((op_t *) dstp)[0] = MERGE (a0, 8, a1, (32-8));
          a2 = a0;
    
          len -= 2;
        }
      while (len != 0);
      break;
    case 2:
      do
        {
          srcp -= 2 * OPSIZ;
          dstp -= 2 * OPSIZ;
    
          a1 = ((op_t *) srcp)[1];
          a0 = ((op_t *) srcp)[0];
          ((op_t *) dstp)[1] = MERGE (a1, 16, a2, (32-16));
          ((op_t *) dstp)[0] = MERGE (a0, 16, a1, (32-16));
          a2 = a0;
    
          len -= 2;
        }
      while (len != 0);
      break;
    case 3:
      do
        {
          srcp -= 2 * OPSIZ;
          dstp -= 2 * OPSIZ;
    
          a1 = ((op_t *) srcp)[1];
          a0 = ((op_t *) srcp)[0];
          ((op_t *) dstp)[1] = MERGE (a1, 24, a2, (32-24));
          ((op_t *) dstp)[0] = MERGE (a0, 24, a1, (32-24));
          a2 = a0;
    
          len -= 2;
        }
      while (len != 0);
      break;
    }
}
