/* Copyright (C) 1991, 1993, 1994, 1996, 1997 Free Software Foundation, Inc.
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

#define _ASM 1
#include <jmp_buf.h>

ENTRY (__sigsetjmp)
	/* Save our SP and FP; in the delay slot of the jump, save our
	   return PC.  Save the signal mask if requested with a tail-call
	   for simplicity; it always returns zero.  */
#ifdef PIC
2:
	jmpl 1f,%g2
	nop
1:
	sethi %hi(_GLOBAL_OFFSET_TABLE_-(2b-.)),%l7
	sethi %hi(C_SYMBOL_NAME (__sigjmp_save)), %g3
	st    %sp, [%o0 + (JB_SP * 4)]
	or    %l7,%lo(_GLOBAL_OFFSET_TABLE_-(2b-.)),%l7
	or    %g3,%lo(C_SYMBOL_NAME (__sigjmp_save)), %g3
	st    %fp, [%o0 + (JB_FP * 4)]
	ld    [%l7+%g3],%g1
	jmp   %g1
	st   %o7, [%o0+(JB_PC*4)]
#else
	sethi %hi(C_SYMBOL_NAME (__sigjmp_save)), %g1
	st %sp, [%o0 + (JB_SP*4)]
	or %lo(C_SYMBOL_NAME (__sigjmp_save)), %g1, %g1
	st %fp, [%o0 + (JB_FP*4)]
	jmp %g1
	 st %o7, [%o0 + (JB_PC*4)]
#endif /* PIC */
