/* Machine-specific calling sequence for `mcount' profiling function.  MIPS
   Copyright (C) 1996, 1997, 2000 Free Software Foundation, Inc.
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

#define _MCOUNT_DECL static void __mcount

/* Call __mcount with our the return PC for our caller,
   and the return PC our caller will return to.  */
#ifdef __PIC__
#define CPLOAD ".cpload $25;"
#else
#define CPLOAD
#endif

#define MCOUNT asm(\
	".globl _mcount;" \
	".align 2;" \
	".type _mcount,@function;" \
        "_mcount:;" \
        ".set noreorder;" \
        ".set noat;" \
        CPLOAD \
        "sw $4,8($29);" \
        "sw $5,12($29);" \
        "sw $6,16($29);" \
        "sw $7,20($29);" \
        "sw $1,0($29);" \
        "sw $31,4($29);" \
        "move $5,$31;" \
        "jal __mcount;" \
        "move $4,$1;" \
        "lw $4,8($29);" \
        "lw $5,12($29);" \
        "lw $6,16($29);" \
        "lw $7,20($29);" \
        "lw $31,4($29);" \
        "lw $1,0($29);" \
        "addu $29,$29,8;" \
        "j $31;" \
        "move $31,$1;" \
        ".set reorder;" \
        ".set at");
