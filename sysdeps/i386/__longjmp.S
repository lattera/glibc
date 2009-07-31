/* longjmp for i386.
   Copyright (C) 1995-1998,2000,2002,2005,2006,2009
   Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <jmpbuf-offsets.h>
#include <asm-syntax.h>

	.text
ENTRY (__longjmp)
#ifdef PTR_DEMANGLE
	movl 4(%esp), %eax	/* User's jmp_buf in %eax.  */

	/* Save the return address now.  */
	movl (JB_PC*4)(%eax), %edx
	/* Get the stack pointer.  */
	movl (JB_SP*4)(%eax), %ecx
	PTR_DEMANGLE (%edx)
	PTR_DEMANGLE (%ecx)
	cfi_def_cfa(%eax, 0)
	cfi_register(%eip, %edx)
	cfi_register(%esp, %ecx)
	cfi_offset(%ebx, JB_BX*4)
	cfi_offset(%esi, JB_SI*4)
	cfi_offset(%edi, JB_DI*4)
	cfi_offset(%ebp, JB_BP*4)
     	/* Restore registers.  */
	movl (JB_BX*4)(%eax), %ebx
	movl (JB_SI*4)(%eax), %esi
	movl (JB_DI*4)(%eax), %edi
	movl (JB_BP*4)(%eax), %ebp
	cfi_restore(%ebx)
	cfi_restore(%esi)
	cfi_restore(%edi)
	cfi_restore(%ebp)

	movl 8(%esp), %eax	/* Second argument is return value.  */
	movl %ecx, %esp
#else
	movl 4(%esp), %ecx	/* User's jmp_buf in %ecx.  */
	movl 8(%esp), %eax	/* Second argument is return value.  */
	/* Save the return address now.  */
	movl (JB_PC*4)(%ecx), %edx
     	/* Restore registers.  */
	movl (JB_BX*4)(%ecx), %ebx
	movl (JB_SI*4)(%ecx), %esi
	movl (JB_DI*4)(%ecx), %edi
	movl (JB_BP*4)(%ecx), %ebp
	movl (JB_SP*4)(%ecx), %esp
#endif
	/* Jump to saved PC.  */
     	jmp *%edx
END (__longjmp)
