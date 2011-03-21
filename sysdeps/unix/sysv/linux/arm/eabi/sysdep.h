/* Copyright (C) 2005, 2006, 2007, 2009
   Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   Contributed by Daniel Jacobowitz <dan@codesourcery.com>, Oct 2005.

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

#ifndef _LINUX_ARM_EABI_SYSDEP_H
#define _LINUX_ARM_EABI_SYSDEP_H 1

#include <arm/sysdep.h>

#include <tls.h>

#if __NR_SYSCALL_BASE != 0
# error Kernel headers are too old
#endif

/* Don't use stime, even if the kernel headers define it.  We have
   settimeofday, and some EABI kernels have removed stime.  Similarly
   use setitimer to implement alarm.  */
#undef __NR_stime
#undef __NR_alarm

/* The ARM EABI user interface passes the syscall number in r7, instead
   of in the swi.  This is more efficient, because the kernel does not need
   to fetch the swi from memory to find out the number; which can be painful
   with separate I-cache and D-cache.  Make sure to use 0 for the SWI
   argument; otherwise the (optional) compatibility code for APCS binaries
   may be invoked.  */

#if defined(__thumb__)
/* We can not expose the use of r7 to the compiler.  GCC (as
   of 4.5) uses r7 as the hard frame pointer for Thumb - although
   for Thumb-2 it isn't obviously a better choice than r11.
   And GCC does not support asms that conflict with the frame
   pointer.

   This would be easier if syscall numbers never exceeded 255,
   but they do.  For the moment the LOAD_ARGS_7 is sacrificed.
   We can't use push/pop inside the asm because that breaks
   unwinding (i.e. thread cancellation) for this frame.  We can't
   locally save and restore r7, because we do not know if this
   function uses r7 or if it is our caller's r7; if it is our caller's,
   then unwinding will fail higher up the stack.  So we move the
   syscall out of line and provide its own unwind information.  */
#undef LOAD_ARGS_7
#undef INTERNAL_SYSCALL_RAW
#define INTERNAL_SYSCALL_RAW(name, err, nr, args...)		\
  ({								\
      register int _a1 asm ("a1");				\
      int _nametmp = name;					\
      LOAD_ARGS_##nr (args)					\
      register int _name asm ("ip") = _nametmp;			\
      asm volatile ("bl      __libc_do_syscall"			\
                    : "=r" (_a1)				\
                    : "r" (_name) ASM_ARGS_##nr			\
                    : "memory", "lr");				\
      _a1; })
#else /* ARM */
#undef INTERNAL_SYSCALL_RAW
#define INTERNAL_SYSCALL_RAW(name, err, nr, args...)		\
  ({								\
       register int _a1 asm ("r0"), _nr asm ("r7");		\
       LOAD_ARGS_##nr (args)					\
       _nr = name;						\
       asm volatile ("swi	0x0	@ syscall " #name	\
		     : "=r" (_a1)				\
		     : "r" (_nr) ASM_ARGS_##nr			\
		     : "memory");				\
       _a1; })
#endif

/* For EABI, non-constant syscalls are actually pretty easy...  */
#undef INTERNAL_SYSCALL_NCS
#define INTERNAL_SYSCALL_NCS(number, err, nr, args...)          \
  INTERNAL_SYSCALL_RAW (number, err, nr, args)

/* We must save and restore r7 (call-saved) for the syscall number.
   We never make function calls from inside here (only potentially
   signal handlers), so we do not bother with doubleword alignment.

   Just like the APCS syscall convention, the EABI syscall convention uses
   r0 through r6 for up to seven syscall arguments.  None are ever passed to
   the kernel on the stack, although incoming arguments are on the stack for
   syscalls with five or more arguments.

   The assembler will convert the literal pool load to a move for most
   syscalls.  */

#undef	DO_CALL
#define DO_CALL(syscall_name, args)		\
    DOARGS_##args;				\
    ldr r7, =SYS_ify (syscall_name);		\
    swi 0x0;					\
    UNDOARGS_##args

#undef  DOARGS_0
#define DOARGS_0 \
  .fnstart; \
  str r7, [sp, #-4]!; \
  cfi_adjust_cfa_offset (4); \
  cfi_rel_offset (r7, 0); \
  .save { r7 }
#undef  DOARGS_1
#define DOARGS_1 DOARGS_0
#undef  DOARGS_2
#define DOARGS_2 DOARGS_0
#undef  DOARGS_3
#define DOARGS_3 DOARGS_0
#undef  DOARGS_4
#define DOARGS_4 DOARGS_0
#undef  DOARGS_5
#define DOARGS_5 \
  .fnstart; \
  stmfd sp!, {r4, r7}; \
  cfi_adjust_cfa_offset (8); \
  cfi_rel_offset (r4, 0); \
  cfi_rel_offset (r7, 4); \
  .save { r4, r7 }; \
  ldr r4, [sp, #8]
#undef  DOARGS_6
#define DOARGS_6 \
  .fnstart; \
  mov ip, sp; \
  stmfd sp!, {r4, r5, r7}; \
  cfi_adjust_cfa_offset (12); \
  cfi_rel_offset (r4, 0); \
  cfi_rel_offset (r5, 4); \
  cfi_rel_offset (r7, 8); \
  .save { r4, r5, r7 }; \
  ldmia ip, {r4, r5}
#undef  DOARGS_7
#define DOARGS_7 \
  .fnstart; \
  mov ip, sp; \
  stmfd sp!, {r4, r5, r6, r7}; \
  cfi_adjust_cfa_offset (16); \
  cfi_rel_offset (r4, 0); \
  cfi_rel_offset (r5, 4); \
  cfi_rel_offset (r6, 8); \
  cfi_rel_offset (r7, 12); \
  .save { r4, r5, r6, r7 }; \
  ldmia ip, {r4, r5, r6}

#undef  UNDOARGS_0
#define UNDOARGS_0 \
  ldr r7, [sp], #4; \
  cfi_adjust_cfa_offset (-4); \
  cfi_restore (r7); \
  .fnend
#undef  UNDOARGS_1
#define UNDOARGS_1 UNDOARGS_0
#undef  UNDOARGS_2
#define UNDOARGS_2 UNDOARGS_0
#undef  UNDOARGS_3
#define UNDOARGS_3 UNDOARGS_0
#undef  UNDOARGS_4
#define UNDOARGS_4 UNDOARGS_0
#undef  UNDOARGS_5
#define UNDOARGS_5 \
  ldmfd sp!, {r4, r7}; \
  cfi_adjust_cfa_offset (-8); \
  cfi_restore (r4); \
  cfi_restore (r7); \
  .fnend
#undef  UNDOARGS_6
#define UNDOARGS_6 \
  ldmfd sp!, {r4, r5, r7}; \
  cfi_adjust_cfa_offset (-12); \
  cfi_restore (r4); \
  cfi_restore (r5); \
  cfi_restore (r7); \
  .fnend
#undef  UNDOARGS_7
#define UNDOARGS_7 \
  ldmfd sp!, {r4, r5, r6, r7}; \
  cfi_adjust_cfa_offset (-16); \
  cfi_restore (r4); \
  cfi_restore (r5); \
  cfi_restore (r6); \
  cfi_restore (r7); \
  .fnend

#endif /* _LINUX_ARM_EABI_SYSDEP_H */
