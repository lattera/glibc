/* POSIX.1 `sigaction' call for Linux/x86-64.
   Copyright (C) 2001, 2002, 2003, 2005, 2006 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <errno.h>
#include <stddef.h>
#include <signal.h>
#include <string.h>

#include <sysdep.h>
#include <sys/syscall.h>

#include <kernel-features.h>

/* The difference here is that the sigaction structure used in the
   kernel is not the same as we use in the libc.  Therefore we must
   translate it here.  */
#include <kernel_sigaction.h>

#include "ucontext_i.h"

/* We do not globally define the SA_RESTORER flag so do it here.  */
#define SA_RESTORER 0x04000000

/* Using the hidden attribute here does not change the code but it
   helps to avoid warnings.  */
extern void restore_rt (void) asm ("__restore_rt") attribute_hidden;


/* If ACT is not NULL, change the action for SIG to *ACT.
   If OACT is not NULL, put the old action for SIG in *OACT.  */
int
__libc_sigaction (int sig, const struct sigaction *act, struct sigaction *oact)
{
  int result;
  struct kernel_sigaction kact, koact;

  if (act)
    {
      kact.k_sa_handler = act->sa_handler;
      memcpy (&kact.sa_mask, &act->sa_mask, sizeof (sigset_t));
      kact.sa_flags = act->sa_flags | SA_RESTORER;

      kact.sa_restorer = &restore_rt;
    }

  /* XXX The size argument hopefully will have to be changed to the
     real size of the user-level sigset_t.  */
  result = INLINE_SYSCALL (rt_sigaction, 4,
			   sig, act ? __ptrvalue (&kact) : NULL,
			   oact ? __ptrvalue (&koact) : NULL, _NSIG / 8);
  if (oact && result >= 0)
    {
      oact->sa_handler = koact.k_sa_handler;
      memcpy (&oact->sa_mask, &koact.sa_mask, sizeof (sigset_t));
      oact->sa_flags = koact.sa_flags;
      oact->sa_restorer = koact.sa_restorer;
    }
  return result;
}
libc_hidden_def (__libc_sigaction)

#ifdef WRAPPER_INCLUDE
# include WRAPPER_INCLUDE
#endif

#ifndef LIBC_SIGACTION
weak_alias (__libc_sigaction, __sigaction)
libc_hidden_weak (__sigaction)
weak_alias (__libc_sigaction, sigaction)
#endif

/* NOTE: Please think twice before making any changes to the bits of
   code below.  GDB needs some intimate knowledge about it to
   recognize them as signal trampolines, and make backtraces through
   signal handlers work right.  Important are both the names
   (__restore_rt) and the exact instruction sequence.
   If you ever feel the need to make any changes, please notify the
   appropriate GDB maintainer.

   The unwind information starts a byte before __restore_rt, so that
   it is found when unwinding, to get an address the unwinder assumes
   will be in the middle of a call instruction.  See the Linux kernel
   (the i386 vsyscall, in particular) for an explanation of the complex
   unwind information used here in order to get the traditional CFA.
   We do not restore cs - it's only stored as two bytes here so that's
   a bit tricky.  We don't use the gas cfi directives, so that we can
   reliably add .cfi_signal_frame.  */

#define do_cfa_expr						\
  "	.byte 0x0f\n"		/* DW_CFA_def_cfa_expression */	\
  "	.uleb128 2f-1f\n"	/* length */			\
  "1:	.byte 0x77\n"		/* DW_OP_breg7 */		\
  "	.sleb128 " CFI_STRINGIFY (oRSP) "\n"			\
  "	.byte 0x06\n"		/* DW_OP_deref */		\
  "2:"

#define do_expr(regno, offset)					\
  "	.byte 0x10\n"		/* DW_CFA_expression */		\
  "	.uleb128 " CFI_STRINGIFY (regno) "\n"			\
  "	.uleb128 2f-1f\n"	/* length */			\
  "1:	.byte 0x77\n"		/* DW_OP_breg7 */		\
  "	.sleb128 " CFI_STRINGIFY (offset) "\n"			\
  "2:"

#define RESTORE(name, syscall) RESTORE2 (name, syscall)
# define RESTORE2(name, syscall) \
asm									\
  (									\
   /* `nop' for debuggers assuming `call' should not disalign the code.  */ \
   "	nop\n"								\
   ".align 16\n"							\
   ".LSTART_" #name ":\n"						\
   "	.type __" #name ",@function\n"					\
   "__" #name ":\n"							\
   "	movq $" #syscall ", %rax\n"					\
   "	syscall\n"							\
   ".LEND_" #name ":\n"							\
   ".section .eh_frame,\"a\",@progbits\n"				\
   ".LSTARTFRAME_" #name ":\n"						\
   "	.long .LENDCIE_" #name "-.LSTARTCIE_" #name "\n"		\
   ".LSTARTCIE_" #name ":\n"						\
   "	.long 0\n"	/* CIE ID */					\
   "	.byte 1\n"	/* Version number */				\
   "	.string \"zRS\"\n" /* NUL-terminated augmentation string */	\
   "	.uleb128 1\n"	/* Code alignment factor */			\
   "	.sleb128 -8\n"	/* Data alignment factor */			\
   "	.uleb128 16\n"	/* Return address register column (rip) */	\
   /* Augmentation value length */					\
   "	.uleb128 .LENDAUGMNT_" #name "-.LSTARTAUGMNT_" #name "\n"	\
   ".LSTARTAUGMNT_" #name ":\n"						\
   "	.byte 0x1b\n"	/* DW_EH_PE_pcrel|DW_EH_PE_sdata4. */		\
   ".LENDAUGMNT_" #name ":\n"						\
   "	.align " LP_SIZE "\n"						\
   ".LENDCIE_" #name ":\n"						\
   "	.long .LENDFDE_" #name "-.LSTARTFDE_" #name "\n" /* FDE len */	\
   ".LSTARTFDE_" #name ":\n"						\
   "	.long .LSTARTFDE_" #name "-.LSTARTFRAME_" #name "\n" /* CIE */	\
   /* `LSTART_' is subtracted 1 as debuggers assume a `call' here.  */	\
   "	.long (.LSTART_" #name "-1)-.\n" /* PC-relative start addr.  */	\
   "	.long .LEND_" #name "-(.LSTART_" #name "-1)\n"			\
   "	.uleb128 0\n"			/* FDE augmentation length */	\
   do_cfa_expr								\
   do_expr (8 /* r8 */, oR8)						\
   do_expr (9 /* r9 */, oR9)						\
   do_expr (10 /* r10 */, oR10)						\
   do_expr (11 /* r11 */, oR11)						\
   do_expr (12 /* r12 */, oR12)						\
   do_expr (13 /* r13 */, oR13)						\
   do_expr (14 /* r14 */, oR14)						\
   do_expr (15 /* r15 */, oR15)						\
   do_expr (5 /* rdi */, oRDI)						\
   do_expr (4 /* rsi */, oRSI)						\
   do_expr (6 /* rbp */, oRBP)						\
   do_expr (3 /* rbx */, oRBX)						\
   do_expr (1 /* rdx */, oRDX)						\
   do_expr (0 /* rax */, oRAX)						\
   do_expr (2 /* rcx */, oRCX)						\
   do_expr (7 /* rsp */, oRSP)						\
   do_expr (16 /* rip */, oRIP)						\
   /* libgcc-4.1.1 has only `DWARF_FRAME_REGISTERS == 17'.  */		\
   /* do_expr (49 |* rflags *|, oEFL) */				\
   /* `cs'/`ds'/`fs' are unaligned and a different size.  */		\
   /* gas: Error: register save offset not a multiple of 8  */		\
   "	.align " LP_SIZE "\n"						\
   ".LENDFDE_" #name ":\n"						\
   "	.previous\n"							\
   );
/* The return code for realtime-signals.  */
RESTORE (restore_rt, __NR_rt_sigreturn)
