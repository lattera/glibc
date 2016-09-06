/* PLT trampolines.  x86-64 version.
   Copyright (C) 2009-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#undef REGISTER_SAVE_AREA_RAW
#ifdef __ILP32__
/* X32 saves RCX, RDX, RSI, RDI, R8 and R9 plus RAX as well as VEC0 to
   VEC7.  */
# define REGISTER_SAVE_AREA_RAW	(8 * 7 + VEC_SIZE * 8)
#else
/* X86-64 saves RCX, RDX, RSI, RDI, R8 and R9 plus RAX as well as
   BND0, BND1, BND2, BND3 and VEC0 to VEC7. */
# define REGISTER_SAVE_AREA_RAW	(8 * 7 + 16 * 4 + VEC_SIZE * 8)
#endif

#undef REGISTER_SAVE_AREA
#undef LOCAL_STORAGE_AREA
#undef BASE
#if DL_RUNTIME_RESOLVE_REALIGN_STACK
# define REGISTER_SAVE_AREA	(REGISTER_SAVE_AREA_RAW + 8)
/* Local stack area before jumping to function address: RBX.  */
# define LOCAL_STORAGE_AREA	8
# define BASE			rbx
# if (REGISTER_SAVE_AREA % VEC_SIZE) != 0
#  error REGISTER_SAVE_AREA must be multples of VEC_SIZE
# endif
#else
# define REGISTER_SAVE_AREA	REGISTER_SAVE_AREA_RAW
/* Local stack area before jumping to function address:  All saved
   registers.  */
# define LOCAL_STORAGE_AREA	REGISTER_SAVE_AREA
# define BASE			rsp
# if (REGISTER_SAVE_AREA % 16) != 8
#  error REGISTER_SAVE_AREA must be odd multples of 8
# endif
#endif

	.text
#ifdef _dl_runtime_resolve_opt
/* Use the smallest vector registers to preserve the full YMM/ZMM
   registers to avoid SSE transition penalty.  */

# if VEC_SIZE == 32
/* Check if the upper 128 bits in %ymm0 - %ymm7 registers are non-zero
   and preserve %xmm0 - %xmm7 registers with the zero upper bits.  Since
   there is no SSE transition penalty on AVX512 processors which don't
   support XGETBV with ECX == 1, _dl_runtime_resolve_avx512_slow isn't
   provided.   */
	.globl _dl_runtime_resolve_avx_slow
	.hidden _dl_runtime_resolve_avx_slow
	.type _dl_runtime_resolve_avx_slow, @function
	.align 16
_dl_runtime_resolve_avx_slow:
	cfi_startproc
	cfi_adjust_cfa_offset(16) # Incorporate PLT
	vorpd %ymm0, %ymm1, %ymm8
	vorpd %ymm2, %ymm3, %ymm9
	vorpd %ymm4, %ymm5, %ymm10
	vorpd %ymm6, %ymm7, %ymm11
	vorpd %ymm8, %ymm9, %ymm9
	vorpd %ymm10, %ymm11, %ymm10
	vpcmpeqd %xmm8, %xmm8, %xmm8
	vorpd %ymm9, %ymm10, %ymm10
	vptest %ymm10, %ymm8
	# Preserve %ymm0 - %ymm7 registers if the upper 128 bits of any
	# %ymm0 - %ymm7 registers aren't zero.
	PRESERVE_BND_REGS_PREFIX
	jnc _dl_runtime_resolve_avx
	# Use vzeroupper to avoid SSE transition penalty.
	vzeroupper
	# Preserve %xmm0 - %xmm7 registers with the zero upper 128 bits
	# when the upper 128 bits of %ymm0 - %ymm7 registers are zero.
	PRESERVE_BND_REGS_PREFIX
	jmp _dl_runtime_resolve_sse_vex
	cfi_adjust_cfa_offset(-16) # Restore PLT adjustment
	cfi_endproc
	.size _dl_runtime_resolve_avx_slow, .-_dl_runtime_resolve_avx_slow
# endif

/* Use XGETBV with ECX == 1 to check which bits in vector registers are
   non-zero and only preserve the non-zero lower bits with zero upper
   bits.  */
	.globl _dl_runtime_resolve_opt
	.hidden _dl_runtime_resolve_opt
	.type _dl_runtime_resolve_opt, @function
	.align 16
_dl_runtime_resolve_opt:
	cfi_startproc
	cfi_adjust_cfa_offset(16) # Incorporate PLT
	pushq %rax
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%rax, 0)
	pushq %rcx
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%rcx, 0)
	pushq %rdx
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%rdx, 0)
	movl $1, %ecx
	xgetbv
	movl %eax, %r11d
	popq %rdx
	cfi_adjust_cfa_offset(-8)
	cfi_restore (%rdx)
	popq %rcx
	cfi_adjust_cfa_offset(-8)
	cfi_restore (%rcx)
	popq %rax
	cfi_adjust_cfa_offset(-8)
	cfi_restore (%rax)
# if VEC_SIZE == 32
	# For YMM registers, check if YMM state is in use.
	andl $bit_YMM_state, %r11d
	# Preserve %xmm0 - %xmm7 registers with the zero upper 128 bits if
	# YMM state isn't in use.
	PRESERVE_BND_REGS_PREFIX
	jz _dl_runtime_resolve_sse_vex
# elif VEC_SIZE == 64
	# For ZMM registers, check if YMM state and ZMM state are in
	# use.
	andl $(bit_YMM_state | bit_ZMM0_15_state), %r11d
	cmpl $bit_YMM_state, %r11d
	# Preserve %xmm0 - %xmm7 registers with the zero upper 384 bits if
	# neither YMM state nor ZMM state are in use.
	PRESERVE_BND_REGS_PREFIX
	jl _dl_runtime_resolve_sse_vex
	# Preserve %ymm0 - %ymm7 registers with the zero upper 256 bits if
	# ZMM state isn't in use.
	PRESERVE_BND_REGS_PREFIX
	je _dl_runtime_resolve_avx
# else
#  error Unsupported VEC_SIZE!
# endif
	cfi_adjust_cfa_offset(-16) # Restore PLT adjustment
	cfi_endproc
	.size _dl_runtime_resolve_opt, .-_dl_runtime_resolve_opt
#endif
	.globl _dl_runtime_resolve
	.hidden _dl_runtime_resolve
	.type _dl_runtime_resolve, @function
	.align 16
	cfi_startproc
_dl_runtime_resolve:
	cfi_adjust_cfa_offset(16) # Incorporate PLT
#if DL_RUNTIME_RESOLVE_REALIGN_STACK
# if LOCAL_STORAGE_AREA != 8
#  error LOCAL_STORAGE_AREA must be 8
# endif
	pushq %rbx			# push subtracts stack by 8.
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%rbx, 0)
	mov %RSP_LP, %RBX_LP
	cfi_def_cfa_register(%rbx)
	and $-VEC_SIZE, %RSP_LP
#endif
	sub $REGISTER_SAVE_AREA, %RSP_LP
#if !DL_RUNTIME_RESOLVE_REALIGN_STACK
	cfi_adjust_cfa_offset(REGISTER_SAVE_AREA)
#endif
	# Preserve registers otherwise clobbered.
	movq %rax, REGISTER_SAVE_RAX(%rsp)
	movq %rcx, REGISTER_SAVE_RCX(%rsp)
	movq %rdx, REGISTER_SAVE_RDX(%rsp)
	movq %rsi, REGISTER_SAVE_RSI(%rsp)
	movq %rdi, REGISTER_SAVE_RDI(%rsp)
	movq %r8, REGISTER_SAVE_R8(%rsp)
	movq %r9, REGISTER_SAVE_R9(%rsp)
	VMOV %VEC(0), (REGISTER_SAVE_VEC_OFF)(%rsp)
	VMOV %VEC(1), (REGISTER_SAVE_VEC_OFF + VEC_SIZE)(%rsp)
	VMOV %VEC(2), (REGISTER_SAVE_VEC_OFF + VEC_SIZE * 2)(%rsp)
	VMOV %VEC(3), (REGISTER_SAVE_VEC_OFF + VEC_SIZE * 3)(%rsp)
	VMOV %VEC(4), (REGISTER_SAVE_VEC_OFF + VEC_SIZE * 4)(%rsp)
	VMOV %VEC(5), (REGISTER_SAVE_VEC_OFF + VEC_SIZE * 5)(%rsp)
	VMOV %VEC(6), (REGISTER_SAVE_VEC_OFF + VEC_SIZE * 6)(%rsp)
	VMOV %VEC(7), (REGISTER_SAVE_VEC_OFF + VEC_SIZE * 7)(%rsp)
#ifndef __ILP32__
	# We also have to preserve bound registers.  These are nops if
	# Intel MPX isn't available or disabled.
# ifdef HAVE_MPX_SUPPORT
	bndmov %bnd0, REGISTER_SAVE_BND0(%rsp)
	bndmov %bnd1, REGISTER_SAVE_BND1(%rsp)
	bndmov %bnd2, REGISTER_SAVE_BND2(%rsp)
	bndmov %bnd3, REGISTER_SAVE_BND3(%rsp)
# else
#  if REGISTER_SAVE_BND0 == 0
	.byte 0x66,0x0f,0x1b,0x04,0x24
#  else
	.byte 0x66,0x0f,0x1b,0x44,0x24,REGISTER_SAVE_BND0
#  endif
	.byte 0x66,0x0f,0x1b,0x4c,0x24,REGISTER_SAVE_BND1
	.byte 0x66,0x0f,0x1b,0x54,0x24,REGISTER_SAVE_BND2
	.byte 0x66,0x0f,0x1b,0x5c,0x24,REGISTER_SAVE_BND3
# endif
#endif
	# Copy args pushed by PLT in register.
	# %rdi: link_map, %rsi: reloc_index
	mov (LOCAL_STORAGE_AREA + 8)(%BASE), %RSI_LP
	mov LOCAL_STORAGE_AREA(%BASE), %RDI_LP
	call _dl_fixup		# Call resolver.
	mov %RAX_LP, %R11_LP	# Save return value
#ifndef __ILP32__
	# Restore bound registers.  These are nops if Intel MPX isn't
	# avaiable or disabled.
# ifdef HAVE_MPX_SUPPORT
	bndmov REGISTER_SAVE_BND3(%rsp), %bnd3
	bndmov REGISTER_SAVE_BND2(%rsp), %bnd2
	bndmov REGISTER_SAVE_BND1(%rsp), %bnd1
	bndmov REGISTER_SAVE_BND0(%rsp), %bnd0
# else
	.byte 0x66,0x0f,0x1a,0x5c,0x24,REGISTER_SAVE_BND3
	.byte 0x66,0x0f,0x1a,0x54,0x24,REGISTER_SAVE_BND2
	.byte 0x66,0x0f,0x1a,0x4c,0x24,REGISTER_SAVE_BND1
#  if REGISTER_SAVE_BND0 == 0
	.byte 0x66,0x0f,0x1a,0x04,0x24
#  else
	.byte 0x66,0x0f,0x1a,0x44,0x24,REGISTER_SAVE_BND0
#  endif
# endif
#endif
	# Get register content back.
	movq REGISTER_SAVE_R9(%rsp), %r9
	movq REGISTER_SAVE_R8(%rsp), %r8
	movq REGISTER_SAVE_RDI(%rsp), %rdi
	movq REGISTER_SAVE_RSI(%rsp), %rsi
	movq REGISTER_SAVE_RDX(%rsp), %rdx
	movq REGISTER_SAVE_RCX(%rsp), %rcx
	movq REGISTER_SAVE_RAX(%rsp), %rax
	VMOV (REGISTER_SAVE_VEC_OFF)(%rsp), %VEC(0)
	VMOV (REGISTER_SAVE_VEC_OFF + VEC_SIZE)(%rsp), %VEC(1)
	VMOV (REGISTER_SAVE_VEC_OFF + VEC_SIZE * 2)(%rsp), %VEC(2)
	VMOV (REGISTER_SAVE_VEC_OFF + VEC_SIZE * 3)(%rsp), %VEC(3)
	VMOV (REGISTER_SAVE_VEC_OFF + VEC_SIZE * 4)(%rsp), %VEC(4)
	VMOV (REGISTER_SAVE_VEC_OFF + VEC_SIZE * 5)(%rsp), %VEC(5)
	VMOV (REGISTER_SAVE_VEC_OFF + VEC_SIZE * 6)(%rsp), %VEC(6)
	VMOV (REGISTER_SAVE_VEC_OFF + VEC_SIZE * 7)(%rsp), %VEC(7)
#if DL_RUNTIME_RESOLVE_REALIGN_STACK
	mov %RBX_LP, %RSP_LP
	cfi_def_cfa_register(%rsp)
	movq (%rsp), %rbx
	cfi_restore(%rbx)
#endif
	# Adjust stack(PLT did 2 pushes)
	add $(LOCAL_STORAGE_AREA + 16), %RSP_LP
	cfi_adjust_cfa_offset(-(LOCAL_STORAGE_AREA + 16))
	# Preserve bound registers.
	PRESERVE_BND_REGS_PREFIX
	jmp *%r11		# Jump to function address.
	cfi_endproc
	.size _dl_runtime_resolve, .-_dl_runtime_resolve


/* To preserve %xmm0 - %xmm7 registers, dl-trampoline.h is included
   twice, for _dl_runtime_resolve_sse and _dl_runtime_resolve_sse_vex.
   But we don't need another _dl_runtime_profile for XMM registers.  */
#if !defined PROF && defined _dl_runtime_profile
# if (LR_VECTOR_OFFSET % VEC_SIZE) != 0
#  error LR_VECTOR_OFFSET must be multples of VEC_SIZE
# endif

	.globl _dl_runtime_profile
	.hidden _dl_runtime_profile
	.type _dl_runtime_profile, @function
	.align 16
_dl_runtime_profile:
	cfi_startproc
	cfi_adjust_cfa_offset(16) # Incorporate PLT
	/* The La_x86_64_regs data structure pointed to by the
	   fourth paramater must be VEC_SIZE-byte aligned.  This must
	   be explicitly enforced.  We have the set up a dynamically
	   sized stack frame.  %rbx points to the top half which
	   has a fixed size and preserves the original stack pointer.  */

	sub $32, %RSP_LP	# Allocate the local storage.
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
	mov %RSP_LP, %RBX_LP
	cfi_def_cfa_register(%rbx)

	/* Actively align the La_x86_64_regs structure.  */
	and $-VEC_SIZE, %RSP_LP
	/* sizeof(La_x86_64_regs).  Need extra space for 8 SSE registers
	   to detect if any xmm0-xmm7 registers are changed by audit
	   module.  */
	sub $(LR_SIZE + XMM_SIZE*8), %RSP_LP
	movq %rsp, 24(%rbx)

	/* Fill the La_x86_64_regs structure.  */
	movq %rdx, LR_RDX_OFFSET(%rsp)
	movq %r8,  LR_R8_OFFSET(%rsp)
	movq %r9,  LR_R9_OFFSET(%rsp)
	movq %rcx, LR_RCX_OFFSET(%rsp)
	movq %rsi, LR_RSI_OFFSET(%rsp)
	movq %rdi, LR_RDI_OFFSET(%rsp)
	movq %rbp, LR_RBP_OFFSET(%rsp)

	lea 48(%rbx), %RAX_LP
	movq %rax, LR_RSP_OFFSET(%rsp)

	/* We always store the XMM registers even if AVX is available.
	   This is to provide backward binary compatibility for existing
	   audit modules.  */
	movaps %xmm0,		   (LR_XMM_OFFSET)(%rsp)
	movaps %xmm1, (LR_XMM_OFFSET +   XMM_SIZE)(%rsp)
	movaps %xmm2, (LR_XMM_OFFSET + XMM_SIZE*2)(%rsp)
	movaps %xmm3, (LR_XMM_OFFSET + XMM_SIZE*3)(%rsp)
	movaps %xmm4, (LR_XMM_OFFSET + XMM_SIZE*4)(%rsp)
	movaps %xmm5, (LR_XMM_OFFSET + XMM_SIZE*5)(%rsp)
	movaps %xmm6, (LR_XMM_OFFSET + XMM_SIZE*6)(%rsp)
	movaps %xmm7, (LR_XMM_OFFSET + XMM_SIZE*7)(%rsp)

# ifndef __ILP32__
#  ifdef HAVE_MPX_SUPPORT
	bndmov %bnd0, 		   (LR_BND_OFFSET)(%rsp)  # Preserve bound
	bndmov %bnd1, (LR_BND_OFFSET +   BND_SIZE)(%rsp)  # registers. Nops if
	bndmov %bnd2, (LR_BND_OFFSET + BND_SIZE*2)(%rsp)  # MPX not available
	bndmov %bnd3, (LR_BND_OFFSET + BND_SIZE*3)(%rsp)  # or disabled.
#  else
	.byte 0x66,0x0f,0x1b,0x84,0x24;.long (LR_BND_OFFSET)
	.byte 0x66,0x0f,0x1b,0x8c,0x24;.long (LR_BND_OFFSET + BND_SIZE)
	.byte 0x66,0x0f,0x1b,0x94,0x24;.long (LR_BND_OFFSET + BND_SIZE*2)
	.byte 0x66,0x0f,0x1b,0x9c,0x24;.long (LR_BND_OFFSET + BND_SIZE*3)
#  endif
# endif

# ifdef RESTORE_AVX
	/* This is to support AVX audit modules.  */
	VMOVA %VEC(0),		      (LR_VECTOR_OFFSET)(%rsp)
	VMOVA %VEC(1), (LR_VECTOR_OFFSET +   VECTOR_SIZE)(%rsp)
	VMOVA %VEC(2), (LR_VECTOR_OFFSET + VECTOR_SIZE*2)(%rsp)
	VMOVA %VEC(3), (LR_VECTOR_OFFSET + VECTOR_SIZE*3)(%rsp)
	VMOVA %VEC(4), (LR_VECTOR_OFFSET + VECTOR_SIZE*4)(%rsp)
	VMOVA %VEC(5), (LR_VECTOR_OFFSET + VECTOR_SIZE*5)(%rsp)
	VMOVA %VEC(6), (LR_VECTOR_OFFSET + VECTOR_SIZE*6)(%rsp)
	VMOVA %VEC(7), (LR_VECTOR_OFFSET + VECTOR_SIZE*7)(%rsp)

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
# endif

	mov %RSP_LP, %RCX_LP	# La_x86_64_regs pointer to %rcx.
	mov 48(%rbx), %RDX_LP	# Load return address if needed.
	mov 40(%rbx), %RSI_LP	# Copy args pushed by PLT in register.
	mov 32(%rbx), %RDI_LP	# %rdi: link_map, %rsi: reloc_index
	lea 16(%rbx), %R8_LP	# Address of framesize
	call _dl_profile_fixup	# Call resolver.

	mov %RAX_LP, %R11_LP	# Save return value.

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

# ifdef RESTORE_AVX
	/* Check if any xmm0-xmm7 registers are changed by audit
	   module.  */
	vpcmpeqq (LR_SIZE)(%rsp), %xmm0, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	je 2f
	vmovdqa	%xmm0, (LR_VECTOR_OFFSET)(%rsp)
	jmp 1f
2:	VMOVA (LR_VECTOR_OFFSET)(%rsp), %VEC(0)
	vmovdqa	%xmm0, (LR_XMM_OFFSET)(%rsp)

1:	vpcmpeqq (LR_SIZE + XMM_SIZE)(%rsp), %xmm1, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	je 2f
	vmovdqa	%xmm1, (LR_VECTOR_OFFSET + VECTOR_SIZE)(%rsp)
	jmp 1f
2:	VMOVA (LR_VECTOR_OFFSET + VECTOR_SIZE)(%rsp), %VEC(1)
	vmovdqa	%xmm1, (LR_XMM_OFFSET + XMM_SIZE)(%rsp)

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*2)(%rsp), %xmm2, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	je 2f
	vmovdqa	%xmm2, (LR_VECTOR_OFFSET + VECTOR_SIZE*2)(%rsp)
	jmp 1f
2:	VMOVA (LR_VECTOR_OFFSET + VECTOR_SIZE*2)(%rsp), %VEC(2)
	vmovdqa	%xmm2, (LR_XMM_OFFSET + XMM_SIZE*2)(%rsp)

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*3)(%rsp), %xmm3, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	je 2f
	vmovdqa	%xmm3, (LR_VECTOR_OFFSET + VECTOR_SIZE*3)(%rsp)
	jmp 1f
2:	VMOVA (LR_VECTOR_OFFSET + VECTOR_SIZE*3)(%rsp), %VEC(3)
	vmovdqa	%xmm3, (LR_XMM_OFFSET + XMM_SIZE*3)(%rsp)

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*4)(%rsp), %xmm4, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	je 2f
	vmovdqa	%xmm4, (LR_VECTOR_OFFSET + VECTOR_SIZE*4)(%rsp)
	jmp 1f
2:	VMOVA (LR_VECTOR_OFFSET + VECTOR_SIZE*4)(%rsp), %VEC(4)
	vmovdqa	%xmm4, (LR_XMM_OFFSET + XMM_SIZE*4)(%rsp)

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*5)(%rsp), %xmm5, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	je 2f
	vmovdqa	%xmm5, (LR_VECTOR_OFFSET + VECTOR_SIZE*5)(%rsp)
	jmp 1f
2:	VMOVA (LR_VECTOR_OFFSET + VECTOR_SIZE*5)(%rsp), %VEC(5)
	vmovdqa	%xmm5, (LR_XMM_OFFSET + XMM_SIZE*5)(%rsp)

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*6)(%rsp), %xmm6, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	je 2f
	vmovdqa	%xmm6, (LR_VECTOR_OFFSET + VECTOR_SIZE*6)(%rsp)
	jmp 1f
2:	VMOVA (LR_VECTOR_OFFSET + VECTOR_SIZE*6)(%rsp), %VEC(6)
	vmovdqa	%xmm6, (LR_XMM_OFFSET + XMM_SIZE*6)(%rsp)

1:	vpcmpeqq (LR_SIZE + XMM_SIZE*7)(%rsp), %xmm7, %xmm8
	vpmovmskb %xmm8, %esi
	cmpl $0xffff, %esi
	je 2f
	vmovdqa	%xmm7, (LR_VECTOR_OFFSET + VECTOR_SIZE*7)(%rsp)
	jmp 1f
2:	VMOVA (LR_VECTOR_OFFSET + VECTOR_SIZE*7)(%rsp), %VEC(7)
	vmovdqa	%xmm7, (LR_XMM_OFFSET + XMM_SIZE*7)(%rsp)

1:
# endif

# ifndef __ILP32__
#  ifdef HAVE_MPX_SUPPORT
	bndmov              (LR_BND_OFFSET)(%rsp), %bnd0  # Restore bound
	bndmov (LR_BND_OFFSET +   BND_SIZE)(%rsp), %bnd1  # registers.
	bndmov (LR_BND_OFFSET + BND_SIZE*2)(%rsp), %bnd2
	bndmov (LR_BND_OFFSET + BND_SIZE*3)(%rsp), %bnd3
#  else
	.byte 0x66,0x0f,0x1a,0x84,0x24;.long (LR_BND_OFFSET)
	.byte 0x66,0x0f,0x1a,0x8c,0x24;.long (LR_BND_OFFSET + BND_SIZE)
	.byte 0x66,0x0f,0x1a,0x94,0x24;.long (LR_BND_OFFSET + BND_SIZE*2)
	.byte 0x66,0x0f,0x1a,0x9c,0x24;.long (LR_BND_OFFSET + BND_SIZE*3)
#  endif
# endif

	mov  16(%rbx), %R10_LP	# Anything in framesize?
	test %R10_LP, %R10_LP
	PRESERVE_BND_REGS_PREFIX
	jns 3f

	/* There's nothing in the frame size, so there
	   will be no call to the _dl_call_pltexit. */

	/* Get back registers content.  */
	movq LR_RCX_OFFSET(%rsp), %rcx
	movq LR_RSI_OFFSET(%rsp), %rsi
	movq LR_RDI_OFFSET(%rsp), %rdi

	mov %RBX_LP, %RSP_LP
	movq (%rsp), %rbx
	cfi_restore(%rbx)
	cfi_def_cfa_register(%rsp)

	add $48, %RSP_LP	# Adjust the stack to the return value
				# (eats the reloc index and link_map)
	cfi_adjust_cfa_offset(-48)
	PRESERVE_BND_REGS_PREFIX
	jmp *%r11		# Jump to function address.

3:
	cfi_adjust_cfa_offset(48)
	cfi_rel_offset(%rbx, 0)
	cfi_def_cfa_register(%rbx)

	/* At this point we need to prepare new stack for the function
	   which has to be called.  We copy the original stack to a
	   temporary buffer of the size specified by the 'framesize'
	   returned from _dl_profile_fixup */

	lea LR_RSP_OFFSET(%rbx), %RSI_LP # stack
	add $8, %R10_LP
	and $-16, %R10_LP
	mov %R10_LP, %RCX_LP
	sub %R10_LP, %RSP_LP
	mov %RSP_LP, %RDI_LP
	shr $3, %RCX_LP
	rep
	movsq

	movq 24(%rdi), %rcx	# Get back register content.
	movq 32(%rdi), %rsi
	movq 40(%rdi), %rdi

	PRESERVE_BND_REGS_PREFIX
	call *%r11

	mov 24(%rbx), %RSP_LP	# Drop the copied stack content

	/* Now we have to prepare the La_x86_64_retval structure for the
	   _dl_call_pltexit.  The La_x86_64_regs is being pointed by rsp now,
	   so we just need to allocate the sizeof(La_x86_64_retval) space on
	   the stack, since the alignment has already been taken care of. */
# ifdef RESTORE_AVX
	/* sizeof(La_x86_64_retval).  Need extra space for 2 SSE
	   registers to detect if xmm0/xmm1 registers are changed
	   by audit module.  */
	sub $(LRV_SIZE + XMM_SIZE*2), %RSP_LP
# else
	sub $LRV_SIZE, %RSP_LP	# sizeof(La_x86_64_retval)
# endif
	mov %RSP_LP, %RCX_LP	# La_x86_64_retval argument to %rcx.

	/* Fill in the La_x86_64_retval structure.  */
	movq %rax, LRV_RAX_OFFSET(%rcx)
	movq %rdx, LRV_RDX_OFFSET(%rcx)

	movaps %xmm0, LRV_XMM0_OFFSET(%rcx)
	movaps %xmm1, LRV_XMM1_OFFSET(%rcx)

# ifdef RESTORE_AVX
	/* This is to support AVX audit modules.  */
	VMOVA %VEC(0), LRV_VECTOR0_OFFSET(%rcx)
	VMOVA %VEC(1), LRV_VECTOR1_OFFSET(%rcx)

	/* Save xmm0/xmm1 registers to detect if they are changed
	   by audit module.  */
	vmovdqa %xmm0,		  (LRV_SIZE)(%rcx)
	vmovdqa %xmm1, (LRV_SIZE + XMM_SIZE)(%rcx)
# endif

# ifndef __ILP32__
#  ifdef HAVE_MPX_SUPPORT
	bndmov %bnd0, LRV_BND0_OFFSET(%rcx)  # Preserve returned bounds.
	bndmov %bnd1, LRV_BND1_OFFSET(%rcx)
#  else
	.byte  0x66,0x0f,0x1b,0x81;.long (LRV_BND0_OFFSET)
	.byte  0x66,0x0f,0x1b,0x89;.long (LRV_BND1_OFFSET)
#  endif
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

# ifdef RESTORE_AVX
	/* Check if xmm0/xmm1 registers are changed by audit module.  */
	vpcmpeqq (LRV_SIZE)(%rsp), %xmm0, %xmm2
	vpmovmskb %xmm2, %esi
	cmpl $0xffff, %esi
	jne 1f
	VMOVA LRV_VECTOR0_OFFSET(%rsp), %VEC(0)

1:	vpcmpeqq (LRV_SIZE + XMM_SIZE)(%rsp), %xmm1, %xmm2
	vpmovmskb %xmm2, %esi
	cmpl $0xffff, %esi
	jne 1f
	VMOVA LRV_VECTOR1_OFFSET(%rsp), %VEC(1)

1:
# endif

# ifndef __ILP32__
#  ifdef HAVE_MPX_SUPPORT
	bndmov LRV_BND0_OFFSET(%rsp), %bnd0  # Restore bound registers.
	bndmov LRV_BND1_OFFSET(%rsp), %bnd1
#  else
	.byte  0x66,0x0f,0x1a,0x84,0x24;.long (LRV_BND0_OFFSET)
	.byte  0x66,0x0f,0x1a,0x8c,0x24;.long (LRV_BND1_OFFSET)
#  endif
# endif

	fldt LRV_ST1_OFFSET(%rsp)
	fldt LRV_ST0_OFFSET(%rsp)

	mov %RBX_LP, %RSP_LP
	movq (%rsp), %rbx
	cfi_restore(%rbx)
	cfi_def_cfa_register(%rsp)

	add $48, %RSP_LP	# Adjust the stack to the return value
				# (eats the reloc index and link_map)
	cfi_adjust_cfa_offset(-48)
	PRESERVE_BND_REGS_PREFIX
	retq

	cfi_endproc
	.size _dl_runtime_profile, .-_dl_runtime_profile
#endif
