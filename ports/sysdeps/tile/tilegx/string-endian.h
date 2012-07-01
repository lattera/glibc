/* Copyright (C) 2011-2012 Free Software Foundation, Inc.
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

/* Provide a mask based on the pointer alignment that
   sets up non-zero bytes before the beginning of the string.
   The MASK expression works because shift counts are taken mod 64.
   Also, specify how to count "first" and "last" bits
   when the bits have been read as a word.  */

#ifndef __BIG_ENDIAN__
#define MASK(x) (__insn_shl(1ULL, (x << 3)) - 1)
#define NULMASK(x) ((2ULL << x) - 1)
#define CFZ(x) __insn_ctz(x)
#define REVCZ(x) __insn_clz(x)
#else
#define MASK(x) (__insn_shl(-2LL, ((-x << 3) - 1)))
#define NULMASK(x) (-2LL << (63 - x))
#define CFZ(x) __insn_clz(x)
#define REVCZ(x) __insn_ctz(x)
#endif

/* Create eight copies of the byte in a uint64_t. */
static inline uint64_t copy_byte(uint8_t byte)
{
  uint64_t word = byte;
  word = __insn_bfins(word, word, 8, 15);
  word = __insn_bfins(word, word, 16, 31);
  word = __insn_bfins(word, word, 32, 63);
  return word;
}
