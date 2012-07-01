/* FPU control word bits.  AM33/2.0 version.
   Copyright (C) 1996, 1997, 1998, 1999, 2000, 2004
   Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>
   based on the corresponding file in the mips port.

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

#ifndef _FPU_CONTROL_H
#define _FPU_CONTROL_H

/* AM33/2.0 FPU floating point control register bits.
 *
 * 31-22  -> reserved
 * 21-18  -> floating-point condition codes (L, G, E, U)
 * 17-16  -> rounding modes (00 is to-nearest; other values are reserved
 * 15     -> reserved (read as 0, write with 0)
 * 14-10  -> Exception Cause (inValid, divZero, Overflow, Underflow, Inexact)
 *  9- 5  -> Exception Enable
 *  4- 0  -> Exception Flag, cleared when exception cause is set
 */

#include <features.h>
#include <fenv.h>

/* masking of interrupts */
#define _FPU_MASK_V     0x0200  /* Invalid operation */
#define _FPU_MASK_Z     0x0100  /* Division by zero  */
#define _FPU_MASK_O     0x0080  /* Overflow          */
#define _FPU_MASK_U     0x0040  /* Underflow         */
#define _FPU_MASK_I     0x0020  /* Inexact operation */

/* rounding control */
#define _FPU_RC_NEAREST 0x0     /* Only available mode */

#define _FPU_RESERVED 0xffc08000  /* Reserved bits in fpcr */


/* The fdlibm code requires strict IEEE double precision arithmetic,
   and no interrupts for exceptions, rounding to nearest.  */

#define _FPU_DEFAULT  0x0000001f

/* IEEE:  same as above, but exceptions */
#define _FPU_IEEE     0x000003ff

/* Type of the control word.  */
typedef unsigned int fpu_control_t;

/* Macros for accessing the hardware control word.  _FPU_SETCW is
   defined such that it won't modify the EF bits, that are cleared
   when assigned bits that are set.  Use SETFCW to get them actually
   reset.  */
#define _FPU_SETFCW(cw) __asm__ ("fmov %0,fpcr" : : "ri" (cw))
#define _FPU_SETCW(cw) _FPU_SETFCW((cw) & ~FE_ALL_EXCEPT)
#define _FPU_GETCW(cw) __asm__ ("fmov fpcr,%0" : "=r" (cw))

/* Default control word set at startup.  */
extern fpu_control_t __fpu_control;

#endif	/* fpu_control.h */
