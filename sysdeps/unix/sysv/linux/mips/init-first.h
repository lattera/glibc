/* Prepare arguments for library initialization function.
   Copyright (C) 2000 Free Software Foundation, Inc.
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

/* The job of this fragment it to find argc and friends for INIT.
   This is done in one of two ways: either in the stack context
   of program start, or having dlopen pass them in.  */

#define SYSDEP_CALL_INIT(NAME, INIT)		\
    asm(".weak _dl_starting_up\n\t"		\
        ".globl " #NAME "\n\t"			\
	".ent " #NAME "\n"			\
	#NAME ":\n\t"				\
	".set	noreorder\n\t"			\
	".cpload $25\n\t"			\
	".set	reorder\n\t"			\
	/* Are we a dynamic libc being loaded into a static program?  */ \
	"la	$8, _dl_starting_up\n\t"	\
	"beqz	$8, 1f\n\t"			\
	"lw	$8, 0($8)\n\t"			\
	"seq	$8, $8, 0\n"			\
	"1:\t"					\
	"sw	$8, __libc_multiple_libcs\n\t"	\
	/* If so, argc et al are in a0-a2 already.  Otherwise, load them.  */ \
	"bnez	$8, 2f\n\t"			\
	"lw	$4, 16($29)\n\t"		\
	"addiu	$5, $29, 20\n\t"		\
	"sll	$6, $4, 2\n\t"			\
	"addiu	$6, $6, 4\n\t"			\
	"addu	$6, $5, $6\n"			\
	"2:\t"					\
	"la	$25, " #INIT "\n\t"		\
	"jr	$25\n\t"			\
	".end " #NAME "\n\t"			\
	"3:\t"					\
	".size	" #NAME ", 3b-" #NAME);
