/* Copyright (C) 1998, 2000, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper, <drepper@cygnus.com>, 1998.

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

#ifndef _LINUX_I386_I686_SYSDEP_H
#define _LINUX_I386_I686_SYSDEP_H 1

/* There is some commonality.  */
#include <sysdeps/unix/sysv/linux/i386/sysdep.h>
#include <bp-sym.h>
#include <bp-asm.h>

/* We define special versions of the error handler code to match the i686's
   deep branch prediction mechanism.  */
#ifdef PIC
# undef SYSCALL_ERROR_HANDLER

# undef SETUP_PIC_REG
# ifndef HAVE_HIDDEN
#  define SETUP_PIC_REG(reg) \
  call 1f;								      \
  .subsection 1;							      \
1:movl (%esp), %e##reg;							      \
  ret;									      \
  .previous
# else
#  define SETUP_PIC_REG(reg) \
  .section .gnu.linkonce.t.__i686.get_pc_thunk.reg,"ax",@progbits;	      \
  .globl __i686.get_pc_thunk.reg;					      \
  .hidden __i686.get_pc_thunk.reg;					      \
  .type __i686.get_pc_thunk.reg,@function;				      \
__i686.get_pc_thunk.reg:						      \
  movl (%esp), %e##reg;							      \
  ret;									      \
  .previous;								      \
  call __i686.get_pc_thunk.reg
# endif

/* Store (- %eax) into errno through the GOT.  */
# ifdef _LIBC_REENTRANT
#  if USE_TLS && HAVE___THREAD
#   define SYSCALL_ERROR_HANDLER					      \
0:SETUP_PIC_REG (cx);							      \
  addl $_GLOBAL_OFFSET_TABLE_, %ecx;					      \
  xorl %edx, %edx;							      \
  subl %eax, %edx;							      \
  movl %gs:0, %eax;							      \
  subl errno@gottpoff(%ecx), %eax;					      \
  movl %edx, (%eax);							      \
  orl $-1, %eax;							      \
  jmp L(pseudo_end);
#  else
#   define SYSCALL_ERROR_HANDLER					      \
0:pushl %ebx;								      \
  SETUP_PIC_REG(bx);							      \
  addl $_GLOBAL_OFFSET_TABLE_, %ebx;					      \
  xorl %edx, %edx;							      \
  subl %eax, %edx;							      \
  pushl %edx;								      \
  PUSH_ERRNO_LOCATION_RETURN;						      \
  call BP_SYM (__errno_location)@PLT;					      \
  POP_ERRNO_LOCATION_RETURN;						      \
  popl %ecx;								      \
  popl %ebx;								      \
  movl %ecx, (%eax);							      \
  orl $-1, %eax;							      \
  jmp L(pseudo_end);
/* A quick note: it is assumed that the call to `__errno_location' does
   not modify the stack!  */
#  endif
# else
#  define SYSCALL_ERROR_HANDLER						      \
0:SETUP_PIC_REG(cx);							      \
  addl $_GLOBAL_OFFSET_TABLE_, %ecx;					      \
  xorl %edx, %edx;							      \
  subl %eax, %edx;							      \
  movl errno@GOT(%ecx), %ecx;						      \
  movl %edx, (%ecx);							      \
  orl $-1, %eax;							      \
  jmp L(pseudo_end);
# endif	/* _LIBC_REENTRANT */
#endif	/* PIC */

#endif /* linux/i386/i686/sysdep.h */
