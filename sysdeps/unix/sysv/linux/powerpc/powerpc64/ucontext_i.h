/* Offsets and other constants needed in the *context() function
   implementation.
   Copyright (C) 2002 Free Software Foundation, Inc.
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

#define SIG_BLOCK	0
#define SIG_SETMASK	2

/* Offsets of the fields in the powerpc64 ABI stack frame.  */

#define FRAME_BACKCHAIN 0
#define FRAME_CR_SAVE 8
#define FRAME_LR_SAVE 16
#define FRAME_COMPILER_DW 24
#define FRAME_LINKER_DW 32
#define FRAME_TOC_SAVE 40
#define FRAME_PARM_SAVE 48
#define FRAME_PARM1_SAVE 48
#define FRAME_PARM2_SAVE 56
#define FRAME_PARM3_SAVE 64
#define FRAME_PARM4_SAVE 72
#define FRAME_PARM5_SAVE 80
#define FRAME_PARM6_SAVE 88
#define FRAME_PARM7_SAVE 96
#define FRAME_PARM8_SAVE 104
#define FRAME_PARM9_SAVE 112


/* Offsets of the fields in the ucontext_t structure.  */

#define UCONTEXT_LINK 8
#define UCONTEXT_STACK 16
#define UCONTEXT_STACK_SP 16
#define UCONTEXT_STACK_FLAGS 24
#define UCONTEXT_STACK_SIZE 32
#define UCONTEXT_SIGMASK 40
#define UCONTEXT_MCONTEXT 168
#define SIGCONTEXT_SIGNAL 200
#define SIGCONTEXT_HANDLER 208
#define SIGCONTEXT_OLDMASK 216
#define SIGCONTEXT_PT_REGS 224
#define SIGCONTEXT_GP_REGS 232
#define SIGCONTEXT_FP_REGS 616
#define SIGCONTEXT_V_REGS_PTR 880
#define SIGCONTEXT_V_RESERVE 888
