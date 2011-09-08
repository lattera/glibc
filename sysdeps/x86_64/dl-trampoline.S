/* PLT trampolines.  x86-64 version.
   Copyright (C) 2004, 2005, 2007, 2009, 2011 Free Software Foundation, Inc.
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
	cfi_adjust_cfa_offset(16) # Incorporate PLT
	subq $56,%rsp
	cfi_adjust_cfa_offset(56)
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
	cfi_adjust_cfa_offset(16) # Incorporate PLT
	/* The La_x86_64_regs data structure pointed to by the
	   fourth paramater must be 16-byte aligned.  This must
	   be explicitly enforced.  We have the set up a dynamically
	   sized stack frame.  %rbx points to the top half which
	   has a fixed size and preserves the original stack pointer.  */

	subq $32, %rsp		# Allocate the local storage.
	cfi_adjust_cfa_offset(32)
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
# else
	subq $LR_SIZE, %rsp		# sizeof(La_x86_64_regs)
# endif
	movq %rsp, 24(%rbx)

	/* Fill the La_x86_64_regs structure.  */
	movq %rdx, LR_RDX_OFFSET(%rsp)
	movq %r8,  LR_R8_OFFSET(%rsp)
	movq %r9,  LR_R9_OFFSET(%rsp)
	movq %rcx, LR_RCX_OFFSET(%rsp)
	movq %rsi, LR_RSI_OFFSET(%rsp)
	movq %rdi, LR_RDI_OFFSET(%rsp)
	movq %rbp, LR_RBP_OFFSET(%rsp)

	leaq 48(%rbx), %rax
	movq %rax, LR_RSP_OFFSET(%rsp)

	/* We always store the XMM registers even if AVX is available.
	   This is to provide backward binary compatility for existing
	   audit modules.  */
	movaps %xmm0,		   (LR_XMM_OFFSET)(%rsp)
	movaps %xmm1, (LR_XMM_OFFSET +   XMM_SIZE)(%rsp)
	movaps %xmm2, (LR_XMM_OFFSET + XMM_SIZE*2)(%rsp)
	movaps %xmm3, (LR_XMM_OFFSET + XMM_SIZE*3)(%rsp)
	movaps %xmm4, (LR_XMM_OFFSET + XMM_SIZE*4)(%rsp)
	movaps %xmm5, (LR_XMM_OFFSET + XMM_SIZE*5)(%rsp)
	movaps %xmm6, (LR_XMM_OFFSET + XMM_SIZE*6)(%rsp)
	movaps %xmm7, (LR_XMM_OFFSET + XMM_SIZE*7)(%rsp)

# ifdef HAVE_AVX_SUPPORT
	.data
L(have_avx):
	.zero 4
	.size L(have_avx), 4
	.previous

	cmpl	$0, L(have_avx)(%rip)
	jne	1f
	movq	%rbx, %r11		# Save rbx
	movl	$1, %eax
	cpuid
	movq	%r11,%rbx		# Restore rbx
	xorl	%eax, %eax
	// AVX and XSAVE supported?
	andl	$((1 << 28) | (1 << 27)), %ecx
	cmpl	$((1 << 28) | (1 << 27)), %ecx
	jne	2f
	xorl	%ecx, %ecx
	// Get XFEATURE_ENABLED_MASK
	xgetbv
	andl	$0x6, %eax
2:	subl	$0x5, %eax
	movl	%eax, L(have_avx)(%rip)
	cmpl	$0, %eax

1:	js	L(no_avx)

#  define RESTORE_AVX
#  define MORE_CODE
#  include "dl-trampoline.h"

	.align 16
L(no_avx):
# endif

# undef RESTORE_AVX
# include "dl-trampoline.h"

	cfi_endproc
	.size _dl_runtime_profile, .-_dl_runtime_profile
#endif


#ifdef SHARED
	.globl _dl_x86_64_save_sse
	.type _dl_x86_64_save_sse, @function
	.align 16
	cfi_startproc
_dl_x86_64_save_sse:
# ifdef HAVE_AVX_SUPPORT
	cmpl	$0, L(have_avx)(%rip)
	jne	1f
	movq	%rbx, %r11		# Save rbx
	movl	$1, %eax
	cpuid
	movq	%r11,%rbx		# Restore rbx
	xorl	%eax, %eax
	// AVX and XSAVE supported?
	andl	$((1 << 28) | (1 << 27)), %ecx
	cmpl	$((1 << 28) | (1 << 27)), %ecx
	jne	2f
	xorl	%ecx, %ecx
	// Get XFEATURE_ENABLED_MASK
	xgetbv
	andl	$0x6, %eax
	cmpl	$0x6, %eax
	// Nonzero if SSE and AVX state saving is enabled.
	sete	%al
2:	leal	-1(%eax,%eax), %eax
	movl	%eax, L(have_avx)(%rip)
	cmpl	$0, %eax

1:	js	L(no_avx5)

#  define YMM_SIZE 32
	vmovdqa %ymm0, %fs:RTLD_SAVESPACE_SSE+0*YMM_SIZE
	vmovdqa %ymm1, %fs:RTLD_SAVESPACE_SSE+1*YMM_SIZE
	vmovdqa %ymm2, %fs:RTLD_SAVESPACE_SSE+2*YMM_SIZE
	vmovdqa %ymm3, %fs:RTLD_SAVESPACE_SSE+3*YMM_SIZE
	vmovdqa %ymm4, %fs:RTLD_SAVESPACE_SSE+4*YMM_SIZE
	vmovdqa %ymm5, %fs:RTLD_SAVESPACE_SSE+5*YMM_SIZE
	vmovdqa %ymm6, %fs:RTLD_SAVESPACE_SSE+6*YMM_SIZE
	vmovdqa %ymm7, %fs:RTLD_SAVESPACE_SSE+7*YMM_SIZE
	ret
L(no_avx5):
# endif
	movdqa	%xmm0, %fs:RTLD_SAVESPACE_SSE+0*XMM_SIZE
	movdqa	%xmm1, %fs:RTLD_SAVESPACE_SSE+1*XMM_SIZE
	movdqa	%xmm2, %fs:RTLD_SAVESPACE_SSE+2*XMM_SIZE
	movdqa	%xmm3, %fs:RTLD_SAVESPACE_SSE+3*XMM_SIZE
	movdqa	%xmm4, %fs:RTLD_SAVESPACE_SSE+4*XMM_SIZE
	movdqa	%xmm5, %fs:RTLD_SAVESPACE_SSE+5*XMM_SIZE
	movdqa	%xmm6, %fs:RTLD_SAVESPACE_SSE+6*XMM_SIZE
	movdqa	%xmm7, %fs:RTLD_SAVESPACE_SSE+7*XMM_SIZE
	ret
	cfi_endproc
	.size _dl_x86_64_save_sse, .-_dl_x86_64_save_sse


	.globl _dl_x86_64_restore_sse
	.type _dl_x86_64_restore_sse, @function
	.align 16
	cfi_startproc
_dl_x86_64_restore_sse:
# ifdef HAVE_AVX_SUPPORT
	cmpl	$0, L(have_avx)(%rip)
	js	L(no_avx6)

	vmovdqa %fs:RTLD_SAVESPACE_SSE+0*YMM_SIZE, %ymm0
	vmovdqa %fs:RTLD_SAVESPACE_SSE+1*YMM_SIZE, %ymm1
	vmovdqa %fs:RTLD_SAVESPACE_SSE+2*YMM_SIZE, %ymm2
	vmovdqa %fs:RTLD_SAVESPACE_SSE+3*YMM_SIZE, %ymm3
	vmovdqa %fs:RTLD_SAVESPACE_SSE+4*YMM_SIZE, %ymm4
	vmovdqa %fs:RTLD_SAVESPACE_SSE+5*YMM_SIZE, %ymm5
	vmovdqa %fs:RTLD_SAVESPACE_SSE+6*YMM_SIZE, %ymm6
	vmovdqa %fs:RTLD_SAVESPACE_SSE+7*YMM_SIZE, %ymm7
	ret
L(no_avx6):
# endif
	movdqa	%fs:RTLD_SAVESPACE_SSE+0*XMM_SIZE, %xmm0
	movdqa	%fs:RTLD_SAVESPACE_SSE+1*XMM_SIZE, %xmm1
	movdqa	%fs:RTLD_SAVESPACE_SSE+2*XMM_SIZE, %xmm2
	movdqa	%fs:RTLD_SAVESPACE_SSE+3*XMM_SIZE, %xmm3
	movdqa	%fs:RTLD_SAVESPACE_SSE+4*XMM_SIZE, %xmm4
	movdqa	%fs:RTLD_SAVESPACE_SSE+5*XMM_SIZE, %xmm5
	movdqa	%fs:RTLD_SAVESPACE_SSE+6*XMM_SIZE, %xmm6
	movdqa	%fs:RTLD_SAVESPACE_SSE+7*XMM_SIZE, %xmm7
	ret
	cfi_endproc
	.size _dl_x86_64_restore_sse, .-_dl_x86_64_restore_sse
#endif
