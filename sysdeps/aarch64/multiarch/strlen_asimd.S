/* Strlen implementation that uses ASIMD instructions for load and NULL checks.
   Copyright (C) 2018 Free Software Foundation, Inc.

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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

/* Assumptions:

   ARMv8-a, AArch64, ASIMD, unaligned accesses, min page size 4k.  */

/* To test the page crossing code path more thoroughly, compile with
   -DTEST_PAGE_CROSS - this will force all calls through the slower
   entry path.  This option is not intended for production use.  */

/* Arguments and results.  */
#define srcin		x0
#define len		x0

/* Locals and temporaries.  */
#define src		x1
#define data1		x2
#define data2		x3
#define has_nul1	x4
#define has_nul2	x5
#define tmp1		x4
#define tmp2		x5
#define tmp3		x6
#define tmp4		x7
#define zeroones	x8
#define dataq		q2
#define datav		v2
#define datab2		b3
#define dataq2		q3
#define datav2		v3

#ifdef TEST_PAGE_CROSS
# define MIN_PAGE_SIZE 16
#else
# define MIN_PAGE_SIZE 4096
#endif

	/* Since strings are short on average, we check the first 16 bytes
	   of the string for a NUL character.  In order to do an unaligned load
	   safely we have to do a page cross check first.  If there is a NUL
	   byte we calculate the length from the 2 8-byte words using
	   conditional select to reduce branch mispredictions (it is unlikely
	   strlen_asimd will be repeatedly called on strings with the same
	   length).

	   If the string is longer than 16 bytes, we align src so don't need
	   further page cross checks, and process 16 bytes per iteration.

	   If the page cross check fails, we read 16 bytes from an aligned
	   address, remove any characters before the string, and continue
	   in the main loop using aligned loads.  Since strings crossing a
	   page in the first 16 bytes are rare (probability of
	   16/MIN_PAGE_SIZE ~= 0.4%), this case does not need to be optimized.

	   AArch64 systems have a minimum page size of 4k.  We don't bother
	   checking for larger page sizes - the cost of setting up the correct
	   page size is just not worth the extra gain from a small reduction in
	   the cases taking the slow path.  Note that we only care about
	   whether the first fetch, which may be misaligned, crosses a page
	   boundary.  */

ENTRY_ALIGN (__strlen_asimd, 6)
	DELOUSE (0)
	DELOUSE (1)
	and	tmp1, srcin, MIN_PAGE_SIZE - 1
	cmp	tmp1, MIN_PAGE_SIZE - 16
	b.gt	L(page_cross)
	ldr	dataq, [srcin]
#ifdef __AARCH64EB__
	rev64	datav.16b, datav.16b
#endif

	/* Get the minimum value and keep going if it is not zero.  */
	uminv	datab2, datav.16b
	mov	tmp1, datav2.d[0]
	cbnz	tmp1, L(main_loop_entry)

	cmeq	datav.16b, datav.16b, #0
	mov	data1, datav.d[0]
	mov	data2, datav.d[1]
	cmp	data1, 0
	csel	data1, data1, data2, ne
	mov	len, 8
	rev	data1, data1
	clz	tmp1, data1
	csel	len, xzr, len, ne
	add	len, len, tmp1, lsr 3
	ret

L(main_loop_entry):
	bic	src, srcin, 15

L(main_loop):
	ldr	dataq, [src, 16]!
L(page_cross_entry):
	/* Get the minimum value and keep going if it is not zero.  */
	uminv	datab2, datav.16b
	mov	tmp1, datav2.d[0]
	cbnz	tmp1, L(main_loop)

L(tail):
#ifdef __AARCH64EB__
	rev64	datav.16b, datav.16b
#endif
	/* Set te NULL byte as 0xff and the rest as 0x00, move the data into a
	   pair of scalars and then compute the length from the earliest NULL
	   byte.  */
	cmeq	datav.16b, datav.16b, #0
	mov	data1, datav.d[0]
	mov	data2, datav.d[1]
	cmp	data1, 0
	csel	data1, data1, data2, ne
	sub	len, src, srcin
	rev	data1, data1
	add	tmp2, len, 8
	clz	tmp1, data1
	csel	len, len, tmp2, ne
	add	len, len, tmp1, lsr 3
	ret

	/* Load 16 bytes from [srcin & ~15] and force the bytes that precede
	   srcin to 0xff, so we ignore any NUL bytes before the string.
	   Then continue in the aligned loop.  */
L(page_cross):
	mov	tmp3, 63
	bic	src, srcin, 15
	and	tmp1, srcin, 7
	ands	tmp2, srcin, 8
	ldr	dataq, [src]
	lsl	tmp1, tmp1, 3
	csel	tmp2, tmp2, tmp1, eq
	csel	tmp1, tmp1, tmp3, eq
	mov	tmp4, -1
#ifdef __AARCH64EB__
	/* Big-endian.  Early bytes are at MSB.  */
	lsr	tmp1, tmp4, tmp1
	lsr	tmp2, tmp4, tmp2
#else
	/* Little-endian.  Early bytes are at LSB.  */
	lsl	tmp1, tmp4, tmp1
	lsl	tmp2, tmp4, tmp2
#endif
	mov	datav2.d[0], tmp1
	mov	datav2.d[1], tmp2
	orn	datav.16b, datav.16b, datav2.16b
	b	L(page_cross_entry)
END (__strlen_asimd)
weak_alias (__strlen_asimd, strlen_asimd)
libc_hidden_builtin_def (strlen_asimd)
