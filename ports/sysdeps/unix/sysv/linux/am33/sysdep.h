/* Copyright 2001, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>.
   Based on ../i386/sysdep.h.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _LINUX_AM33_SYSDEP_H
#define _LINUX_AM33_SYSDEP_H 1

/* There is some commonality.  */
#include "../../../am33/sysdep.h"

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
   negative even if the call succeeded.  E.g., the `lseek' system call
   might return a large offset.  Therefore we must not anymore test
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
    DO_CALL (syscall_name, args);					      \
    cmp -126,d0;							      \
    bls L(pseudo_end);							      \
    jmp SYSCALL_ERROR_LABEL;						      \
  L(pseudo_end):							      \
    mov d0,a0;

#undef	PSEUDO_END
#define	PSEUDO_END(name)						      \
  SYSCALL_ERROR_HANDLER							      \
  END (name)

#undef  PSEUDO_NOERROR
#define	PSEUDO_NOERRNO(name, syscall_name, args)			      \
  .text;								      \
  ENTRY (name)								      \
    DO_CALL (syscall_name, args)

#undef	PSEUDO_END_NOERRNO
#define	PSEUDO_END_NOERRNO(name)					      \
  END (name)

#define ret_NOERRNO ret

/* The function has to return the error code.  */
#undef	PSEUDO_ERRVAL
#define	PSEUDO_ERRVAL(name, syscall_name, args) \
  .text;								      \
  ENTRY (name)								      \
    DO_CALL (syscall_name, args);					      \
    clr d1;								      \
    sub d0,d1,d0

#undef	PSEUDO_END_ERRVAL
#define	PSEUDO_END_ERRVAL(name) \
  END (name)

#define ret_ERRVAL ret

#ifndef PIC
#define SYSCALL_ERROR_HANDLER	/* Nothing here; code in sysdep.S is used.  */
#else
/* Store (- d0) into errno through the GOT.  */
#ifdef _LIBC_REENTRANT
#define SYSCALL_ERROR_HANDLER						      \
0:movm [d2,a2],(sp);							      \
  add -12,sp;								      \
1:mov pc,a2;								      \
  add _GLOBAL_OFFSET_TABLE_-(1b-.),a2;					      \
  clr d2;								      \
  sub d0,d2;								      \
  call __errno_location@PLT,[],0;					      \
  mov d2,(a0);								      \
  add 12,sp;								      \
  movm (sp),[d2,a2];							      \
  mov -1,d0;								      \
  mov d0,a0;								      \
  jmp L(pseudo_end);
/* A quick note: it is assumed that the call to `__errno_location' does
   not modify the stack!  */
#else
#define SYSCALL_ERROR_HANDLER						      \
0:mov pc,a0;								      \
  add _GLOBAL_OFFSET_TABLE_-(0b-.),a0;					      \
  clr d1;								      \
  sub d0,d1;								      \
  mov (errno@GOT,a0),a1;						      \
  mov d1,(a0);								      \
  mov -1,d0;								      \
  mov d0,a0;								      \
  jmp L(pseudo_end);
#endif	/* _LIBC_REENTRANT */
#endif	/* PIC */

/* Linux takes system call arguments in registers:

	syscall number	d0	     call-clobbered
	arg 1		a0	     call-clobbered
	arg 2		d1	     call-clobbered
	arg 3		a3	     call-saved
	arg 4		a2	     call-saved
	arg 5		d3	     call-saved
	arg 6		d2	     call-saved

   The stack layout upon entering the function is:

	 (24,sp)	Arg# 6
	 (20,sp)	Arg# 5
	 (16,sp)	Arg# 4
	 (12,sp)	Arg# 3
	  d1		Arg# 2
	  d0		Arg# 1
	  (sp)		Return address

   (Of course a function with say 3 arguments does not have entries for
   arguments 4, 5 and 6.)  */

#undef	DO_CALL
#define DO_CALL(syscall_name, args)			      		      \
    PUSHARGS_##args							      \
    DOARGS_##args							      \
    mov SYS_ify (syscall_name),d0;					      \
    syscall 0								      \
    POPARGS_##args

#define PUSHARGS_0	/* No arguments to push.  */
#define	_DOARGS_0(N)	/* No arguments to frob.  */
#define	DOARGS_0	/* No arguments to frob.  */
#define	POPARGS_0	/* No arguments to pop.  */

#define PUSHARGS_1	/* No arguments to push.  */
#define	_DOARGS_1(N)	_DOARGS_0 (N-4) mov d0,a0;
#define	DOARGS_1	_DOARGS_1 (4)
#define	POPARGS_1	/* No arguments to pop.  */

#define PUSHARGS_2	/* No arguments to push.  */
#define	_DOARGS_2(N)	_DOARGS_1 (N-4) /* Argument already in d1.  */
#define	DOARGS_2	_DOARGS_2 (8)
#define	POPARGS_2	/* No arguments to pop.  */

#define PUSHARGS_3	movm [a3],(sp);
#define	_DOARGS_3(N)	_DOARGS_2 (N-4) mov (N,sp),a3;
#define DOARGS_3	_DOARGS_3 (16)
#define POPARGS_3	; movm (sp),[a3]

#define PUSHARGS_4	movm [a2,a3],(sp);
#define	_DOARGS_4(N)	_DOARGS_3 (N-4) mov (N,sp),a2;
#define DOARGS_4	_DOARGS_4 (24)
#define POPARGS_4	; movm (sp),[a2,a3]

#define PUSHARGS_5	movm [d3,a2,a3],(sp);
#define	_DOARGS_5(N)	_DOARGS_4 (N-4) mov (N,sp),d3;
#define DOARGS_5	_DOARGS_5 (32)
#define POPARGS_5	; movm (sp),[d3,a2,a3]

#define PUSHARGS_6	movm [d2,d3,a2,a3],(sp);
#define	_DOARGS_6(N)	_DOARGS_5 (N-4) mov (N,sp),d2;
#define DOARGS_6	_DOARGS_6 (40)
#define POPARGS_6	; movm (sp),[d2,d3,a2,a3]

#else	/* !__ASSEMBLER__ */

/* Define a macro which expands inline into the wrapper code for a system
   call.  */
#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...) \
  ({									\
    unsigned int resultvar = INTERNAL_SYSCALL (name, , nr, args);	\
    if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (resultvar, ), 0))	\
      {									\
	__set_errno (INTERNAL_SYSCALL_ERRNO (resultvar, ));		\
	resultvar = 0xffffffff;						\
      }									\
    (int) resultvar; })

#define INTERNAL_SYSCALL(name, err, nr, args...)			\
({									\
	register long __sc0 asm ("d0") = __NR_##name; 			\
	inline_syscall##nr(name, ## args);				\
	__sc0;								\
})

#undef INTERNAL_SYSCALL_DECL
#define INTERNAL_SYSCALL_DECL(err) do { } while (0)

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((unsigned int) (val) >= (unsigned long)-125)

#undef INTERNAL_SYSCALL_ERRNO
#define INTERNAL_SYSCALL_ERRNO(val, err) (-(val))

#define inline_syscall0(name,dummy...) \
__asm__ __volatile__ ("syscall 0" \
	              : "+d" (__sc0) \
	              : : "memory")

#define inline_syscall1(name,arg1) \
register long __sc1 asm ("a0") = (long) (arg1); \
inline_syscall0 (name); \
__asm__ __volatile__ ("" : : "r" (__sc1))

#define inline_syscall2(name,arg1,arg2) \
register long __sc2 asm ("d1") = (long) (arg2); \
inline_syscall1 (name,(arg1)); \
__asm__ __volatile__ ("" : : "r" (__sc2))

/* We can't tell whether a3 is going to be eliminated in the enclosing
   function, so we have to assume it isn't.  We first load the value
   of any arguments into their registers, except for a3 itself, that
   may be needed to load the value of the other arguments.  Then, we
   save a3's value in some other register, and load the argument value
   into a3.  We have to force both a3 and its copy to be live in
   different registers at the same time, to avoid having the copy
   spilled and the value reloaded into the same register, in which
   case we'd be unable to get the value of a3 back, should the stack
   slot reference be (offset,a3).  */
#define inline_syscall3(name,arg1,arg2,arg3) \
long __sc3v = (long) (arg3); \
register long __sc1 asm ("a0") = (long) (arg1); \
register long __sc2 asm ("d1") = (long) (arg2); \
register long __sc3 asm ("a3") = __sc3;	\
register long __sc3c; \
__asm__ __volatile__ ("mov %1,%0" : "=&r" (__sc3c) : "r" (__sc3)); \
__sc3 = __sc3v; \
__asm__ __volatile__ ("" : : "r" (__sc3c), "r" (__sc3)); \
inline_syscall0 (name); \
__sc3 = __sc3c; \
__asm__ __volatile__ ("" : : "r" (__sc3), "r" (__sc2), "r" (__sc1))

#ifdef PIC
/* Since a2 is the PIC register, it requires similar handling as a3
   when we're generating PIC, as a2's value may be needed to load
   arguments whose values live in global variables.  The difference is
   that we don't need to require its value to be live in a register;
   it may well be in a stack slot, as long as we save it before
   clobbering a3 and restore it after restoring a3.  */
#define inline_syscall4(name,arg1,arg2,arg3,arg4) \
long __sc4v = (long) (arg4); \
long __sc3v = (long) (arg3); \
register long __sc1 asm ("a0") = (long) (arg1); \
register long __sc2 asm ("d1") = (long) (arg2); \
register long __sc3 asm ("a3") = __sc3;	\
register long __sc3c; \
register long __sc4 asm ("a2") = __sc4;	\
long __sc4c = __sc4; \
__sc4 = __sc4v; \
__asm__ __volatile__ ("mov %1,%0" : "=&r" (__sc3c) : "r" (__sc3)); \
__sc3 = __sc3v; \
__asm__ __volatile__ ("" : : "r" (__sc3c), "r" (__sc3), "r" (__sc4)); \
inline_syscall0 (name); \
__sc3 = __sc3c; \
__sc4 = __sc4c; \
__asm__ __volatile__ ("" : : "r" (__sc4), "r" (__sc3), \
			     "r" (__sc2), "r" (__sc1))
#else
#define inline_syscall4(name,arg1,arg2,arg3,arg4) \
register long __sc4 asm ("a2") = (long) (arg4); \
inline_syscall3 (name,(arg1),(arg2),(arg3)); \
__asm__ __volatile__ ("" : : "r" (__sc4))
#endif

#define inline_syscall5(name,arg1,arg2,arg3,arg4,arg5) \
register long __sc5 asm ("d3") = (long) (arg5); \
inline_syscall4 (name,(arg1),(arg2),(arg3),(arg4)); \
__asm__ __volatile__ ("" : : "r" (__sc5))

#define inline_syscall6(name,arg1,arg2,arg3,arg4,arg5,arg6) \
register long __sc6 asm ("d2") = (long) (arg6); \
inline_syscall5 (name,(arg1),(arg2),(arg3),(arg4),(arg5)); \
__asm__ __volatile__ ("" : : "r" (__sc6))

#endif	/* __ASSEMBLER__ */

#endif /* linux/am33/sysdep.h */
