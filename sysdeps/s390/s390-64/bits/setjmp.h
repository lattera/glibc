/* Copyright (C) 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* Define the machine-dependent type `jmp_buf'.  64 bit S/390 version.  */

#ifndef __S390_SETJMP_H__
#define __S390_SETJMP_H__

#define	__JB_GPR6	0
#define __JB_GPR7	1
#define __JB_GPR8	2
#define __JB_GPR9	3
#define __JB_GPR10	4 
#define __JB_GPR11	5
#define __JB_GPR12	6 
#define __JB_GPR13	7
#define __JB_GPR14	8
#define __JB_GPR15	9

#ifndef	_ASM

typedef struct {
    /* We save registers 6-15.  */
    long int __gregs[10];

    /* We save fpu registers 4 and 6.  */
    long __fpregs[8];
} __jmp_buf[1];

#endif

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((int) (address) < (jmpbuf)->gregs[__JB_GPR15])

#endif /* __S390_SETJMP_H__ */

