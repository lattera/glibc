/* PLT trampolines.  x86-64 version.
   Copyright (C) 2004, 2005, 2007 Free Software Foundation, Inc.
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
	movq %rsi, %r11		# Multiply by 24
	addq %r11, %rsi
	addq %r11, %rsi
	shlq $3, %rsi
	movq 56(%rsp), %rdi	# %rdi: link_map, %rsi: reloc_offset
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
	subq $88, %rsp
	cfi_adjust_cfa_offset(104) # Incorporate PLT
	movq %rax, (%rsp)	# Preserve registers otherwise clobbered.
	movq %rdx, 8(%rsp)
	movq %r8, 16(%rsp)
	movq %r9, 24(%rsp)
	movq %rcx, 32(%rsp)
	movq %rsi, 40(%rsp)
	movq %rdi, 48(%rsp)
	movq %rbp, 56(%rsp)	# Information for auditors.
	leaq 104(%rsp), %rax
	movq %rax, 64(%rsp)
	leaq 8(%rsp), %rcx
	movq 104(%rsp), %rdx	# Load return address if needed
	movq 96(%rsp), %rsi	# Copy args pushed by PLT in register.
	movq %rsi,%r11		# Multiply by 24
	addq %r11,%rsi
	addq %r11,%rsi
	shlq $3, %rsi
	movq 88(%rsp), %rdi	# %rdi: link_map, %rsi: reloc_offset
	leaq 72(%rsp), %r8
	call _dl_profile_fixup	# Call resolver.
	movq %rax, %r11		# Save return value
	movq 8(%rsp), %rdx	# Get back register content.
	movq 16(%rsp), %r8
	movq 24(%rsp), %r9
	movq (%rsp),%rax
	movq 72(%rsp), %r10
	testq %r10, %r10
	jns 1f
	movq 32(%rsp), %rcx
	movq 40(%rsp), %rsi
	movq 48(%rsp), %rdi
	addq $104,%rsp		# Adjust stack
	cfi_adjust_cfa_offset (-104)
	jmp *%r11		# Jump to function address.

	/*
	    +104     return address
	    +96     PLT2
	    +88     PLT1
	    +80     free
	    +72     free
	    +64     %rsp
	    +56     %rbp
	    +48     %rdi
	    +40     %rsi
	    +32     %rcx
	    +24     %r9
	    +16     %r8
	    +8      %rdx
	   %rsp     %rax
	*/
	cfi_adjust_cfa_offset (104)
1:	movq %rbx, 72(%rsp)
	cfi_rel_offset (rbx, 72)
	leaq 112(%rsp), %rsi
	movq %rsp, %rbx
	cfi_def_cfa_register (%rbx)
	movq %r10, %rcx
	addq $8, %r10
	andq $0xfffffffffffffff0, %r10
	subq %r10, %rsp
	movq %rsp, %rdi
	shrq $3, %rcx
	rep
	movsq
	movq 32(%rbx), %rcx
	movq 40(%rbx), %rsi
	movq 48(%rbx), %rdi
	call *%r11
	movq %rbx, %rsp
	cfi_def_cfa_register (%rsp)
	subq $72, %rsp
	cfi_adjust_cfa_offset (72)
	movq %rsp, %rcx
	movq %rax, (%rcx)
	movq %rdx, 8(%rcx)
	/* Even though the stack is correctly aligned to allow using movaps
	   we use movups.  Some callers might provide an incorrectly aligned
	   stack and we do not want to have it blow up here.  */
	movups %xmm0, 16(%rcx)
	movups %xmm1, 32(%rcx)
	fstpt 48(%rcx)
	fstpt 64(%rcx)
	/*
	    +176    return address
	    +168    PLT2
	    +160    PLT1
	    +152    free
	    +144    free
	    +136    %rsp
	    +128    %rbp
	    +120    %rdi
	    +112    %rsi
	    +104    %rcx
	    +96     %r9
	    +88     %r8
	    +80     %rdx
	    +64     %st1 result
	    +48     %st result
	    +32     %xmm1 result
	    +16     %xmm0 result
	    +8      %rdx result
	   %rsp     %rax result
	*/
	leaq 80(%rsp), %rdx
	movq 144(%rsp), %rbx
	cfi_restore (rbx)
	movq 168(%rsp), %rsi	# Copy args pushed by PLT in register.
	movq %rsi,%r11		# Multiply by 24
	addq %r11,%rsi
	addq %r11,%rsi
	shlq $3, %rsi
	movq 160(%rsp), %rdi	# %rdi: link_map, %rsi: reloc_offset
	call _dl_call_pltexit
	movq (%rsp), %rax
	movq 8(%rsp), %rdx
	movups 16(%rsp), %xmm0
	movups 32(%rsp), %xmm1
	fldt 64(%rsp)
	fldt 48(%rsp)
	addq $176, %rsp
	cfi_adjust_cfa_offset (-176)
	retq
	cfi_endproc
	.size _dl_runtime_profile, .-_dl_runtime_profile
#endif
