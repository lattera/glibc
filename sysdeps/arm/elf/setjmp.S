/* setjmp for arm, ELF version.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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
#define _SETJMP_H
#include <bits/setjmp.h>

	/* We include the BSD entry points here as well but we make
	   them weak.  */
ENTRY (setjmp)
	.weak C_SYMBOL_NAME (setjmp)
	@ call comes here with
	@ lr = return pc
	@ r0 = &jump_buf

	@ add a second argument into the fray
	mov r1, #1
	@ call the common code
	b __sigsetjmp(PLT)
END (setjmp)

	/* Binary compatibility entry point.  */
ENTRY (_setjmp)
	.weak C_SYMBOL_NAME (_setjmp)
ENTRY (__setjmp)
	@ call comes here with
	@ lr = return pc
	@ r0 = &jump_buf

	@ add a second argument into the fray
	mov r1, #1
	@ fall through to the common code....

ENTRY (__sigsetjmp)
	@ save the registers into the jmp_buf....
	stmia r0, {r1-r6, sl, fp, sp, lr}

	/* Make a tail call to __sigjmp_save; it takes the same args.  */
#ifdef	PIC
	/* We cannot use the PLT, because it requires that sl==r10 be set, but
           we can't save and restore our caller's value.  Instead, we do an
           indirect jump through the GOT, using for the temporary register
           ip, which is call-clobbered.  */
	ldr ip, 1f
	add ip, ip, pc
2:	@ get the address of __sigjmp_save
	ldr r1, 3f
	ldr ip, [ip, r1]
	@ restore r1
	ldr r1, [r0]
	@ jump to *__sigjmp_save
	mov pc, ip
1:	.word _GLOBAL_OFFSET_TABLE_ - 2b - 4
3:	.word __sigjmp_save(GOT)

#else
	b __sigjmp_save
#endif
END (__sigsetjmp)
