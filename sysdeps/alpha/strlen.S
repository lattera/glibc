/* Copyright (C) 1996 Free Software Foundation, Inc.
   Contributed by David Mosberger (davidm@cs.arizona.edu).

This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* Finds length of a 0-terminated string.  Optimized for the Alpha
architecture:

      - memory accessed as aligned quadwords only
      - uses bcmpge to compare 8 bytes in parallel
      - does binary search to find 0 byte in last
        quadword (HAKMEM needed 12 instructions to
        do this instead of the 9 instructions that
        binary search needs).  */

#include <sysdep.h>

        .set noreorder
        .set noat

ENTRY(strlen)
        ldq_u   t0, 0(a0)       # load first quadword (a0 may be misaligned)
        lda     t1, -1(zero)
        insqh   t1, a0, t1
        andnot  a0, 7, v0
        or      t1, t0, t0
        cmpbge  zero, t0, t1    # t1 <- bitmask: bit i == 1 <==> i-th byte == 0
        bne     t1, found

loop:   ldq     t0, 8(v0)
        addq    v0, 8, v0       # addr += 8
        nop                     # helps dual issue last two insns
        cmpbge  zero, t0, t1
        beq     t1, loop

found:  blbs    t1, done        # make aligned case fast
        negq    t1, t2
        and     t1, t2, t1

        and     t1, 0x0f, t0
        addq    v0, 4, t2
        cmoveq  t0, t2, v0

        and     t1, 0x33, t0
        addq    v0, 2, t2
        cmoveq  t0, t2, v0

        and     t1, 0x55, t0
        addq    v0, 1, t2
        cmoveq  t0, t2, v0

done:   subq    v0, a0, v0
        ret

        .end    strlen
