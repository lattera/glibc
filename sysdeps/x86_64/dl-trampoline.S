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
	movl	$1, %eax
	testl	$(1 << 28), %ecx
	jne	2f
	negl	%eax
2:	movl	%eax, L(have_avx)(%rip)
	cmpl	$0, %eax

1:	js	L(no_avx1)

	/* This is to support AVX audit modules.  */
	vmovdqu %ymm0,		      (LR_VECTOR_OFFSET)(%rsp)
	vmovdqu %ymm1, (LR_VECTOR_OFFSET +   VECTOR_SIZE)(%rsp)
	vmovdqu %ymm2, (LR_VECTOR_OFFSET + VECTOR_SIZE*2)(%rsp)
	vmovdqu %ymm3, (LR_VECTOR_OFFSET + VECTOR_SIZE*3)(%rsp)
	vmovdqu %ymm4, (LR_VECTOR_OFFSET + VECTOR_SIZE*4)(%rsp)
	vmovdqu %ymm5, (LR_VECTOR_OFFSET + VECTOR_SIZE*5)(%rsp)
	vmovdqu %ymm6, (LR_VECTOR_OFFSET + VECTOR_SIZE*6)(%rsp)
	vmovdqu %ymm7, (LR_VECTOR_OFFSET + VECTOR_SIZE*7)(%rsp)

	/* Save xmm0-xmm7 registers to detect if any of them are
	   changed by audit module.  */
	vmovdqa %xmm0,		    (LR_SIZE)(%rsp)
	vmovdqa %xmm1, (LR_SIZE +   XMM_SIZE)(%rsp)
	vmovdqa %xmm2, (LR_SIZE + XMM_SIZE*2)(%rsp)
	vmovdqa %xmm3, (LR_SIZE + XMM_SIZE*3)(%rsp)
	vmovdqa %xmm4, (LR_SIZE + XMM_SIZE*4)(%rsp)
	vmovdqa %xmm5, (LR_SIZE + XMM_SIZE*5)(%rsp)
	vmovdqa %xmm6, (LR_SIZE + XMM_SIZE*6)(%rsp)
	vmovdqa %xmm7, (LR_SIZE + XMM_SIZE*7)(%rsp)

L(no_avx1):
# endif

	movq %rsp, %rcx		# La_x86_64_regs pointer to %rcx.
	movq 48(%rbx), %rdx	# Load return address if needed.
	movq 40(%rbx), %rsi	# Copy args pushed by PLT in register.
	movq 32(%rbx), %rdi	# %rdi: link_map, %rsi: reloc_index
	leaq 16(%rbx), %r8
	call _dl_profile_fixup	# Call resolver.

	movq %rax, %r11		# Save return value.

	movq 8(%rbx), %rax	# Get back register content.
	movq LR_RDX_OFFSET(%rsp), %rdx
	movq  LR_R8_OFFSET(%rsp), %r8
	movq  LR_R9_OFFSET(%rsp), %r9

	movaps		    (LR_XMM_OFFSET)(%rsp), %xmm0
	movaps	 (LR_XMM_OFFSET + XMM_SIZE)(%rsp), %xmm1
	movaps (LR_XMM_OFFSET + XMM_SIZE*2)(%rsp), %xmm2
	movaps (LR_XMM_OFFSET + XMM_SIZE*3)(%rsp), %xmm3
	movaps (LR_XMM_OFFSET + XMM_SIZE*4)(%rsp), %xmm4
	movaps (LR_XMM_OFFSET + XMM_SIZE*5)(%rsp), %xmm5
	movaps (LR_XMM_OFFSET + XMM_SIZE*6)(%rsp), %xmm6
	movaps (LR_XMM_OFFSET + XMM_SIZE*7)(%rsp), %xmm7

# ifdef HAVE_AVX_SUPPORT
	cmpl	$0, L(have_avx)(%rip)
	js	L(no_avx2)

	/* Check if any xmm0-xmm7 registers are changed by audit
	   module.  */
	vpcmpeqq (LR_SIZE)(%rsp), %xmm0, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	jne 1f
	vmovdqu			(LR_VECTOR_OFFSET)(%rsp), %ymm0

1:	vpcmpeqq (LR_SIZE + XMM_SIZE)(%rsp), %xmm1, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	jne 1f
	vmovdqu	  (LR_VECTOR_OFFSET + VECTOR_SIZE)(%rsp), %ymm1

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*2)(%rsp), %xmm2, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	jne 1f
	vmovdqu (LR_VECTOR_OFFSET + VECTOR_SIZE*2)(%rsp), %ymm2

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*3)(%rsp), %xmm3, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	jne 1f
	vmovdqu (LR_VECTOR_OFFSET + VECTOR_SIZE*3)(%rsp), %ymm3

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*4)(%rsp), %xmm4, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	jne 1f
	vmovdqu (LR_VECTOR_OFFSET + VECTOR_SIZE*4)(%rsp), %ymm4

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*5)(%rsp), %xmm5, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	jne 1f
	vmovdqu (LR_VECTOR_OFFSET + VECTOR_SIZE*5)(%rsp), %ymm5

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*6)(%rsp), %xmm6, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	jne 1f
	vmovdqu (LR_VECTOR_OFFSET + VECTOR_SIZE*6)(%rsp), %ymm6

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*7)(%rsp), %xmm7, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	jne 1f
	vmovdqu (LR_VECTOR_OFFSET + VECTOR_SIZE*7)(%rsp), %ymm7

L(no_avx2):
1:
# endif
	movq 16(%rbx), %r10	# Anything in framesize?
	testq %r10, %r10
	jns 3f

	/* There's nothing in the frame size, so there
	   will be no call to the _dl_call_pltexit. */

	/* Get back registers content.  */
	movq LR_RCX_OFFSET(%rsp), %rcx
	movq LR_RSI_OFFSET(%rsp), %rsi
	movq LR_RDI_OFFSET(%rsp), %rdi

	movq %rbx, %rsp
	movq (%rsp), %rbx
	cfi_restore(rbx)
	cfi_def_cfa_register(%rsp)

	addq $48, %rsp		# Adjust the stack to the return value
				# (eats the reloc index and link_map)
	cfi_adjust_cfa_offset(-48)
	jmp *%r11		# Jump to function address.

3:
	cfi_adjust_cfa_offset(48)
	cfi_rel_offset(%rbx, 0)
	cfi_def_cfa_register(%rbx)

	/* At this point we need to prepare new stack for the function
	   which has to be called.  We copy the original stack to a
	   temporary buffer of the size specified by the 'framesize'
	   returned from _dl_profile_fixup */

	leaq LR_RSP_OFFSET(%rbx), %rsi	# stack
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
# ifdef HAVE_AVX_SUPPORT
	/* sizeof(La_x86_64_retval).  Need extra space for 2 SSE
	   registers to detect if xmm0/xmm1 registers are changed
	   by audit module.  */
	subq $(LRV_SIZE + XMM_SIZE*2), %rsp
# else
	subq $LRV_SIZE, %rsp	# sizeof(La_x86_64_retval)
# endif
	movq %rsp, %rcx		# La_x86_64_retval argument to %rcx.

	/* Fill in the La_x86_64_retval structure.  */
	movq %rax, LRV_RAX_OFFSET(%rcx)
	movq %rdx, LRV_RDX_OFFSET(%rcx)

	movaps %xmm0, LRV_XMM0_OFFSET(%rcx)
	movaps %xmm1, LRV_XMM1_OFFSET(%rcx)

# ifdef HAVE_AVX_SUPPORT
	cmpl	$0, L(have_avx)(%rip)
	js	L(no_avx3)

	/* This is to support AVX audit modules.  */
	vmovdqu %ymm0, LRV_VECTOR0_OFFSET(%rcx)
	vmovdqu %ymm1, LRV_VECTOR1_OFFSET(%rcx)

	/* Save xmm0/xmm1 registers to detect if they are changed
	   by audit module.  */
	vmovdqa %xmm0,		  (LRV_SIZE)(%rcx)
	vmovdqa %xmm1, (LRV_SIZE + XMM_SIZE)(%rcx)

L(no_avx3):
# endif

	fstpt LRV_ST0_OFFSET(%rcx)
	fstpt LRV_ST1_OFFSET(%rcx)

	movq 24(%rbx), %rdx	# La_x86_64_regs argument to %rdx.
	movq 40(%rbx), %rsi	# Copy args pushed by PLT in register.
        movq 32(%rbx), %rdi	# %rdi: link_map, %rsi: reloc_index
	call _dl_call_pltexit

	/* Restore return registers.  */
	movq LRV_RAX_OFFSET(%rsp), %rax
	movq LRV_RDX_OFFSET(%rsp), %rdx

	movaps LRV_XMM0_OFFSET(%rsp), %xmm0
	movaps LRV_XMM1_OFFSET(%rsp), %xmm1

# ifdef HAVE_AVX_SUPPORT
	cmpl	$0, L(have_avx)(%rip)
	js	L(no_avx4)

	/* Check if xmm0/xmm1 registers are changed by audit module.  */
	vpcmpeqq (LRV_SIZE)(%rsp), %xmm0, %xmm2
	vpmovmskb %xmm2, %esi
	cmpl $0xffff, %esi
	jne 1f
	vmovdqu LRV_VECTOR0_OFFSET(%rsp), %ymm0

1:	vpcmpeqq (LRV_SIZE + XMM_SIZE)(%rsp), %xmm1, %xmm2
	vpmovmskb %xmm2, %esi
	cmpl $0xffff, %esi
	jne 1f
	vmovdqu LRV_VECTOR1_OFFSET(%rsp), %ymm1

L(no_avx4):
1:
# endif

	fldt LRV_ST1_OFFSET(%rsp)
	fldt LRV_ST0_OFFSET(%rsp)

	movq %rbx, %rsp
	movq (%rsp), %rbx
	cfi_restore(rbx)
	cfi_def_cfa_register(%rsp)

	addq $48, %rsp		# Adjust the stack to the return value
				# (eats the reloc index and link_map)
	cfi_adjust_cfa_offset(-48)
	retq

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
	movl	$1, %eax
	testl	$(1 << 28), %ecx
	jne	2f
	negl	%eax
2:	movl	%eax, L(have_avx)(%rip)
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
# define YMM_SIZE 16
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
