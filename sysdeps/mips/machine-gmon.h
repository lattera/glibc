/* Machine-specific calling sequence for `mcount' profiling function.  MIPS
   Copyright (C) 1996, 1997, 2000, 2001, 2002 Free Software Foundation, Inc.
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

#define _MCOUNT_DECL(frompc,selfpc) \
static void __attribute_used__ __mcount (u_long frompc, u_long selfpc)

/* Call __mcount with our the return PC for our caller,
   and the return PC our caller will return to.  */
#ifdef __PIC__
#define CPLOAD ".cpload $25;"
#define CPRESTORE ".cprestore 44\n\t"
#else
#define CPLOAD
#define CPRESTORE
#endif

#define MCOUNT asm(\
	".globl _mcount;\n\t" \
	".align 2;\n\t" \
	".type _mcount,@function;\n\t" \
	".ent _mcount\n\t" \
        "_mcount:\n\t" \
        ".frame $sp,44,$31\n\t" \
        ".set noreorder;\n\t" \
        ".set noat;\n\t" \
        CPLOAD \
	"subu $29,$29,48;\n\t" \
	CPRESTORE \
        "sw $4,24($29);\n\t" \
        "sw $5,28($29);\n\t" \
        "sw $6,32($29);\n\t" \
        "sw $7,36($29);\n\t" \
        "sw $2,40($29);\n\t" \
        "sw $1,16($29);\n\t" \
        "sw $31,20($29);\n\t" \
        "move $5,$31;\n\t" \
        "move $4,$1;\n\t" \
        "jal __mcount;\n\t" \
        "nop;\n\t" \
        "lw $4,24($29);\n\t" \
        "lw $5,28($29);\n\t" \
        "lw $6,32($29);\n\t" \
        "lw $7,36($29);\n\t" \
        "lw $2,40($29);\n\t" \
        "lw $31,20($29);\n\t" \
        "lw $1,16($29);\n\t" \
        "addu $29,$29,56;\n\t" \
        "j $31;\n\t" \
        "move $31,$1;\n\t" \
        ".set reorder;\n\t" \
        ".set at\n\t" \
        ".end _mcount");
