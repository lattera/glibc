/* setjmp for i386.
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
#define _ASM
#include <bits/setjmp.h>
#include <asm-syntax.h>

	/* Binary compatibility entry point.  */
ENTRY (__setjmp)
	popl %eax		/* Pop return address.  */
	popl %ecx		/* Pop jmp_buf.  */
	pushl $0		/* Push zero argument.  */
	pushl %ecx		/* Push jmp_buf.  */
	pushl %eax		/* Push back return address.  */

ENTRY (__sigsetjmp)
	movl 4(%esp), %eax	/* User's jmp_buf in %eax.  */
     	/* Save registers.  */
	movl %ebx, (JB_BX*4)(%eax)
	movl %esi, (JB_SI*4)(%eax)
	movl %edi, (JB_DI*4)(%eax)
	movl %ebp, (JB_BP*4)(%eax)
	leal 4(%esp), %ecx	/* Save SP as it will be after we return.  */
     	movl %ecx, (JB_SP*4)(%eax)
	movl 0(%esp), %ecx	/* Save PC we are returning to now.  */
     	movl %ecx, (JB_PC*4)(%eax)

	/* Make a tail call to __sigjmp_save; it takes the same args.  */
#ifdef	PIC
	/* We cannot use the PLT, because it requires that %ebx be set, but
           we can't save and restore our caller's value.  Instead, we do an
           indirect jump through the GOT, using for the temporary register
           %ecx, which is call-clobbered.  */
	call L(here)
L(here):
	popl %ecx
	addl $_GLOBAL_OFFSET_TABLE_+[.-L(here)], %ecx
	movl C_SYMBOL_NAME(__sigjmp_save@GOT)(%ecx), %ecx
	jmp *%ecx
#else
	jmp __sigjmp_save
#endif
END (__sigsetjmp)
