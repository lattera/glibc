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
#include <stdlib.h>
#include <arch/chip.h>

/* Must be 8 bytes in size. */
#define word_t uint64_t

/* How many cache lines ahead should we prefetch? */
#define PREFETCH_LINES_AHEAD 3

void *
__memcpy (void *__restrict dstv, const void *__restrict srcv, size_t n)
{
  char *__restrict dst1 = (char *) dstv;
  const char *__restrict src1 = (const char *) srcv;
  const char *__restrict src1_end;
  const char *__restrict prefetch;
  word_t *__restrict dst8; /* 8-byte pointer to destination memory. */
  word_t final; /* Final bytes to write to trailing word, if any */
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
      prefetch = (prefetch > src1_end) ? prefetch : src1;
    }

  /* Copy bytes until dst is word-aligned. */
  for (; (uintptr_t) dst1 & (sizeof (word_t) - 1); n--)
    *dst1++ = *src1++;

  /* 8-byte pointer to destination memory. */
  dst8 = (word_t *) dst1;

  if (__builtin_expect ((uintptr_t) src1 & (sizeof (word_t) - 1), 0))
    {
      /* Misaligned copy.  Copy 8 bytes at a time, but don't bother
         with other fanciness.
         TODO: Consider prefetching and using wh64 as well.  */

      /* Create an aligned src8. */
      const word_t *__restrict src8 =
        (const word_t *) ((uintptr_t) src1 & -sizeof (word_t));
      word_t b;

      word_t a = *src8++;
      for (; n >= sizeof (word_t); n -= sizeof (word_t))
        {
          b = *src8++;
          a = __insn_dblalign (a, b, src1);
          *dst8++ = a;
          a = b;
        }

      if (n == 0)
        return dstv;

      b = ((const char *) src8 <= src1_end) ? *src8 : 0;

      /* Final source bytes to write to trailing partial word, if any. */
      final = __insn_dblalign (a, b, src1);
    }
  else
    {
      /* Aligned copy. */

      const word_t *__restrict src8 = (const word_t *) src1;

      /* src8 and dst8 are both word-aligned. */
      if (n >= CHIP_L2_LINE_SIZE ())
        {
          /* Copy until 'dst' is cache-line-aligned. */
          for (; (uintptr_t) dst8 & (CHIP_L2_LINE_SIZE () - 1);
               n -= sizeof (word_t))
            *dst8++ = *src8++;

          for (; n >= CHIP_L2_LINE_SIZE ();)
            {
              __insn_wh64 (dst8);

              /* Prefetch and advance to next line to prefetch, but
                 don't go past the end.  */
              __insn_prefetch (prefetch);
              prefetch += CHIP_L2_LINE_SIZE ();
              prefetch = (prefetch > src1_end) ? prefetch :
                (const char *) src8;

              /* Copy an entire cache line.  Manually unrolled to
                 avoid idiosyncracies of compiler unrolling.  */
#define COPY_WORD(offset) ({ dst8[offset] = src8[offset]; n -= 8; })
              COPY_WORD (0);
              COPY_WORD (1);
              COPY_WORD (2);
              COPY_WORD (3);
              COPY_WORD (4);
              COPY_WORD (5);
              COPY_WORD (6);
              COPY_WORD (7);
#if CHIP_L2_LINE_SIZE() != 64
# error "Fix code that assumes particular L2 cache line size."
#endif

              dst8 += CHIP_L2_LINE_SIZE () / sizeof (word_t);
              src8 += CHIP_L2_LINE_SIZE () / sizeof (word_t);
            }
        }

      for (; n >= sizeof (word_t); n -= sizeof (word_t))
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
