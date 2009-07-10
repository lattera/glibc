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

#include <config.h>
#include <sysdep.h>
#include <link-defines.h>

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
# ifdef HAVE_AVX_SUPPORT
	/* sizeof(La_x86_64_regs).  Need extra space for 8 SSE registers
	   to detect if any xmm0-xmm7 registers are changed by audit
	   module.  */
	subq $(LR_SIZE + XMM_SIZE*8), %rsp
#else
	subq $LR_SIZE, %rsp		# sizeof(La_x86_64_regs)
#endif
	movq %rsp, 24(%rbx)

	/* Fill the La_x86_64_regs structure.  */
	movq %rdx, LR_RDX_OFFSET(%rsp)
	movq %r8,  LR_R8_OFFSET(%rsp)
	movq %r9,  LR_R9_OFFSET(%rsp)
	movq %rcx, LR_RCX_OFFSET(%rsp)
	movq %rsi, LR_RSI_OFFSET(%rsp)
	movq %rdi, LR_RDI_OFFSET(%rsp)
	movq %rbp, LR_RBP_OFFSET(%rsp)

# ifdef HAVE_AVX_SUPPORT
	jmp *L(save_and_restore_vector)(%rip)

	.align 16
L(save_and_restore_vector_sse):
# endif

# define MOVXMM movaps
# include "dl-trampoline.h"

# ifdef HAVE_AVX_SUPPORT
#  undef  MOVXMM
#  define MOVXMM vmovdqa
#  define RESTORE_AVX
	.align 16
L(save_and_restore_vector_avx):
#  include "dl-trampoline.h"
# endif

	cfi_endproc
	.size _dl_runtime_profile, .-_dl_runtime_profile

# ifdef HAVE_AVX_SUPPORT
L(check_avx):
	mov	%rbx,%r11		# Save rbx
	movl	$1, %eax
	cpuid
	mov	%r11,%rbx		# Restore rbx
	leaq    L(save_and_restore_vector_sse)(%rip), %rax
	andl	$(1 << 28), %ecx	# Check if AVX is available.
	jz	L(ret)
	leaq    L(save_and_restore_vector_avx)(%rip), %rax
L(ret):
	movq	%rax,L(save_and_restore_vector)(%rip)
	jmp	*%rax

	.section .data.rel.local,"aw",@progbits
	.align	8
L(save_and_restore_vector):
	.quad L(check_avx)
# endif
#endif
