/* FPU control word bits.  Mips version.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Olaf Flebbe.

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

#ifndef _FPU_CONTROL_H
#define _FPU_CONTROL_H

/* FP control/status register bit assignments.
 *
 *     31-25    24  23     22-18       17-12          11-7       6-2     1-0
 *                                    (cause)      (enables)   (flags)
 * | reserved | FS | C | reserved | E V Z O U I | V Z O U I | V Z O U I | RM
 *
 * FS: When set, denormalized results are flushed to zero instead of
 *     causing an unimplemented operation exception.
 * C:  Condition bit.
 * E:  Unimplemented Operation.
 * V:  Invalid Operation.
 * Z:  Division by zero.
 * O:  Overflow.
 * U:  Underflow.
 * I:  Inexact Operation
 * RM: Rounding mode bits
 * 00 (RN) - rounding to nearest
 * 01 (RZ) - rounding toward zero
 * 10 (RP) - rounding down (toward - infinity)
 * 11 (RM) - rounding up (toward + infinity)
 *
 */

#include <features.h>

/* masking of interrupts */
#define _FPU_MASK_IM  (1 << 11)
#define _FPU_MASK_DM  (1 << 24)	/* XXX */
#define _FPU_MASK_ZM  (1 << 10)
#define _FPU_MASK_OM  (1 << 9)
#define _FPU_MASK_UM  (1 << 8)
#define _FPU_MASK_PM  (1 << 7)

/* precision control */
#define _FPU_EXTENDED 0
#define _FPU_DOUBLE   0
#define _FPU_SINGLE   0

/* rounding control */
#define _FPU_RC_NEAREST 0x0    /* RECOMMENDED */
#define _FPU_RC_DOWN    0x2
#define _FPU_RC_UP      0x3
#define _FPU_RC_ZERO    0x1

#define _FPU_RESERVED 0xfe7c0000  /* Reserved bits */


/* The fdlibm code requires strict IEEE double precision arithmetic,
   and no interrupts for exceptions, rounding to nearest.  */

#define _FPU_DEFAULT  0x0

/* IEEE:  same as above, but exceptions */
#define _FPU_IEEE     (0x1f << 7)

/* Type of the control word.  */
typedef unsigned int fpu_control_t;

/* Macros for accessing the hardware control word.  */
#define _FPU_GETCW(cw) __asm__ ("cfc1 %0, $31; nop; nop" : "=r" (cw))
#define _FPU_SETCW(cw) __asm__ ("ctc1 %0, $31; nop; nop" : : "r" (cw))

/* Default control word set at startup.  */
extern fpu_control_t __fpu_control;

__BEGIN_DECLS

/* Called at startup.  It can be used to manipulate fpu control register.  */
extern void __setfpucw __P ((fpu_control_t));

__END_DECLS

#endif	/* fpu_control.h */
