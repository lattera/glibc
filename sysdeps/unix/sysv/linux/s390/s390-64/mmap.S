/* Wrapper around mmap system call.  64 bit S/390 version.
   Copyright (C) 2001-2015 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

#define EINVAL	22

        .text

ENTRY(__mmap)
	/* Save registers and setup stack frame.  */
        stmg    %r6,%r15,48(%r15)
	cfi_offset (%r15,-40)
	cfi_offset (%r14,-48)
	cfi_offset (%r13,-56)
	cfi_offset (%r12,-64)
	cfi_offset (%r11,-72)
	cfi_offset (%r10,-80)
	cfi_offset (%r9,-88)
	cfi_offset (%r8,-96)
	cfi_offset (%r7,-104)
	cfi_offset (%r6,-112)
        lgr     %r1,%r15
        lg      %r0,8(%r15)             /* Load eos.  */
        aghi    %r15,-208               /* Buy stack space.  */
	cfi_adjust_cfa_offset (208)
        stg     %r1,0(%r15)             /* Store back chain.  */
        stg     %r0,8(%r15)             /* Store eos.  */

        /* Store parameters on stack, because old_mmap
           takes only one parameter: a pointer to the parameter area.  */
        mvc     200(8,%r15),368(%r15)   /* Move 'offset'.  */
        lgfr    %r6,%r6
        stg     %r6,192(%r15)           /* Store 'fd'.  */
        lgfr    %r5,%r5
        stg     %r5,184(%r15)           /* Store 'flags'.  */
        lgfr    %r4,%r4
        stg     %r4,176(%r15)           /* Store 'prot'.  */
        stg     %r3,168(%r15)           /* Store 'length'.  */
        stg     %r2,160(%r15)           /* Store 'start'.  */

        la      %r2,160(%r15)           /* Load address of parameter list.  */
        /* Do the system call trap.  */
        svc     SYS_ify(mmap)

        lg      %r15,0(%r15)            /* Load back chain.  */
	cfi_adjust_cfa_offset (-208)
        lmg     %r6,%r15,48(%r15)       /* Load registers.  */

        /* Check gpr 2 for error.  */
        lghi    %r0,-4096
        clgr    %r2,%r0
        jgnl    SYSCALL_ERROR_LABEL

        /* Successful; return the syscall's value.  */
        br      %r14

PSEUDO_END (__mmap)

weak_alias (__mmap, mmap)
weak_alias (__mmap, mmap64)
weak_alias (__mmap, __mmap64)
