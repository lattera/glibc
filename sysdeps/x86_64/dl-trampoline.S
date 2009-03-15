/* PLT trampolines.  x86-64 version.
   Copyright (C) 2004, 2005, 2007, 2009 Free Software Foundation, Inc.
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

	.text
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, @function
	.align 16
	cfi_startproc
_dl_runtime_resolve:
	subq $56,%rsp
	cfi_adjust_cfa_offset(72) # Incorporate PLT
	movq %rax,(%rsp)	# Preserve registers otherwise clobbered.
	movq %rcx, 8(%rsp)
	movq %rdx, 16(%rsp)
	movq %rsi, 24(%rsp)
	movq %rdi, 32(%rsp)
	movq %r8, 40(%rsp)
	movq %r9, 48(%rsp)
	movq 64(%rsp), %rsi	# Copy args pushed by PLT in register.
	movq 56(%rsp), %rdi	# %rdi: link_map, %rsi: reloc_index
	call _dl_fixup		# Call resolver.
	movq %rax, %r11		# Save return value
	movq 48(%rsp), %r9	# Get register content back.
	movq 40(%rsp), %r8
	movq 32(%rsp), %rdi
	movq 24(%rsp), %rsi
	movq 16(%rsp), %rdx
	movq 8(%rsp), %rcx
	movq (%rsp), %rax
	addq $72, %rsp		# Adjust stack(PLT did 2 pushes)
	cfi_adjust_cfa_offset(-72)
	jmp *%r11		# Jump to function address.
	cfi_endproc
	.size _dl_runtime_resolve, .-_dl_runtime_resolve


#ifndef PROF
	.globl _dl_runtime_profile
	.type _dl_runtime_profile, @function
	.align 16
	cfi_startproc

_dl_runtime_profile:
	/* The La_x86_64_regs data structure pointed to by the
	   fourth paramater must be 16-byte aligned.  This must
	   be explicitly enforced.  We have the set up a dynamically
	   sized stack frame.  %rbx points to the top half which
	   has a fixed size and preserves the original stack pointer.  */

	subq $32, %rsp		# Allocate the local storage.
	cfi_adjust_cfa_offset(48) # Incorporate PLT
	movq %rbx, (%rsp)
	cfi_rel_offset(%rbx, 0)

	/* On the stack:
		56(%rbx)	parameter #1
		48(%rbx)	return address

		40(%rbx)	reloc index
		32(%rbx)	link_map

		24(%rbx)	La_x86_64_regs pointer
		16(%rbx)	framesize
		 8(%rbx)	rax
		  (%rbx)	rbx
	*/

	movq %rax, 8(%rsp)
	movq %rsp, %rbx
	cfi_def_cfa_register(%rbx)

	/* Actively align the La_x86_64_regs structure.  */
	andq $0xfffffffffffffff0, %rsp
	subq $192, %rsp		# sizeof(La_x86_64_regs)
	movq %rsp, 24(%rbx)

	movq %rdx,   (%rsp)	# Fill the La_x86_64_regs structure.
	movq %r8,   8(%rsp)
	movq %r9,  16(%rsp)
	movq %rcx, 24(%rsp)
	movq %rsi, 32(%rsp)
	movq %rdi, 40(%rsp)
	movq %rbp, 48(%rsp)
	leaq 48(%rbx), %rax
	movq %rax, 56(%rsp)
	movaps %xmm0,  64(%rsp)
	movaps %xmm1,  80(%rsp)
	movaps %xmm2,  96(%rsp)
	movaps %xmm3, 112(%rsp)
	movaps %xmm4, 128(%rsp)
	movaps %xmm5, 144(%rsp)
	movaps %xmm7, 160(%rsp)

	movq %rsp, %rcx		# La_x86_64_regs pointer to %rcx.
	movq 48(%rbx), %rdx	# Load return address if needed.
	movq 40(%rbx), %rsi	# Copy args pushed by PLT in register.
	movq 32(%rbx), %rdi	# %rdi: link_map, %rsi: reloc_index
	leaq 16(%rbx), %r8
	call _dl_profile_fixup	# Call resolver.

	movq %rax, %r11		# Save return value.

	movq 8(%rbx), %rax	# Get back register content.
	movq      (%rsp), %rdx
	movq     8(%rsp), %r8
	movq    16(%rsp), %r9
	movaps  64(%rsp), %xmm0
	movaps  80(%rsp), %xmm1
	movaps  96(%rsp), %xmm2
	movaps 112(%rsp), %xmm3
	movaps 128(%rsp), %xmm4
	movaps 144(%rsp), %xmm5
	movaps 160(%rsp), %xmm7

	movq 16(%rbx), %r10	# Anything in framesize?
	testq %r10, %r10
	jns 1f

	/* There's nothing in the frame size, so there
	   will be no call to the _dl_call_pltexit. */

	movq 24(%rsp), %rcx	# Get back registers content.
	movq 32(%rsp), %rsi
	movq 40(%rsp), %rdi

	movq %rbx, %rsp
	movq (%rsp), %rbx
	cfi_restore(rbx)
	cfi_def_cfa_register(%rsp)

	addq $48, %rsp		# Adjust the stack to the return value
				# (eats the reloc index and link_map)
	cfi_adjust_cfa_offset(-48)
	jmp *%r11		# Jump to function address.

1:
	cfi_adjust_cfa_offset(48)
	cfi_rel_offset(%rbx, 0)
	cfi_def_cfa_register(%rbx)

	/* At this point we need to prepare new stack for the function
	   which has to be called.  We copy the original stack to a
	   temporary buffer of the size specified by the 'framesize'
	   returned from _dl_profile_fixup */

	leaq 56(%rbx), %rsi	# stack
	addq $8, %r10
	andq $0xfffffffffffffff0, %r10
	movq %r10, %rcx
	subq %r10, %rsp
	movq %rsp, %rdi
	shrq $3, %rcx
	rep
	movsq

	movq 24(%rdi), %rcx	# Get back register content.
	movq 32(%rdi), %rsi
	movq 40(%rdi), %rdi

	call *%r11

	mov 24(%rbx), %rsp	# Drop the copied stack content

	/* Now we have to prepare the La_x86_64_retval structure for the
	   _dl_call_pltexit.  The La_x86_64_regs is being pointed by rsp now,
	   so we just need to allocate the sizeof(La_x86_64_retval) space on
	   the stack, since the alignment has already been taken care of. */

	subq $80, %rsp		# sizeof(La_x86_64_retval)
	movq %rsp, %rcx		# La_x86_64_retval argument to %rcx.

	movq %rax, (%rcx)	# Fill in the La_x86_64_retval structure.
	movq %rdx, 8(%rcx)
	movaps %xmm0, 16(%rcx)
	movaps %xmm1, 32(%rcx)
	fstpt 48(%rcx)
	fstpt 64(%rcx)

	movq 24(%rbx), %rdx	# La_x86_64_regs argument to %rdx.
	movq 40(%rbx), %rsi	# Copy args pushed by PLT in register.
        movq 32(%rbx), %rdi	# %rdi: link_map, %rsi: reloc_index
	call _dl_call_pltexit

	movq  (%rsp), %rax	# Restore return registers.
	movq 8(%rsp), %rdx
	movaps 16(%rsp), %xmm0
	movaps 32(%rsp), %xmm1
	fldt 64(%rsp)
	fldt 48(%rsp)

	movq %rbx, %rsp
	movq  (%rsp), %rbx
	cfi_restore(rbx)
	cfi_def_cfa_register(%rsp)

	addq $48, %rsp		# Adjust the stack to the return value
				# (eats the reloc index and link_map)
	cfi_adjust_cfa_offset(-48)
	retq

	cfi_endproc
	.size _dl_runtime_profile, .-_dl_runtime_profile
#endif
