/* wordcopy.c -- subroutines for memory copy functions.  Tile version.
   Copyright (C) 1991-2016 Free Software Foundation, Inc.
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

/* To optimize for tile, we make the following changes from the
   default glibc version:
   - Use the double align instruction instead of the MERGE macro.
   - Since we don't have offset addressing mode, make sure the loads /
     stores in the inner loop always have indices of 0.
   - Use post-increment addresses in the inner loops, which yields
     better scheduling.  */

/* BE VERY CAREFUL IF YOU CHANGE THIS CODE...!  */

#include <stddef.h>
#include <memcopy.h>

/* Provide the appropriate dblalign builtin to shift two registers
   based on the alignment of a pointer held in a third register.  */
#ifdef __tilegx__
#define DBLALIGN __insn_dblalign
#else
#define DBLALIGN __insn_dword_align
#endif

/* _wordcopy_fwd_aligned -- Copy block beginning at SRCP to
   block beginning at DSTP with LEN `op_t' words (not LEN bytes!).
   Both SRCP and DSTP should be aligned for memory operations on `op_t's.  */

void
_wordcopy_fwd_aligned (long int dstp, long int srcp, size_t len)
{
  op_t a0, a1;

  switch (len % 8)
    {
    case 2:
      a0 = ((op_t *) srcp)[0];
      srcp += OPSIZ;
      len += 6;
      goto do1;
    case 3:
      a1 = ((op_t *) srcp)[0];
      srcp += OPSIZ;
      len += 5;
      goto do2;
    case 4:
      a0 = ((op_t *) srcp)[0];
      srcp += OPSIZ;
      len += 4;
      goto do3;
    case 5:
      a1 = ((op_t *) srcp)[0];
      srcp += OPSIZ;
      len += 3;
      goto do4;
    case 6:
      a0 = ((op_t *) srcp)[0];
      srcp += OPSIZ;
      len += 2;
      goto do5;
    case 7:
      a1 = ((op_t *) srcp)[0];
      srcp += OPSIZ;
      len += 1;
      goto do6;

    case 0:
      if (OP_T_THRES <= 3 * OPSIZ && len == 0)
	return;
      a0 = ((op_t *) srcp)[0];
      srcp += OPSIZ;
      goto do7;
    case 1:
      a1 = ((op_t *) srcp)[0];
      srcp += OPSIZ;
      len -= 1;
      if (OP_T_THRES <= 3 * OPSIZ && len == 0)
	goto do0;
      goto do8;			/* No-op.  */
    }

  do
    {
    do8:
      a0 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[0] = a1;
      srcp += OPSIZ;
      dstp += OPSIZ;
    do7:
      a1 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[0] = a0;
      srcp += OPSIZ;
      dstp += OPSIZ;
    do6:
      a0 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[0] = a1;
      srcp += OPSIZ;
      dstp += OPSIZ;
    do5:
      a1 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[0] = a0;
      srcp += OPSIZ;
      dstp += OPSIZ;
    do4:
      a0 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[0] = a1;
      srcp += OPSIZ;
      dstp += OPSIZ;
    do3:
      a1 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[0] = a0;
      srcp += OPSIZ;
      dstp += OPSIZ;
    do2:
      a0 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[0] = a1;
      srcp += OPSIZ;
      dstp += OPSIZ;
    do1:
      a1 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[0] = a0;
      srcp += OPSIZ;
      dstp += OPSIZ;

      len -= 8;
    }
  while (len != 0);

  /* This is the right position for do0.  Please don't move
     it into the loop.  */
 do0:
  ((op_t *) dstp)[0] = a1;
}

/* _wordcopy_fwd_dest_aligned -- Copy block beginning at SRCP to
   block beginning at DSTP with LEN `op_t' words (not LEN bytes!).
   DSTP should be aligned for memory operations on `op_t's, but SRCP must
   *not* be aligned.  */

void
_wordcopy_fwd_dest_aligned (long int dstp, long int srcp, size_t len)
{
  void * srci;
  op_t a0, a1, a2, a3;

  /* Save the initial source pointer so we know the number of bytes to
     shift for merging two unaligned results.  */
  srci = (void *) srcp;

  /* Make SRCP aligned by rounding it down to the beginning of the `op_t'
     it points in the middle of.  */
  srcp &= -OPSIZ;

  switch (len % 4)
    {
    case 2:
      a1 = ((op_t *) srcp)[0];
      a2 = ((op_t *) srcp)[1];
      len += 2;
      srcp += 2 * OPSIZ;
      goto do1;
    case 3:
      a0 = ((op_t *) srcp)[0];
      a1 = ((op_t *) srcp)[1];
      len += 1;
      srcp += 2 * OPSIZ;
      goto do2;
    case 0:
      if (OP_T_THRES <= 3 * OPSIZ && len == 0)
	return;
      a3 = ((op_t *) srcp)[0];
      a0 = ((op_t *) srcp)[1];
      len += 0;
      srcp += 2 * OPSIZ;
      goto do3;
    case 1:
      a2 = ((op_t *) srcp)[0];
      a3 = ((op_t *) srcp)[1];
      srcp += 2 * OPSIZ;
      len -= 1;
      if (OP_T_THRES <= 3 * OPSIZ && len == 0)
	goto do0;
      goto do4;			/* No-op.  */
    }

  do
    {
    do4:
      a0 = ((op_t *) srcp)[0];
      a2 = DBLALIGN (a2, a3, srci);
      ((op_t *) dstp)[0] = a2;
      srcp += OPSIZ;
      dstp += OPSIZ;
    do3:
      a1 = ((op_t *) srcp)[0];
      a3 = DBLALIGN (a3, a0, srci);
      ((op_t *) dstp)[0] = a3;
      srcp += OPSIZ;
      dstp += OPSIZ;
    do2:
      a2 = ((op_t *) srcp)[0];
      a0 = DBLALIGN (a0, a1, srci);
      ((op_t *) dstp)[0] = a0;
      srcp += OPSIZ;
      dstp += OPSIZ;
    do1:
      a3 = ((op_t *) srcp)[0];
      a1 = DBLALIGN (a1, a2, srci);
      ((op_t *) dstp)[0] = a1;
      srcp += OPSIZ;
      dstp += OPSIZ;
      len -= 4;
    }
  while (len != 0);

  /* This is the right position for do0.  Please don't move
     it into the loop.  */
 do0:
  ((op_t *) dstp)[0] = DBLALIGN (a2, a3, srci);
}

/* _wordcopy_bwd_aligned -- Copy block finishing right before
   SRCP to block finishing right before DSTP with LEN `op_t' words
   (not LEN bytes!).  Both SRCP and DSTP should be aligned for memory
   operations on `op_t's.  */

void
_wordcopy_bwd_aligned (long int dstp, long int srcp, size_t len)
{
  op_t a0, a1;
  long int srcp1;

  srcp1 = srcp - 1 * OPSIZ;
  srcp -= 2 * OPSIZ;
  dstp -= 1 * OPSIZ;

  switch (len % 8)
    {
    case 2:
      a0 = ((op_t *) srcp1)[0];
      len += 6;
      goto do1;
    case 3:
      a1 = ((op_t *) srcp1)[0];
      len += 5;
      goto do2;
    case 4:
      a0 = ((op_t *) srcp1)[0];
      len += 4;
      goto do3;
    case 5:
      a1 = ((op_t *) srcp1)[0];
      len += 3;
      goto do4;
    case 6:
      a0 = ((op_t *) srcp1)[0];
      len += 2;
      goto do5;
    case 7:
      a1 = ((op_t *) srcp1)[0];
      len += 1;
      goto do6;

    case 0:
      if (OP_T_THRES <= 3 * OPSIZ && len == 0)
	return;
      a0 = ((op_t *) srcp1)[0];
      goto do7;
    case 1:
      a1 = ((op_t *) srcp1)[0];
      len -= 1;
      if (OP_T_THRES <= 3 * OPSIZ && len == 0)
	goto do0;
      goto do8;			/* No-op.  */
    }

  do
    {
    do8:
      a0 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[0] = a1;
      srcp -= OPSIZ;
      dstp -= OPSIZ;
    do7:
      a1 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[0] = a0;
      srcp -= OPSIZ;
      dstp -= OPSIZ;
    do6:
      a0 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[0] = a1;
      srcp -= OPSIZ;
      dstp -= OPSIZ;
    do5:
      a1 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[0] = a0;
      srcp -= OPSIZ;
      dstp -= OPSIZ;
    do4:
      a0 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[0] = a1;
      srcp -= OPSIZ;
      dstp -= OPSIZ;
    do3:
      a1 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[0] = a0;
      srcp -= OPSIZ;
      dstp -= OPSIZ;
    do2:
      a0 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[0] = a1;
      srcp -= OPSIZ;
      dstp -= OPSIZ;
    do1:
      a1 = ((op_t *) srcp)[0];
      ((op_t *) dstp)[0] = a0;
      srcp -= OPSIZ;
      dstp -= OPSIZ;

      len -= 8;
    }
  while (len != 0);

  /* This is the right position for do0.  Please don't move
     it into the loop.  */
 do0:
  ((op_t *) dstp)[0] = a1;
}

/* _wordcopy_bwd_dest_aligned -- Copy block finishing right
   before SRCP to block finishing right before DSTP with LEN `op_t'
   words (not LEN bytes!).  DSTP should be aligned for memory
   operations on `op_t', but SRCP must *not* be aligned.  */

void
_wordcopy_bwd_dest_aligned (long int dstp, long int srcp, size_t len)
{
  void * srci;
  op_t a0, a1, a2, a3;
  op_t b0, b1, b2, b3;

  /* Save the initial source pointer so we know the number of bytes to
     shift for merging two unaligned results.  */
  srci = (void *) srcp;

  /* Make SRCP aligned by rounding it down to the beginning of the op_t
     it points in the middle of.  */
  srcp &= -OPSIZ;
  srcp += OPSIZ;

  switch (len % 4)
    {
    case 2:
      srcp -= 3 * OPSIZ;
      dstp -= 1 * OPSIZ;
      b2 = ((op_t *) srcp)[2];
      b1 = a1 = ((op_t *) srcp)[1];
      len += 2;
      goto do1;
    case 3:
      srcp -= 3 * OPSIZ;
      dstp -= 1 * OPSIZ;
      b3 = ((op_t *) srcp)[2];
      b2 = a2 = ((op_t *) srcp)[1];
      len += 1;
      goto do2;
    case 0:
      if (OP_T_THRES <= 3 * OPSIZ && len == 0)
	return;
      srcp -= 3 * OPSIZ;
      dstp -= 1 * OPSIZ;
      b0 = ((op_t *) srcp)[2];
      b3 = a3 = ((op_t *) srcp)[1];
      goto do3;
    case 1:
      srcp -= 3 * OPSIZ;
      dstp -= 1 * OPSIZ;
      b1 = ((op_t *) srcp)[2];
      b0 = a0 = ((op_t *) srcp)[1];
      len -= 1;
      if (OP_T_THRES <= 3 * OPSIZ && len == 0)
	goto do0;
      goto do4;			/* No-op.  */
    }

  do
    {
    do4:
      b3 = a3 = ((op_t *) srcp)[0];
      a0 = DBLALIGN (a0, b1, srci);
      ((op_t *) dstp)[0] = a0;
      srcp -= OPSIZ;
      dstp -= OPSIZ;
    do3:
      b2 = a2 = ((op_t *) srcp)[0];
      a3 = DBLALIGN (a3, b0, srci);
      ((op_t *) dstp)[0] = a3;
      srcp -= OPSIZ;
      dstp -= OPSIZ;
    do2:
      b1 = a1 = ((op_t *) srcp)[0];
      a2 = DBLALIGN (a2, b3, srci);
      ((op_t *) dstp)[0] = a2;
      srcp -= OPSIZ;
      dstp -= OPSIZ;
    do1:
      b0 = a0 = ((op_t *) srcp)[0];
      a1 = DBLALIGN (a1, b2, srci);
      ((op_t *) dstp)[0] = a1;
      srcp -= OPSIZ;
      dstp -= OPSIZ;

      len -= 4;
    }
  while (len != 0);

  /* This is the right position for do0.  Please don't move
     it into the loop.  */
 do0:
  a0 = DBLALIGN (a0, b1, srci);
  ((op_t *) dstp)[0] = a0;
}
