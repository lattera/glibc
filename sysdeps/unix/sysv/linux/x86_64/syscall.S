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

#include <sysdep.h>

/* Please consult the file sysdeps/unix/sysv/linux/x86-64/sysdep.h for
   more information about the value -4095 used below.  */

/* Usage: long syscall (syscall_number, arg1, arg2, arg3, arg4, arg5)
   We need to do some arg shifting, the syscall_number will be in
   rax.  */


	.text
ENTRY (syscall)
.LFB1:
	movq %rdi, %rax		/* Syscall number -> rax.  */
	movq %rsi, %rdi		/* shift arg1 - arg5.  */
	movq %rdx, %rsi
	movq %rcx, %rdx
	movq %r8, %r10
	movq %r9, %r8
	syscall			/* Do the system call.  */
	cmpq $-4095, %rax	/* Check %rax for error.  */
	jae SYSCALL_ERROR_LABEL	/* Jump to error handler if error.  */
L(pseudo_end):
.LFE1:
	ret			/* Return to caller.  */

PSEUDO_END (syscall)

        .section        .eh_frame,"aw",@progbits
.Lframe1:
        .long   .LECIE1-.LSCIE1 # Length of Common Information Entry
.LSCIE1:
        .long   0x0     # CIE Identifier Tag
        .byte   0x1     # CIE Version
        .ascii "\0"     # CIE Augmentation
        .uleb128 0x1    # CIE Code Alignment Factor
        .sleb128 -8     # CIE Data Alignment Factor
        .byte   0x10    # CIE RA Column
        .byte   0xc     # DW_CFA_def_cfa
        .uleb128 0x7
        .uleb128 0x8
        .byte   0x90    # DW_CFA_offset, column 0x10
        .uleb128 0x1
        .align 8
.LECIE1:
.LSFDE1:
        .long   .LEFDE1-.LASFDE1        # FDE Length
.LASFDE1:
        .long   .LASFDE1-.Lframe1       # FDE CIE offset
        .quad   .LFB1   # FDE initial location
        .quad   .LFE1-.LFB1     # FDE address range
        .align 8
.LEFDE1:
