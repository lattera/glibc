/* Copyright (C) 2000 Free Software Foundation, Inc.

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


/* int  __clone2(int (*fn) (void *arg), void *child_stack_base, 	*/
/* 	         size_t child_stack_size, int flags, void *arg) */

ENTRY(__clone2)
	cmp.eq p6,p0=0,r32
	mov r8=EINVAL
(p6)	br.cond.spnt.few __syscall_error
	;;
	flushrs			/* This is necessary, since the child	*/
				/* will be running with the same 	*/
				/* register backing store for a few 	*/
				/* instructions.  We need to ensure	*/
				/* that it will not read or write the	*/
				/* backing store.			*/
	mov r17=ar.rsc		/* save ar.rsc	*/
	mov r14=r32		/* save fn	*/
	mov r18=r33		/* save child_stack_base		*/
				/* Note that r15 is used to pass 	*/
				/* syscall # to kernel & not preserved. */
	mov r16=r36		/* save arg	*/
	;;
	dep r36=0,r17,0,2	/* set to enforced lazy mode.	*/
	;;
	mov ar.rsc=r36
	cmp.ne p7,p0=0,r33	/* stack_base 0?	*/
	;;
(p7)	add r33=r33,r34		/* Stack base arg to syscall is 	*/
				/* 0 if child_stack_base is 0,		*/
				/* child_stack_base + child_stack_size	*/
				/* otherwise.				*/
				/* The system call interface seems 	*/
				/* quite contrived at this point.  If	*/
				/* we don't pass the backing store	*/
				/* pointer, why do we pass the sp?	*/
	mov r32=r35		/* Flags are first syscall argument.	*/
        DO_CALL (SYS_ify (clone))
        cmp.eq p6,p0=-1,r10
	;;
(p6)	br.cond.spnt.few __syscall_error

#	define CHILD p6
#	define PARENT p7
	cmp.eq CHILD,PARENT=0,r8 /* Are we the child?	*/
	;;
(CHILD)	ld8 r34=[r14],8		/* Retrieve code pointer.	*/
(CHILD)	mov ar.bspstore=r18	/* Set register backing store in the child */
(CHILD)	mov r32=r16		/* Pass proper argument	to fn */
	mov ar.rsc=r17		/* Restore RSE mode (both threads).	*/
(PARENT) ret
	;;
	ld8 gp=[r14]		/* Load function gp.		*/
	mov b6=r34
	;;
	br.call.dptk.few rp=b6	/* Call fn(arg) in the child 	*/
	;;
	mov r32=r8		/* Argument to _exit		*/
	.globl _exit
	br.call.dpnt.few rp=_exit /* call _exit with result from fn.	*/
	ret			/* Not reached.		*/

PSEUDO_END(__clone2)

/* For now we leave __clone undefined.  This is unlikely to be a	*/
/* problem, since at least the i386 __clone in glibc always failed	*/
/* with a 0 sp (eventhough the kernel explicitly handled it).		*/
/* Thus all such calls needed to pass an explicit sp, and as a result,	*/
/* would be unlikely to work on ia64.					*/
