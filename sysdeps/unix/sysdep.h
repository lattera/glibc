/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
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

#include <sysdeps/generic/sysdep.h>

#include <sys/syscall.h>
#define	HAVE_SYSCALLS

/* Note that using a `PASTE' macro loses.  */
#define	SYSCALL__(name, args)	PSEUDO (__##name, name, args)
#define	SYSCALL(name, args)	PSEUDO (name, name, args)

#define __SYSCALL0(name) \
  INLINE_SYSCALL (name, 0)
#define __SYSCALL1(name, a1) \
  INLINE_SYSCALL (name, 1, a1)
#define __SYSCALL2(name, a1, a2) \
  INLINE_SYSCALL (name, 2, a1, a2)
#define __SYSCALL3(name, a1, a2, a3) \
  INLINE_SYSCALL (name, 3, a1, a2, a3)
#define __SYSCALL4(name, a1, a2, a3, a4) \
  INLINE_SYSCALL (name, 4, a1, a2, a3, a4)
#define __SYSCALL5(name, a1, a2, a3, a4, a5) \
  INLINE_SYSCALL (name, 5, a1, a2, a3, a4, a5)
#define __SYSCALL6(name, a1, a2, a3, a4, a5, a6) \
  INLINE_SYSCALL (name, 6, a1, a2, a3, a4, a5, a6)
#define __SYSCALL7(name, a1, a2, a3, a4, a5, a6, a7) \
  INLINE_SYSCALL (name, 7, a1, a2, a3, a4, a5, a6, a7)

#define __SYSCALL_NARGS_X(a,b,c,d,e,f,g,h,n,...) n
#define __SYSCALL_NARGS(...) \
  __SYSCALL_NARGS_X (__VA_ARGS__,7,6,5,4,3,2,1,0,)
#define __SYSCALL_CONCAT_X(a,b)     a##b
#define __SYSCALL_CONCAT(a,b)       __SYSCALL_CONCAT_X (a, b)
#define __SYSCALL_DISP(b,...) \
  __SYSCALL_CONCAT (b,__SYSCALL_NARGS(__VA_ARGS__))(__VA_ARGS__)

#define __SYSCALL_CALL(...) __SYSCALL_DISP (__SYSCALL, __VA_ARGS__)

#define SYSCALL_CANCEL(...) \
  ({									     \
    long int sc_ret;							     \
    if (SINGLE_THREAD_P) 						     \
      sc_ret = __SYSCALL_CALL (__VA_ARGS__);   				     \
    else								     \
      {									     \
	int sc_cancel_oldtype = LIBC_CANCEL_ASYNC ();			     \
	sc_ret = __SYSCALL_CALL (__VA_ARGS__);				     \
        LIBC_CANCEL_RESET (sc_cancel_oldtype);				     \
      }									     \
    sc_ret;								     \
  })

/* Machine-dependent sysdep.h files are expected to define the macro
   PSEUDO (function_name, syscall_name) to emit assembly code to define the
   C-callable function FUNCTION_NAME to do system call SYSCALL_NAME.
   r0 and r1 are the system call outputs.  MOVE(x, y) should be defined as
   an instruction such that "MOVE(r1, r0)" works.  ret should be defined
   as the return instruction.  */

#ifndef SYS_ify
#define SYS_ify(syscall_name) SYS_##syscall_name
#endif

/* Terminate a system call named SYM.  This is used on some platforms
   to generate correct debugging information.  */
#ifndef PSEUDO_END
#define PSEUDO_END(sym)
#endif
#ifndef PSEUDO_END_NOERRNO
#define PSEUDO_END_NOERRNO(sym)	PSEUDO_END(sym)
#endif
#ifndef PSEUDO_END_ERRVAL
#define PSEUDO_END_ERRVAL(sym)	PSEUDO_END(sym)
#endif

/* Wrappers around system calls should normally inline the system call code.
   But sometimes it is not possible or implemented and we use this code.  */
#ifndef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...) __syscall_##name (args)
#endif
