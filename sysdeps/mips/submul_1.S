/* MIPS __mpn_submul_1 -- Multiply a limb vector with a single limb and
subtract the product from a second limb vector.

Copyright (C) 1995, 2000, 2002, 2003 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
MA 02111-1307, USA.  */

#include <sysdep.h>

/* INPUT PARAMETERS
   res_ptr	$4
   s1_ptr	$5
   size		$6
   s2_limb	$7
*/
#ifdef __PIC__
	.option pic2
#endif
ENTRY (__mpn_submul_1)
	.set    noreorder
#ifdef __PIC__
	.cpload t9
#endif
	.set    nomacro

	/* warm up phase 0 */
	lw	$8,0($5)

	/* warm up phase 1 */
	addiu	$5,$5,4
	multu	$8,$7

	addiu	$6,$6,-1
	beq	$6,$0,L(LC0)
	move	$2,$0		/* zero cy2 */

	addiu	$6,$6,-1
	beq	$6,$0,L(LC1)
	lw	$8,0($5)	/* load new s1 limb as early as possible */

L(Loop):	lw	$10,0($4)
	mflo	$3
	mfhi	$9
	addiu	$5,$5,4
	addu	$3,$3,$2	/* add old carry limb to low product limb */
	multu	$8,$7
	lw	$8,0($5)	/* load new s1 limb as early as possible */
	addiu	$6,$6,-1	/* decrement loop counter */
	sltu	$2,$3,$2	/* carry from previous addition -> $2 */
	subu	$3,$10,$3
	sgtu	$10,$3,$10
	addu	$2,$2,$10
	sw	$3,0($4)
	addiu	$4,$4,4
	bne	$6,$0,L(Loop)	/* should be "bnel" */
	addu	$2,$9,$2	/* add high product limb and carry from addition */

	/* cool down phase 1 */
L(LC1):	lw	$10,0($4)
	mflo	$3
	mfhi	$9
	addu	$3,$3,$2
	sltu	$2,$3,$2
	multu	$8,$7
	subu	$3,$10,$3
	sgtu	$10,$3,$10
	addu	$2,$2,$10
	sw	$3,0($4)
	addiu	$4,$4,4
	addu	$2,$9,$2	/* add high product limb and carry from addition */

	/* cool down phase 0 */
L(LC0):	lw	$10,0($4)
	mflo	$3
	mfhi	$9
	addu	$3,$3,$2
	sltu	$2,$3,$2
	subu	$3,$10,$3
	sgtu	$10,$3,$10
	addu	$2,$2,$10
	sw	$3,0($4)
	j	$31
	addu	$2,$9,$2	/* add high product limb and carry from addition */
	END (__mpn_submul_1)
