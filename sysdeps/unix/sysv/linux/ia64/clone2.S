/* Copyright (C) 2000, 2001 Free Software Foundation, Inc.
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
#include <asm/errno.h>


/* int  __clone2(int (*fn) (void *arg), void *child_stack_base, 	*/
/* 	         size_t child_stack_size, int flags, void *arg) */

ENTRY(__clone2)
	alloc r2=ar.pfs,5,2,3,0
	cmp.eq p6,p0=0,in0
	mov r8=EINVAL
(p6)	br.cond.spnt.few __syscall_error
	;;
	flushrs			/* This is necessary, since the child	*/
				/* will be running with the same 	*/
				/* register backing store for a few 	*/
				/* instructions.  We need to ensure	*/
				/* that it will not read or write the	*/
				/* backing store.			*/
	mov loc0=in0		/* save fn	*/
	mov loc1=in4		/* save arg	*/
	mov out0=in3		/* Flags are first syscall argument.	*/
	mov out1=in1		/* Stack address.			*/
	mov out2=in2		/* Stack size.				*/
        DO_CALL (SYS_ify (clone2))
        cmp.eq p6,p0=-1,r10
	;;
(p6)	br.cond.spnt.few __syscall_error

#	define CHILD p6
#	define PARENT p7
	cmp.eq CHILD,PARENT=0,r8 /* Are we the child?	*/
	;;
(CHILD)	ld8 out1=[loc0],8	/* Retrieve code pointer.	*/
(CHILD)	mov out0=loc1		/* Pass proper argument	to fn */
(PARENT) ret
	;;
	ld8 gp=[loc0]		/* Load function gp.		*/
	mov b6=out1
	;;
	br.call.dptk.few rp=b6	/* Call fn(arg) in the child 	*/
	;;
	mov out0=r8		/* Argument to _exit		*/
	.globl _exit
	br.call.dpnt.few rp=_exit /* call _exit with result from fn.	*/
	ret			/* Not reached.		*/

PSEUDO_END(__clone2)

/* For now we leave __clone undefined.  This is unlikely to be a	*/
/* problem, since at least the i386 __clone in glibc always failed	*/
/* with a 0 sp (eventhough the kernel explicitly handled it).		*/
/* Thus all such calls needed to pass an explicit sp, and as a result,	*/
/* would be unlikely to work on ia64.					*/
