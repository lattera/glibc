/* Copyright (C) 2001 Free Software Foundation, Inc.
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

#ifndef _LINUX_X86_64_SYSDEP_H
#define _LINUX_X86_64_SYSDEP_H 1

/* There is some commonality.  */
#include <sysdeps/unix/x86_64/sysdep.h>
#include <bp-sym.h>
#include <bp-asm.h>

/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#define SYS_ify(syscall_name)	__NR_##syscall_name

/* ELF-like local names start with `.L'.  */
#undef L
#define L(name)	.L##name

#ifdef __ASSEMBLER__

/* Linux uses a negative return value to indicate syscall errors,
   unlike most Unices, which use the condition codes' carry flag.

   Since version 2.1 the return value of a system call might be
   negative even if the call succeeded.	 E.g., the `lseek' system call
   might return a large offset.	 Therefore we must not anymore test
   for < 0, but test for a real error by making sure the value in %eax
   is a real error number.  Linus said he will make sure the no syscall
   returns a value in -1 .. -4095 as a valid result so we can savely
   test with -4095.  */

/* We don't want the label for the error handle to be global when we define
   it here.  */
#ifdef PIC
# define SYSCALL_ERROR_LABEL 0f
#else
# define SYSCALL_ERROR_LABEL syscall_error
#endif

#undef	PSEUDO
#define	PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name)								      \
    DO_CALL (args, syscall_name);					      \
    cmpq $-4095, %rax;							      \
    jae SYSCALL_ERROR_LABEL;						      \
  L(pseudo_end):

#undef	PSEUDO_END
#define	PSEUDO_END(name)						      \
  SYSCALL_ERROR_HANDLER							      \
  END (name)

#ifndef PIC
#define SYSCALL_ERROR_HANDLER	/* Nothing here; code in sysdep.S is used.  */
#else
/* Store (- %rax) into errno through the GOT.  */
#ifdef _LIBC_REENTRANT
#define SYSCALL_ERROR_HANDLER			\
0:						\
  xorq %rdx, %rdx;				\
  subq %rax, %rdx;				\
  pushq %rdx					\
  PUSH_ERRNO_LOCATION_RETURN;			\
  call BP_SYM (__errno_location)@PLT;		\
  POP_ERRNO_LOCATION_RETURN;			\
  popq %rdx;					\
  movq %rdx, (%rax);				\
  orq $-1, %rax;				\
  jmp L(pseudo_end);

/* A quick note: it is assumed that the call to `__errno_location' does
   not modify the stack!  */
#else
#define SYSCALL_ERROR_HANDLER			\
0:movq errno@GOTPCREL(%RIP), %rcx;		\
  xorq %rdx, %rdx;				\
  subq %rax, %rdx;				\
  movq %rdx, (%rcx);				\
  orq $-1, %rax;				\
  jmp L(pseudo_end);
#endif	/* _LIBC_REENTRANT */
#endif	/* PIC */

/* Linux/x86-64 takes system call arguments in registers:

    Register setup:
    system call number	rax
    arg 1		rdi
    arg 2		rsi
    arg 3		rdx
    arg 4		rcx
    arg 5		r8
    arg 6		r9

    return address from
    syscall		rcx
    additionally clobered: r12-r15,rbx,rbp
    eflags from syscall	r11

    The compiler is going to form a call by coming here, through PSEUDO, with arguments:

	syscall number	in the DO_CALL macro
	arg 1		rdi
	arg 2		rsi
	arg 3		rdx
	arg 4		r10
	arg 5		r8
	arg 6		r9

     We have to take care that the stack is alignedto 16 bytes.	 When
     called the stack is not aligned since the return address has just
     been pushed.

     Syscalls of more than 6 arguments are not supported.  */

#undef	DO_CALL
#define DO_CALL(args, syscall_name)		\
    DOARGS_##args				\
    movq $SYS_ify (syscall_name), %rax;		\
    syscall;

#define DOARGS_0 /* nothing */
#define DOARGS_1 /* nothing */
#define DOARGS_2 /* nothing */
#define DOARGS_3 /* nothing */
#define DOARGS_4 movq %rcx, %r10;
#define DOARGS_5 DOARGS_4
#define DOARGS_6 DOARGS_5

#else	/* !__ASSEMBLER__ */
/* Define a macro which expands inline into the wrapper code for a system
   call.  */
#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...) \
  ({									      \
    unsigned long resultvar;						      \
    LOAD_ARGS_##nr (args)						      \
    asm volatile (							      \
    "movq %1, %%rax\n\t"						      \
    "syscall\n\t"							      \
    : "=a" (resultvar)							      \
    : "i" (__NR_##name) ASM_ARGS_##nr : "memory", "cc", "r11", "cx");	      \
    if (resultvar >= (unsigned long) -4095)				      \
      {									      \
	__set_errno (-resultvar);					      \
	resultvar = (unsigned long) -1;					      \
      }									      \
    (long) resultvar; })

#define LOAD_ARGS_0()
#define ASM_ARGS_0

#define LOAD_ARGS_1(a1)					\
  register long int _a1 asm ("rdi") = (long) (a1);	\
  LOAD_ARGS_0 ()
#define ASM_ARGS_1	ASM_ARGS_0, "r" (_a1)

#define LOAD_ARGS_2(a1, a2)				\
  register long int _a2 asm ("rsi") = (long) (a2);	\
  LOAD_ARGS_1 (a1)
#define ASM_ARGS_2	ASM_ARGS_1, "r" (_a2)

#define LOAD_ARGS_3(a1, a2, a3)				\
  register long int _a3 asm ("rdx") = (long) (a3);	\
  LOAD_ARGS_2 (a1, a2)
#define ASM_ARGS_3	ASM_ARGS_2, "r" (_a3)

#define LOAD_ARGS_4(a1, a2, a3, a4)			\
  register long int _a4 asm ("r10") = (long) (a4);	\
  LOAD_ARGS_3 (a1, a2, a3)
#define ASM_ARGS_4	ASM_ARGS_3, "r" (_a4)

#define LOAD_ARGS_5(a1, a2, a3, a4, a5)			\
  register long int _a5 asm ("r8") = (long) (a5);	\
  LOAD_ARGS_4 (a1, a2, a3, a4)
#define ASM_ARGS_5	ASM_ARGS_4, "r" (_a5)

#define LOAD_ARGS_6(a1, a2, a3, a4, a5, a6)		\
  register long int _a6 asm ("r9") = (long) (a6);	\
  LOAD_ARGS_5 (a1, a2, a3, a4, a5)
#define ASM_ARGS_6	ASM_ARGS_5, "r" (_a6)

#endif	/* __ASSEMBLER__ */

#endif /* linux/x86_64/sysdep.h */
