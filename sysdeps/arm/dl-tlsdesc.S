/* Thread-local storage handling in the ELF dynamic linker.  ARM version.
   Copyright (C) 2006, 2010, 2011 Free Software Foundation, Inc.
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
#include <tls.h>
#include "tlsdesc.h"

#ifdef __USE_BX__
  #define BX(x)	bx x
#else
  #define BX(x)	mov pc, x
#endif

	.text
	@ emit debug information with cfi
	@ use arm-specific pseudos for unwinding itself
	.cfi_sections .debug_frame
	.hidden _dl_tlsdesc_return
	.global	_dl_tlsdesc_return
	.type	_dl_tlsdesc_return,#function
	cfi_startproc
	.fnstart
	.align 2
_dl_tlsdesc_return:
	ldr	r0, [r0]
	BX	(lr)
	.fnend
	cfi_endproc
	.size	_dl_tlsdesc_return, .-_dl_tlsdesc_return

	.hidden _dl_tlsdesc_undefweak
	.global	_dl_tlsdesc_undefweak
	.type	_dl_tlsdesc_undefweak,#function
	cfi_startproc
	.fnstart
	.align 2
_dl_tlsdesc_undefweak:
	@ Are we allowed a misaligned stack pointer calling read_tp?
	.save	{lr}
	stmdb 	sp!, {lr}
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (lr,0)
	bl 	__aeabi_read_tp
	rsb 	r0, r0, #0
	ldmia 	sp!, {lr}
	cfi_adjust_cfa_offset (-4)
	cfi_restore (lr)
	BX	(lr)

	cfi_endproc
	.fnend
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
	.fnstart
	.align 2
_dl_tlsdesc_dynamic:
	/* Our calling convention is to clobber r0, r1 and the processor
	   flags.  All others that are modified must be saved */
	.save	{r2,r3,r4,lr}
	stmdb   sp!, {r2,r3,r4,lr}
	cfi_adjust_cfa_offset (16)
	cfi_rel_offset (r2,0)
	cfi_rel_offset (r3,4)
	cfi_rel_offset (r4,8)
	cfi_rel_offset (lr,12)
	ldr	r1, [r0] /* td */
	bl	__aeabi_read_tp
	mov	r4, r0 /* r4 = tp */
	ldr	r0, [r0]
	ldr	r2, [r1, #8] /* gen_count */
	ldr	r3, [r0]
	cmp	r2, r3
	bhi	1f
	ldr	r3, [r1]
	ldr	r2, [r0, r3, lsl #3]
	cmn	r2, #1
	ldrne	r3, [r1, #4]
	addne	r3, r2, r3
	rsbne	r0, r4, r3
	bne	2f
1:	mov	r0, r1
	bl	__tls_get_addr
	rsb	r0, r4, r0
2:	ldmia	sp!, {r2,r3,r4, lr}
	cfi_adjust_cfa_offset (-16)
	cfi_restore (lr)
	cfi_restore (r4)
	cfi_restore (r3)
	cfi_restore (r2)
	BX      (lr)
	.fnend
	cfi_endproc
	.size	_dl_tlsdesc_dynamic, .-_dl_tlsdesc_dynamic
#endif /* SHARED */

/* lazy resolved for tls descriptors.  */
	.hidden _dl_tlsdesc_lazy_resolver
	.global	_dl_tlsdesc_lazy_resolver
	.type	_dl_tlsdesc_lazy_resolver,#function
	cfi_startproc
	.fnstart
	.align 2
_dl_tlsdesc_lazy_resolver:
	/* r0 points at the tlsdesc,
	   r1 points at the GOT
	   r2 was pushed by the trampoline and used as a temp,
	      we need to pop it here.
	  We push the remaining call-clobbered registers here, and also
	  R1 -- to keep the stack correctly aligned.  */
	/* Tell the unwinder that r2 has already been pushed.  */
	.save	{r2}
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r2, 0)
	.save	{r0,r1,r3,ip,lr}
	stmdb	sp!, {r0, r1, r3, ip, lr}
	cfi_adjust_cfa_offset (20)
	cfi_rel_offset (r0, 0)
	cfi_rel_offset (r1, 4)
	cfi_rel_offset (r3, 8)
	cfi_rel_offset (ip, 12)
	cfi_rel_offset (lr, 16)
	bl	_dl_tlsdesc_lazy_resolver_fixup
	ldmia	sp!, {r0, r1, r3, ip, lr}
	cfi_adjust_cfa_offset (-20)
	cfi_restore (lr)
	cfi_restore (ip)
	cfi_restore (r3)
	cfi_restore (r1)
	cfi_restore (r0)
	ldmia	sp!, {r2}
	cfi_adjust_cfa_offset (-4)
	cfi_restore (r2)
	ldr	r1, [r0, #4]
	BX	(r1)
	.fnend
	cfi_endproc
	.size	_dl_tlsdesc_lazy_resolver, .-_dl_tlsdesc_lazy_resolver

/* Holder for lazy tls descriptors being resolve in another thread.
   Same ABI as the lazy resolver itself.  */
	.hidden _dl_tlsdesc_resolve_hold
	.global	_dl_tlsdesc_resolve_hold
	.type	_dl_tlsdesc_resolve_hold,#function
	cfi_startproc
	.fnstart
	.align 2
_dl_tlsdesc_resolve_hold:
	/* Tell the unwinder that r2 has already been pushed.  */
	.save	{r2}
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r2, 0)
	.save	{r0,r1,r3,ip,lr}
	stmdb   sp!, {r0, r1, r3, ip, lr}
	cfi_adjust_cfa_offset (20)
	cfi_rel_offset (r0, 0)
	cfi_rel_offset (r1, 4)
	cfi_rel_offset (r3, 8)
	cfi_rel_offset (ip, 12)
	cfi_rel_offset (lr, 16)
	adr	r2, _dl_tlsdesc_resolve_hold
	bl	_dl_tlsdesc_resolve_hold_fixup
	ldmia   sp!, {r0, r1, r3, ip, lr}
	cfi_adjust_cfa_offset (-20)
	cfi_restore (lr)
	cfi_restore (ip)
	cfi_restore (r3)
	cfi_restore (r1)
	cfi_restore (r0)
	ldmia   sp!, {r2}
	cfi_adjust_cfa_offset (-4)
	cfi_restore (r2)
	ldr     r1, [r0, #4]
	BX      (r1)
	.fnend
	cfi_endproc
	.size	_dl_tlsdesc_resolve_hold, .-_dl_tlsdesc_resolve_hold
