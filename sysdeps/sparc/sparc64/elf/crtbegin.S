/* Destructor cleanup code for elf64-sparc
   Copyright (C) 1997 Free Software Foundation, Inc.
   Contributed by Richard Henderson <richard@gnu.ai.mit.edu>, 1997.
 
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


.section ".ctors",#alloc,#write

	.align 8
__CTOR_LIST__:
	.xword	-1

.section ".dtors",#alloc,#write

	.align 8
__DTOR_LIST__:
	.xword	-1

.section ".fini",#alloc,#execinstr

	call	__do_global_dtors_aux
	 nop

.text

	.align 4
	.type __do_global_dtors_aux,#function
__do_global_dtors_aux:
	save	%sp,-192,%sp

#ifdef PIC
1:	call	11f
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-(1b-.)), %l7
11:	or	%l7, %lo(_GLOBAL_OFFSET_TABLE_-(1b-.)), %l7
	add	%l7, %o7, %l7
	sethi	%hi(__DTOR_LIST__), %l0
	or	%l0, %lo(__DTOR_LIST__), %l0
	ldx	[%l7+%l0], %l0
#else
	sethi	%hi(__DTOR_LIST__), %l0
	or	%l0, %lo(__DTOR_LIST__), %l0
#endif

	ba	3f
	 ldx	[%l0+8], %l1
2:	jmpl	%l1, %o7
	 ldx	[%l0+8], %l1
3:	brnz,pt	%l1, 2b
	 add	%l0, 8, %l0

	ret
	 restore

	.size __do_global_dtors_aux,.-__do_global_dtors_aux
