/* pwrite64 syscall for Linux/ix86.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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
#include <asm/errno.h>

/* Please consult the file sysdeps/unix/sysv/linux/i386/sysdep.h for
   more information about the value -4095 used below.*/

	.text
ENTRY (__syscall_pwrite64)

#ifndef __NR_pwrite
	movl	$-ENOSYS,%eax
# ifndef PIC
	jmp	syscall_error
# endif
#else

	PUSHARGS_5		/* Save register contents.  */

	/* Load arguments.  This is unfortunately a little bit of a problem
	   since the kernel expects the arguments in a different order.  */
	movl	0x20(%esp,1),%esi
	movl	0x1c(%esp,1),%edi
	movl	0x18(%esp,1),%edx
	movl	0x14(%esp,1),%ecx
	movl	0x10(%esp,1),%ebx
	/* Load syscall number into %eax.  */
	movl	$SYS_ify(pwrite), %eax
	int	$0x80			/* Do the system call.  */
	POPARGS_5			/* Restore register contents.  */
	cmpl	$-4095, %eax		/* Check %eax for error.  */
	jae	SYSCALL_ERROR_LABEL	/* Jump to error handler if error.  */
#endif
L(pseudo_end):
	ret			/* Return to caller.  */

PSEUDO_END (__syscall_pwrite64)
