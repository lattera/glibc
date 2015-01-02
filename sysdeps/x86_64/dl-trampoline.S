/* PLT trampolines.  x86-64 version.
   Copyright (C) 2004-2015 Free Software Foundation, Inc.
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

#include <config.h>
#include <sysdep.h>
#include <link-defines.h>

#if (RTLD_SAVESPACE_SSE % 32) != 0
# error RTLD_SAVESPACE_SSE must be aligned to 32 bytes
#endif

/* Area on stack to save and restore registers used for parameter
   passing when calling _dl_fixup.  */
#ifdef __ILP32__
/* X32 saves RCX, RDX, RSI, RDI, R8 and R9 plus RAX.  */
# define REGISTER_SAVE_AREA	(8 * 7)
# define REGISTER_SAVE_RAX	0
#else
/* X86-64 saves RCX, RDX, RSI, RDI, R8 and R9 plus RAX as well as BND0,
   BND1, BND2, BND3.  */
# define REGISTER_SAVE_AREA	(8 * 7 + 16 * 4)
/* Align bound register save area to 16 bytes.  */
# define REGISTER_SAVE_BND0	0
# define REGISTER_SAVE_BND1	(REGISTER_SAVE_BND0 + 16)
# define REGISTER_SAVE_BND2	(REGISTER_SAVE_BND1 + 16)
# define REGISTER_SAVE_BND3	(REGISTER_SAVE_BND2 + 16)
# define REGISTER_SAVE_RAX	(REGISTER_SAVE_BND3 + 16)
#endif
#define REGISTER_SAVE_RCX	(REGISTER_SAVE_RAX + 8)
#define REGISTER_SAVE_RDX	(REGISTER_SAVE_RCX + 8)
#define REGISTER_SAVE_RSI	(REGISTER_SAVE_RDX + 8)
#define REGISTER_SAVE_RDI	(REGISTER_SAVE_RSI + 8)
#define REGISTER_SAVE_R8	(REGISTER_SAVE_RDI + 8)
#define REGISTER_SAVE_R9	(REGISTER_SAVE_R8 + 8)

	.text
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve, @function
	.align 16
	cfi_startproc
_dl_runtime_resolve:
	cfi_adjust_cfa_offset(16) # Incorporate PLT
	subq $REGISTER_SAVE_AREA,%rsp
	cfi_adjust_cfa_offset(REGISTER_SAVE_AREA)
	# Preserve registers otherwise clobbered.
	movq %rax, REGISTER_SAVE_RAX(%rsp)
	movq %rcx, REGISTER_SAVE_RCX(%rsp)
	movq %rdx, REGISTER_SAVE_RDX(%rsp)
	movq %rsi, REGISTER_SAVE_RSI(%rsp)
	movq %rdi, REGISTER_SAVE_RDI(%rsp)
	movq %r8, REGISTER_SAVE_R8(%rsp)
	movq %r9, REGISTER_SAVE_R9(%rsp)
#ifndef __ILP32__
	# We also have to preserve bound registers.  These are nops if
	# Intel MPX isn't available or disabled.
# ifdef HAVE_MPX_SUPPORT
	bndmov %bnd0, REGISTER_SAVE_BND0(%rsp)
	bndmov %bnd1, REGISTER_SAVE_BND1(%rsp)
	bndmov %bnd2, REGISTER_SAVE_BND2(%rsp)
	bndmov %bnd3, REGISTER_SAVE_BND3(%rsp)
# else
	.byte 0x66,0x0f,0x1b,0x44,0x24,REGISTER_SAVE_BND0
	.byte 0x66,0x0f,0x1b,0x4c,0x24,REGISTER_SAVE_BND1
	.byte 0x66,0x0f,0x1b,0x54,0x24,REGISTER_SAVE_BND2
	.byte 0x66,0x0f,0x1b,0x5c,0x24,REGISTER_SAVE_BND3
# endif
#endif
	# Copy args pushed by PLT in register.
	# %rdi: link_map, %rsi: reloc_index
	movq (REGISTER_SAVE_AREA + 8)(%rsp), %rsi
	movq REGISTER_SAVE_AREA(%rsp), %rdi
	call _dl_fixup		# Call resolver.
	movq %rax, %r11		# Save return value
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
	.byte 0x66,0x0f,0x1a,0x44,0x24,REGISTER_SAVE_BND0
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
	# Adjust stack(PLT did 2 pushes)
	addq $(REGISTER_SAVE_AREA + 16), %rsp
	cfi_adjust_cfa_offset(-(REGISTER_SAVE_AREA + 16))
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
# if defined HAVE_AVX_SUPPORT || defined HAVE_AVX512_ASM_SUPPORT
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
	.byte 0x66,0x0f,0x1b,0x84,0x24;.long (LR_BND_OFFSET + BND_SIZE*2)
	.byte 0x66,0x0f,0x1b,0x8c,0x24;.long (LR_BND_OFFSET + BND_SIZE*3)
#  endif
# endif

# if defined HAVE_AVX_SUPPORT || defined HAVE_AVX512_ASM_SUPPORT
	.data
L(have_avx):
	.zero 4
	.size L(have_avx), 4
	.previous

	cmpl	$0, L(have_avx)(%rip)
	jne	L(defined)
	movq	%rbx, %r11		# Save rbx
	movl	$1, %eax
	cpuid
	movq	%r11,%rbx		# Restore rbx
	xorl	%eax, %eax
	// AVX and XSAVE supported?
	andl	$((1 << 28) | (1 << 27)), %ecx
	cmpl	$((1 << 28) | (1 << 27)), %ecx
	jne	10f
#  ifdef HAVE_AVX512_ASM_SUPPORT
	// AVX512 supported in processor?
	movq	%rbx, %r11		# Save rbx
	xorl	%ecx, %ecx
	mov	$0x7, %eax
	cpuid
	andl	$(1 << 16), %ebx
#  endif
	xorl	%ecx, %ecx
	// Get XFEATURE_ENABLED_MASK
	xgetbv
#  ifdef HAVE_AVX512_ASM_SUPPORT
	test	%ebx, %ebx
	movq	%r11, %rbx		# Restore rbx
	je	20f
	// Verify that XCR0[7:5] = '111b' and
	// XCR0[2:1] = '11b' which means
	// that zmm state is enabled
	andl	$0xe6, %eax
	cmpl	$0xe6, %eax
	jne	20f
	movl	%eax, L(have_avx)(%rip)
L(avx512):
#   define RESTORE_AVX
#   define VMOV    vmovdqu64
#   define VEC(i)  zmm##i
#   define MORE_CODE
#   include "dl-trampoline.h"
#   undef VMOV
#   undef VEC
#   undef RESTORE_AVX
#  endif
20:	andl	$0x6, %eax
10:	subl	$0x5, %eax
	movl	%eax, L(have_avx)(%rip)
	cmpl	$0, %eax

L(defined):
	js	L(no_avx)
#  ifdef HAVE_AVX512_ASM_SUPPORT
	cmpl	$0xe6, L(have_avx)(%rip)
	je	L(avx512)
#  endif

#  define RESTORE_AVX
#  define VMOV    vmovdqu
#  define VEC(i)  ymm##i
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
# if defined HAVE_AVX_SUPPORT || defined HAVE_AVX512_ASM_SUPPORT
	cmpl	$0, L(have_avx)(%rip)
	jne	L(defined_5)
	movq	%rbx, %r11		# Save rbx
	movl	$1, %eax
	cpuid
	movq	%r11,%rbx		# Restore rbx
	xorl	%eax, %eax
	// AVX and XSAVE supported?
	andl	$((1 << 28) | (1 << 27)), %ecx
	cmpl	$((1 << 28) | (1 << 27)), %ecx
	jne	1f
#  ifdef HAVE_AVX512_ASM_SUPPORT
	// AVX512 supported in a processor?
	movq	%rbx, %r11              # Save rbx
	xorl	%ecx,%ecx
	mov	$0x7,%eax
	cpuid
	andl	$(1 << 16), %ebx
#  endif
	xorl	%ecx, %ecx
	// Get XFEATURE_ENABLED_MASK
	xgetbv
#  ifdef HAVE_AVX512_ASM_SUPPORT
	test	%ebx, %ebx
	movq	%r11, %rbx		# Restore rbx
	je	2f
	// Verify that XCR0[7:5] = '111b' and
	// XCR0[2:1] = '11b' which means
	// that zmm state is enabled
	andl	$0xe6, %eax
	movl	%eax, L(have_avx)(%rip)
	cmpl	$0xe6, %eax
	je	L(avx512_5)
#  endif

2:	andl	$0x6, %eax
1:	subl	$0x5, %eax
	movl	%eax, L(have_avx)(%rip)
	cmpl	$0, %eax

L(defined_5):
	js	L(no_avx5)
#  ifdef HAVE_AVX512_ASM_SUPPORT
	cmpl	$0xe6, L(have_avx)(%rip)
	je	L(avx512_5)
#  endif

	vmovdqa %ymm0, %fs:RTLD_SAVESPACE_SSE+0*YMM_SIZE
	vmovdqa %ymm1, %fs:RTLD_SAVESPACE_SSE+1*YMM_SIZE
	vmovdqa %ymm2, %fs:RTLD_SAVESPACE_SSE+2*YMM_SIZE
	vmovdqa %ymm3, %fs:RTLD_SAVESPACE_SSE+3*YMM_SIZE
	vmovdqa %ymm4, %fs:RTLD_SAVESPACE_SSE+4*YMM_SIZE
	vmovdqa %ymm5, %fs:RTLD_SAVESPACE_SSE+5*YMM_SIZE
	vmovdqa %ymm6, %fs:RTLD_SAVESPACE_SSE+6*YMM_SIZE
	vmovdqa %ymm7, %fs:RTLD_SAVESPACE_SSE+7*YMM_SIZE
	ret
#  ifdef HAVE_AVX512_ASM_SUPPORT
L(avx512_5):
	vmovdqu64 %zmm0, %fs:RTLD_SAVESPACE_SSE+0*ZMM_SIZE
	vmovdqu64 %zmm1, %fs:RTLD_SAVESPACE_SSE+1*ZMM_SIZE
	vmovdqu64 %zmm2, %fs:RTLD_SAVESPACE_SSE+2*ZMM_SIZE
	vmovdqu64 %zmm3, %fs:RTLD_SAVESPACE_SSE+3*ZMM_SIZE
	vmovdqu64 %zmm4, %fs:RTLD_SAVESPACE_SSE+4*ZMM_SIZE
	vmovdqu64 %zmm5, %fs:RTLD_SAVESPACE_SSE+5*ZMM_SIZE
	vmovdqu64 %zmm6, %fs:RTLD_SAVESPACE_SSE+6*ZMM_SIZE
	vmovdqu64 %zmm7, %fs:RTLD_SAVESPACE_SSE+7*ZMM_SIZE
	ret
#  endif
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
# if defined HAVE_AVX_SUPPORT || defined HAVE_AVX512_ASM_SUPPORT
	cmpl	$0, L(have_avx)(%rip)
	js	L(no_avx6)
#  ifdef HAVE_AVX512_ASM_SUPPORT
	cmpl	$0xe6, L(have_avx)(%rip)
	je	L(avx512_6)
#  endif

	vmovdqa %fs:RTLD_SAVESPACE_SSE+0*YMM_SIZE, %ymm0
	vmovdqa %fs:RTLD_SAVESPACE_SSE+1*YMM_SIZE, %ymm1
	vmovdqa %fs:RTLD_SAVESPACE_SSE+2*YMM_SIZE, %ymm2
	vmovdqa %fs:RTLD_SAVESPACE_SSE+3*YMM_SIZE, %ymm3
	vmovdqa %fs:RTLD_SAVESPACE_SSE+4*YMM_SIZE, %ymm4
	vmovdqa %fs:RTLD_SAVESPACE_SSE+5*YMM_SIZE, %ymm5
	vmovdqa %fs:RTLD_SAVESPACE_SSE+6*YMM_SIZE, %ymm6
	vmovdqa %fs:RTLD_SAVESPACE_SSE+7*YMM_SIZE, %ymm7
	ret
#  ifdef HAVE_AVX512_ASM_SUPPORT
L(avx512_6):
	vmovdqu64 %fs:RTLD_SAVESPACE_SSE+0*ZMM_SIZE, %zmm0
	vmovdqu64 %fs:RTLD_SAVESPACE_SSE+1*ZMM_SIZE, %zmm1
	vmovdqu64 %fs:RTLD_SAVESPACE_SSE+2*ZMM_SIZE, %zmm2
	vmovdqu64 %fs:RTLD_SAVESPACE_SSE+3*ZMM_SIZE, %zmm3
	vmovdqu64 %fs:RTLD_SAVESPACE_SSE+4*ZMM_SIZE, %zmm4
	vmovdqu64 %fs:RTLD_SAVESPACE_SSE+5*ZMM_SIZE, %zmm5
	vmovdqu64 %fs:RTLD_SAVESPACE_SSE+6*ZMM_SIZE, %zmm6
	vmovdqu64 %fs:RTLD_SAVESPACE_SSE+7*ZMM_SIZE, %zmm7
	ret
#  endif
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
