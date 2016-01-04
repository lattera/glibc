/* Thread-local storage handling in the ELF dynamic linker.  ARM version.
   Copyright (C) 2006-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <arm-features.h>
#include <tls.h>
#include "tlsdesc.h"

	.text
	@ emit debug information with cfi
	@ use arm-specific pseudos for unwinding itself
	CFI_SECTIONS
	.hidden _dl_tlsdesc_return
	.global	_dl_tlsdesc_return
	.type	_dl_tlsdesc_return,#function
	cfi_startproc
	eabi_fnstart
	.align 2
_dl_tlsdesc_return:
	sfi_breg r0, \
	ldr	r0, [\B]
	BX	(lr)
	eabi_fnend
	cfi_endproc
	.size	_dl_tlsdesc_return, .-_dl_tlsdesc_return

	.hidden _dl_tlsdesc_undefweak
	.global	_dl_tlsdesc_undefweak
	.type	_dl_tlsdesc_undefweak,#function
	cfi_startproc
	eabi_fnstart
	.align 2
_dl_tlsdesc_undefweak:
	GET_TLS (r1)
	rsb 	r0, r0, #0
	BX	(lr)
	cfi_endproc
	eabi_fnend
	.size	_dl_tlsdesc_undefweak, .-_dl_tlsdesc_undefweak

#ifdef SHARED
	.hidden _dl_tlsdesc_dynamic
	.global	_dl_tlsdesc_dynamic
	.type	_dl_tlsdesc_dynamic,#function


/*
	The assembly code that follows is a rendition of the following
	C code, hand-optimized a little bit.

ptrdiff_t
_dl_tlsdesc_dynamic(struct tlsdesc *tdp)
{
       struct tlsdesc_dynamic_arg *td = tdp->argument.pointer;
       dtv_t *dtv = (dtv_t *)THREAD_DTV();
       if (__builtin_expect (td->gen_count <= dtv[0].counter
                             && dtv[td->tlsinfo.ti_module].pointer.val
                                != TLS_DTV_UNALLOCATED,
                             1))
               return dtv[td->tlsinfo.ti_module].pointer.val +
                       td->tlsinfo.ti_offset - __builtin_thread_pointer();

       return __tls_get_addr (&td->tlsinfo) - __builtin_thread_pointer();
}

*/
	cfi_startproc
	eabi_fnstart
	.align 2
_dl_tlsdesc_dynamic:
	/* Our calling convention is to clobber r0, r1 and the processor
	   flags.  All others that are modified must be saved */
	eabi_save ({r2,r3,r4,lr})
	push	{r2,r3,r4,lr}
	cfi_adjust_cfa_offset (16)
	cfi_rel_offset (r2,0)
	cfi_rel_offset (r3,4)
	cfi_rel_offset (r4,8)
	cfi_rel_offset (lr,12)
	sfi_breg r0, \
	ldr	r1, [\B] /* td */
	GET_TLS (lr)
	mov	r4, r0 /* r4 = tp */
	sfi_breg r0, \
	ldr	r0, [\B]
	sfi_breg r1, \
	ldr	r2, [\B, #8] /* gen_count */
	sfi_breg r0, \
	ldr	r3, [\B]
	cmp	r2, r3
	bhi	1f
	sfi_breg r1, \
	ldr	r3, [\B]
#ifndef ARM_NO_INDEX_REGISTER
	ldr	r2, [r0, r3, lsl #3]
#else
	add	lr, r0, r3, lsl #3
	sfi_breg lr, \
	ldr	r2, [\B]
#endif
	cmn	r2, #1
	ittt	ne
	sfi_breg r1, \
	ldrne	r3, [r1, #4]
	addne	r3, r2, r3
	rsbne	r0, r4, r3
	bne	2f
1:	mov	r0, r1
	bl	__tls_get_addr
	rsb	r0, r4, r0
2:
#if ((defined (__ARM_ARCH_4T__) && defined (__THUMB_INTERWORK__)) \
     || defined (ARM_ALWAYS_BX))
	pop	{r2,r3,r4, lr}
	cfi_adjust_cfa_offset (-16)
	cfi_restore (lr)
	cfi_restore (r4)
	cfi_restore (r3)
	cfi_restore (r2)
	bx	lr
#else
	pop	{r2,r3,r4, pc}
#endif
	eabi_fnend
	cfi_endproc
	.size	_dl_tlsdesc_dynamic, .-_dl_tlsdesc_dynamic
#endif /* SHARED */

/* lazy resolved for tls descriptors.  */
	.hidden _dl_tlsdesc_lazy_resolver
	.global	_dl_tlsdesc_lazy_resolver
	.type	_dl_tlsdesc_lazy_resolver,#function
	cfi_startproc
	eabi_fnstart
	.align 2
_dl_tlsdesc_lazy_resolver:
	/* r0 points at the tlsdesc,
	   r1 points at the GOT
	   r2 was pushed by the trampoline and used as a temp,
	      we need to pop it here.
	  We push the remaining call-clobbered registers here, and also
	  R1 -- to keep the stack correctly aligned.  */
	/* Tell the unwinder that r2 has already been pushed.  */
	eabi_save ({r2})
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r2, 0)
	eabi_save ({r0,r1,r3,ip,lr})
	push	{r0, r1, r3, ip, lr}
	cfi_adjust_cfa_offset (20)
	cfi_rel_offset (r0, 0)
	cfi_rel_offset (r1, 4)
	cfi_rel_offset (r3, 8)
	cfi_rel_offset (ip, 12)
	cfi_rel_offset (lr, 16)
	bl	_dl_tlsdesc_lazy_resolver_fixup
	pop	{r0, r1, r3, ip, lr}
	cfi_adjust_cfa_offset (-20)
	cfi_restore (lr)
	cfi_restore (ip)
	cfi_restore (r3)
	cfi_restore (r1)
	cfi_restore (r0)
	pop	{r2}
	cfi_adjust_cfa_offset (-4)
	cfi_restore (r2)
	sfi_breg r0, \
	ldr	r1, [\B, #4]
	BX	(r1)
	eabi_fnend
	cfi_endproc
	.size	_dl_tlsdesc_lazy_resolver, .-_dl_tlsdesc_lazy_resolver

/* Holder for lazy tls descriptors being resolve in another thread.

   Our calling convention is to clobber r0, r1 and the processor
   flags.  All others that are modified must be saved */
	.hidden _dl_tlsdesc_resolve_hold
	.global	_dl_tlsdesc_resolve_hold
	.type	_dl_tlsdesc_resolve_hold,#function
	cfi_startproc
	eabi_fnstart
	.align 2
_dl_tlsdesc_resolve_hold:
	/* r0 is saved so its original value can be used after the call and
	   r1 is saved only to keep the stack aligned.  (r0 points to the tls
	   descriptor, it is passed to _dl_tlsdesc_resolve_hold_fixup which
	   is a void function that may clobber r0, later r0 is used to load
	   the new resolver.)  */
	eabi_save ({r0,r1,r2,r3,ip,lr})
	push	{r0, r1, r2, r3, ip, lr}
	cfi_adjust_cfa_offset (24)
	cfi_rel_offset (r0, 0)
	cfi_rel_offset (r1, 4)
	cfi_rel_offset (r2, 8)
	cfi_rel_offset (r3, 12)
	cfi_rel_offset (ip, 16)
	cfi_rel_offset (lr, 20)
	adr	r1, _dl_tlsdesc_resolve_hold
	bl	_dl_tlsdesc_resolve_hold_fixup
	pop	{r0, r1, r2, r3, ip, lr}
	cfi_adjust_cfa_offset (-24)
	cfi_restore (lr)
	cfi_restore (ip)
	cfi_restore (r3)
	cfi_restore (r2)
	cfi_restore (r1)
	cfi_restore (r0)
	sfi_breg r0, \
	ldr     r1, [\B, #4]
	BX      (r1)
	eabi_fnend
	cfi_endproc
	.size	_dl_tlsdesc_resolve_hold, .-_dl_tlsdesc_resolve_hold
