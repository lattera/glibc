/* Compare two memory blocks for differences in the first COUNT bytes.
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sysdep.h>
#include "asm-syntax.h"

/*
   INPUT PARAMETERS:
   block1	(sp + 4)
   block2	(sp + 8)
   len		(sp + 12)
*/

	.text
ENTRY (memcmp)
	pushl %esi		/* Save callee-safe registers.  */
	movl %edi, %edx		/* Note that %edx is not used and can
				   so be used to save %edi.  It's faster.  */

	movl 8(%esp), %esi	/* Load address of block #1.  */
	movl 12(%esp), %edi	/* Load address of block #2.  */
	movl 16(%esp), %ecx	/* Load maximal length of compare area.  */

	cld			/* Set direction of comparison.  */

	xorl %eax, %eax		/* Default result.  */

	repe			/* Compare at most %ecx bytes.  */
	cmpsb
	jz L(1)			/* If even last byte was equal we return 0.  */

	/* The memory blocks are not equal.  So result of the last
	   subtraction is present in the carry flag.  It is set when
	   the byte in block #2 is bigger.  In this case we have to
	   return -1 (=0xffffffff), else 1.  */
	sbbl %eax, %eax		/* This is tricky.  %eax == 0 and carry is set
				   or not depending on last subtraction.  */

	/* At this point %eax == 0, if the byte of block #1 was bigger, and
	   0xffffffff if the last byte of block #2 was bigger.  The latter
	   case is already correct but the former needs a little adjustment.
	   Note that the following operation does not change 0xffffffff.  */
	orb $1, %al		/* Change 0 to 1.  */

L(1):	popl %esi		/* Restore registers.  */
	movl %edx, %edi

	ret
END (memcmp)

#undef bcmp
weak_alias (memcmp, bcmp)
