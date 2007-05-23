/* Copyright (C) 1999, 2005 Free Software Foundation, Inc.
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

/* If no SA_RESTORER function was specified by the application we use
   one of these.  This avoids the need for the kernel to synthesise a return
   instruction on the stack, which would involve expensive cache flushes.

   Nowadays (2.6 series, and somewhat earlier) the kernel uses a high page
   for signal trampolines, so the cache flushes are not an issue.  But since
   we do not have a vDSO, continue to use these so that we can provide
   unwind information.

   Start the unwind tables at least one instruction before the signal
   trampoline, because the unwinder will assume we are returning after
   a call site.

   Because the signal frame layout changed in 2.6.18, we provide two
   copies of these functions with different unwind information.  */

#ifndef __ASSUME_SIGFRAME_V2
	.fnstart
	.save {r0-r15}
	.pad #12
	nop
ENTRY(__default_sa_restorer_v1)
	mov	r7, $SYS_ify(sigreturn)
	swi	0x0
	.fnend
#endif

	.fnstart
	.save {r0-r15}
	.pad #32
	nop
ENTRY(__default_sa_restorer_v2)
	mov	r7, $SYS_ify(sigreturn)
	swi	0x0
	.fnend

#ifdef __NR_rt_sigreturn

#ifndef __ASSUME_SIGFRAME_V2
	.fnstart
	.save {r0-r15}
	.pad #168
	nop
ENTRY(__default_rt_sa_restorer_v1)
	mov	r7, $SYS_ify(rt_sigreturn)
	swi	0x0
	.fnend
#endif

	.fnstart
	.save {r0-r15}
	.pad #160
	nop
ENTRY(__default_rt_sa_restorer_v2)
	mov	r7, $SYS_ify(rt_sigreturn)
	swi	0x0
	.fnend

#endif
