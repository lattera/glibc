/* Copyright (C) 2011-2015 Free Software Foundation, Inc.
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
#include <stdlib.h>
#include <memcopy.h>
#include <arch/chip.h>

/* How many cache lines ahead should we prefetch? */
#define PREFETCH_LINES_AHEAD 3

void * inhibit_loop_to_libcall
__memcpy (void *__restrict dstv, const void *__restrict srcv, size_t n)
{
  char *__restrict dst1 = (char *) dstv;
  const char *__restrict src1 = (const char *) srcv;
  const char *__restrict src1_end;
  const char *__restrict prefetch;
  op_t *__restrict dst8; /* 8-byte pointer to destination memory. */
  op_t final; /* Final bytes to write to trailing word, if any */
  long i;

  if (n < 16)
    {
      for (; n; n--)
        *dst1++ = *src1++;
      return dstv;
    }

  /* Locate the end of source memory we will copy.  Don't prefetch
     past this.  */
  src1_end = src1 + n - 1;

  /* Prefetch ahead a few cache lines, but not past the end. */
  prefetch = src1;
  for (i = 0; i < PREFETCH_LINES_AHEAD; i++)
    {
      __insn_prefetch (prefetch);
      prefetch += CHIP_L2_LINE_SIZE ();
      prefetch = (prefetch < src1_end) ? prefetch : src1;
    }

  /* Copy bytes until dst is word-aligned. */
  for (; (uintptr_t) dst1 & (sizeof (op_t) - 1); n--)
    *dst1++ = *src1++;

  /* 8-byte pointer to destination memory. */
  dst8 = (op_t *) dst1;

  if (__builtin_expect ((uintptr_t) src1 & (sizeof (op_t) - 1), 0))
    {
      /* Misaligned copy.  Use glibc's _wordcopy_fwd_dest_aligned, but
         inline it to avoid prologue/epilogue.  TODO: Consider
         prefetching and using wh64 as well.  */
      void * srci;
      op_t a0, a1, a2, a3;
      long int dstp = (long int) dst1;
      long int srcp = (long int) src1;
      long int len = n / OPSIZ;

      /* Save the initial source pointer so we know the number of
         bytes to shift for merging two unaligned results.  */
      srci = (void *) srcp;

      /* Make SRCP aligned by rounding it down to the beginning of the
         `op_t' it points in the middle of.  */
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
	    return dstv;
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
	  a2 = __insn_dblalign (a2, a3, srci);
	  ((op_t *) dstp)[0] = a2;
	  srcp += OPSIZ;
	  dstp += OPSIZ;
	do3:
	  a1 = ((op_t *) srcp)[0];
	  a3 = __insn_dblalign (a3, a0, srci);
	  ((op_t *) dstp)[0] = a3;
	  srcp += OPSIZ;
	  dstp += OPSIZ;
	do2:
	  a2 = ((op_t *) srcp)[0];
	  a0 = __insn_dblalign (a0, a1, srci);
	  ((op_t *) dstp)[0] = a0;
	  srcp += OPSIZ;
	  dstp += OPSIZ;
	do1:
	  a3 = ((op_t *) srcp)[0];
	  a1 = __insn_dblalign (a1, a2, srci);
	  ((op_t *) dstp)[0] = a1;
	  srcp += OPSIZ;
	  dstp += OPSIZ;
	  len -= 4;
	}
      while (len != 0);

      /* This is the right position for do0.  Please don't move
         it into the loop.  */
    do0:
      ((op_t *) dstp)[0] = __insn_dblalign (a2, a3, srci);

      n = n % OPSIZ;
      if (n == 0)
	return dstv;

      a0 = ((const char *) srcp <= src1_end) ? ((op_t *) srcp)[0] : 0;

      final = __insn_dblalign (a3, a0, srci);
      dst8 = (op_t *)(dstp + OPSIZ);
    }
  else
    {
      /* Aligned copy. */

      const op_t *__restrict src8 = (const op_t *) src1;

      /* src8 and dst8 are both word-aligned. */
      if (n >= CHIP_L2_LINE_SIZE ())
        {
          /* Copy until 'dst' is cache-line-aligned. */
          for (; (uintptr_t) dst8 & (CHIP_L2_LINE_SIZE () - 1);
               n -= sizeof (op_t))
            *dst8++ = *src8++;

          for (; n >= CHIP_L2_LINE_SIZE ();)
	    {
	      op_t tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;

	      /* Prefetch and advance to next line to prefetch, but
		 don't go past the end.  */
	      __insn_prefetch (prefetch);
	      prefetch += CHIP_L2_LINE_SIZE ();
	      prefetch = (prefetch < src1_end) ? prefetch :
		(const char *) src8;

	      /* Do all the loads before wh64.  This is necessary if
		 [src8, src8+7] and [dst8, dst8+7] share the same
		 cache line and dst8 <= src8, as can be the case when
		 called from memmove, or with code tested on x86 whose
		 memcpy always works with forward copies.  */
	      tmp0 = *src8++;
	      tmp1 = *src8++;
	      tmp2 = *src8++;
	      tmp3 = *src8++;
	      tmp4 = *src8++;
	      tmp5 = *src8++;
	      tmp6 = *src8++;
	      tmp7 = *src8++;

	      __insn_wh64 (dst8);

	      *dst8++ = tmp0;
	      *dst8++ = tmp1;
	      *dst8++ = tmp2;
	      *dst8++ = tmp3;
	      *dst8++ = tmp4;
	      *dst8++ = tmp5;
	      *dst8++ = tmp6;
	      *dst8++ = tmp7;

	      n -= 64;
	    }
#if CHIP_L2_LINE_SIZE() != 64
# error "Fix code that assumes particular L2 cache line size."
#endif
        }

      for (; n >= sizeof (op_t); n -= sizeof (op_t))
        *dst8++ = *src8++;

      if (__builtin_expect (n == 0, 1))
        return dstv;

      final = *src8;
    }

  /* n != 0 if we get here.  Write out any trailing bytes. */
  dst1 = (char *) dst8;
#ifndef __BIG_ENDIAN__
  if (n & 4)
    {
      *(uint32_t *) dst1 = final;
      dst1 += 4;
      final >>= 32;
      n &= 3;
    }
  if (n & 2)
    {
      *(uint16_t *) dst1 = final;
      dst1 += 2;
      final >>= 16;
      n &= 1;
    }
  if (n)
    *(uint8_t *) dst1 = final;
#else
  if (n & 4)
    {
      *(uint32_t *) dst1 = final >> 32;
      dst1 += 4;
    }
  else
    {
      final >>= 32;
    }
  if (n & 2)
    {
      *(uint16_t *) dst1 = final >> 16;
      dst1 += 2;
    }
  else
    {
      final >>= 16;
    }
  if (n & 1)
    *(uint8_t *) dst1 = final >> 8;
#endif

  return dstv;
}
weak_alias (__memcpy, memcpy)
libc_hidden_builtin_def (memcpy)
