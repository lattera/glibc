/* Copyright (C) 2005-2012 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _LINUX_AARCH64_SYSDEP_H
#define _LINUX_AARCH64_SYSDEP_H 1

#include <sysdeps/unix/sysdep.h>
#include <sysdeps/aarch64/sysdep.h>
#include <sysdeps/unix/sysv/linux/generic/sysdep.h>

/* Defines RTLD_PRIVATE_ERRNO and USE_DL_SYSINFO.  */
#include <dl-sysdep.h>

#include <tls.h>

/* In order to get __set_errno() definition in INLINE_SYSCALL.  */
#ifndef __ASSEMBLER__
#include <errno.h>
#endif

/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#define SYS_ify(syscall_name)	(__NR_##syscall_name)

#ifdef __ASSEMBLER__

/* Linux uses a negative return value to indicate syscall errors,
   unlike most Unices, which use the condition codes' carry flag.

   Since version 2.1 the return value of a system call might be
   negative even if the call succeeded.  E.g., the `lseek' system call
   might return a large offset.  Therefore we must not anymore test
   for < 0, but test for a real error by making sure the value in R0
   is a real error number.  Linus said he will make sure the no syscall
   returns a value in -1 .. -4095 as a valid result so we can safely
   test with -4095.  */

# undef	PSEUDO
# define PSEUDO(name, syscall_name, args)				      \
  .text;								      \
  ENTRY (name);								      \
    DO_CALL (syscall_name, args);					      \
    cmn x0, #4095;

/* Notice the use of 'RET' instead of 'ret' the assembler is case
   insensitive and eglibc already uses the preprocessor symbol 'ret'
   so we use the upper case 'RET' to force through a ret instruction
   to the assembler */
# define PSEUDO_RET							      \
    b.cs 1f;								      \
    RET;								      \
    1:                                                                        \
    b SYSCALL_ERROR
# undef ret
# define ret PSEUDO_RET

# undef	PSEUDO_END
# define PSEUDO_END(name)						      \
  SYSCALL_ERROR_HANDLER							      \
  END (name)

# undef	PSEUDO_NOERRNO
# define PSEUDO_NOERRNO(name, syscall_name, args)			      \
  .text;								      \
  ENTRY (name);								      \
    DO_CALL (syscall_name, args);

/* Notice the use of 'RET' instead of 'ret' the assembler is case
   insensitive and eglibc already uses the preprocessor symbol 'ret'
   so we use the upper case 'RET' to force through a ret instruction
   to the assembler */
# define PSEUDO_RET_NOERRNO						      \
    RET;

# undef ret_NOERRNO
# define ret_NOERRNO PSEUDO_RET_NOERRNO

# undef	PSEUDO_END_NOERRNO
# define PSEUDO_END_NOERRNO(name)					      \
  END (name)

/* The function has to return the error code.  */
# undef	PSEUDO_ERRVAL
# define PSEUDO_ERRVAL(name, syscall_name, args) \
  .text;								      \
  ENTRY (name)								      \
    DO_CALL (syscall_name, args);					      \
    neg x0, x0

# undef	PSEUDO_END_ERRVAL
# define PSEUDO_END_ERRVAL(name) \
  END (name)

# define ret_ERRVAL PSEUDO_RET_NOERRNO

# if NOT_IN_libc
#  define SYSCALL_ERROR __local_syscall_error
#  if RTLD_PRIVATE_ERRNO
#   define SYSCALL_ERROR_HANDLER				\
__local_syscall_error:						\
	adrp	x1, C_SYMBOL_NAME(rtld_errno);			\
	add	x1, x1, #:lo12:C_SYMBOL_NAME(rtld_errno);	\
	neg     w0, w0;						\
	str     w0, [x1];					\
	mov	x0, -1;						\
	RET;
#  else

#   define SYSCALL_ERROR_HANDLER				\
__local_syscall_error:						\
	stp     x29, x30, [sp, -32]!;				\
	cfi_adjust_cfa_offset (32);				\
	cfi_rel_offset (x29, 0);				\
	cfi_rel_offset (x30, 8);				\
        add     x29, sp, 0;					\
        str     x19, [sp,16];					\
	neg	x19, x0;					\
	bl	C_SYMBOL_NAME(__errno_location);		\
	str	x19, [x0];					\
	mov	x0, -1;						\
        ldr     x19, [sp,16];					\
        ldp     x29, x30, [sp], 32;				\
	cfi_adjust_cfa_offset (-32);				\
	cfi_restore (x29);					\
	cfi_restore (x30);					\
	RET;
#  endif
# else
#  define SYSCALL_ERROR_HANDLER	/* Nothing here; code in sysdep.S is used.  */
#  define SYSCALL_ERROR __syscall_error
# endif

/* Linux takes system call args in registers:
	syscall number	in the SVC instruction
	arg 1		x0
	arg 2		x1
	arg 3		x2
	arg 4		x3
	arg 5		x4
	arg 6		x5
	arg 7		x6

   The compiler is going to form a call by coming here, through PSEUDO, with
   arguments
	syscall number	in the DO_CALL macro
	arg 1		x0
	arg 2		x1
	arg 3		x2
	arg 4		x3
	arg 5		x4
	arg 6		x5
	arg 7		x6

   We need to shuffle values between R4..R6 and the stack so that the
   caller's v1..v3 and stack frame are not corrupted, and the kernel
   sees the right arguments.

*/

# undef	DO_CALL
# define DO_CALL(syscall_name, args)		\
    DOARGS_##args				\
    mov x8, SYS_ify (syscall_name);		\
    svc 0;					\
    UNDOARGS_##args

# define DOARGS_0 /* nothing */
# define DOARGS_1 /* nothing */
# define DOARGS_2 /* nothing */
# define DOARGS_3 /* nothing */
# define DOARGS_4 /* nothing */
# define DOARGS_5 /* nothing */
# define DOARGS_6 /* nothing */
# define DOARGS_7 /* nothing */

# define UNDOARGS_0 /* nothing */
# define UNDOARGS_1 /* nothing */
# define UNDOARGS_2 /* nothing */
# define UNDOARGS_3 /* nothing */
# define UNDOARGS_4 /* nothing */
# define UNDOARGS_5 /* nothing */
# define UNDOARGS_6 /* nothing */
# define UNDOARGS_7 /* nothing */

#else /* not __ASSEMBLER__ */

# ifdef SHARED
#  define INLINE_VSYSCALL(name, nr, args...)				      \
  ({									      \
    __label__ out;							      \
    __label__ iserr;							      \
    long sc_ret;							      \
    INTERNAL_SYSCALL_DECL (sc_err);					      \
									      \
    if (__vdso_##name != NULL)						      \
      {									      \
	sc_ret = INTERNAL_VSYSCALL_NCS (__vdso_##name, sc_err, nr, ##args);   \
	if (!INTERNAL_SYSCALL_ERROR_P (sc_ret, sc_err))			      \
	  goto out;							      \
	if (INTERNAL_SYSCALL_ERRNO (sc_ret, sc_err) != ENOSYS)		      \
	  goto iserr;							      \
      }									      \
									      \
    sc_ret = INTERNAL_SYSCALL (name, sc_err, nr, ##args);		      \
    if (INTERNAL_SYSCALL_ERROR_P (sc_ret, sc_err))			      \
      {									      \
      iserr:								      \
        __set_errno (INTERNAL_SYSCALL_ERRNO (sc_ret, sc_err));		      \
        sc_ret = -1L;							      \
      }									      \
  out:									      \
    sc_ret;								      \
  })
# else
#  define INLINE_VSYSCALL(name, nr, args...) \
  INLINE_SYSCALL (name, nr, ##args)
# endif

# ifdef SHARED
#  define INTERNAL_VSYSCALL(name, err, nr, args...)			      \
  ({									      \
    __label__ out;							      \
    long v_ret;								      \
									      \
    if (__vdso_##name != NULL)						      \
      {									      \
	v_ret = INTERNAL_VSYSCALL_NCS (__vdso_##name, err, nr, ##args);	      \
	if (!INTERNAL_SYSCALL_ERROR_P (v_ret, err)			      \
	    || INTERNAL_SYSCALL_ERRNO (v_ret, err) != ENOSYS)		      \
	  goto out;							      \
      }									      \
    v_ret = INTERNAL_SYSCALL (name, err, nr, ##args);			      \
  out:									      \
    v_ret;								      \
  })
# else
#  define INTERNAL_VSYSCALL(name, err, nr, args...) \
  INTERNAL_SYSCALL (name, err, nr, ##args)
# endif

/* List of system calls which are supported as vsyscalls.  */
# define HAVE_CLOCK_GETRES_VSYSCALL	1
# define HAVE_CLOCK_GETTIME_VSYSCALL	1

# define INTERNAL_VSYSCALL_NCS(funcptr, err, nr, args...)	\
  ({								\
    LOAD_ARGS_##nr (args)					\
    asm volatile ("blr %1"					\
		  : "=r" (_x0)					\
		  : "r" (funcptr), ASM_ARGS_##nr		\
		  : "x30", "memory");				\
    (long) _x0;							\
  })


/* Define a macro which expands into the inline wrapper code for a system
   call.  */
# undef INLINE_SYSCALL
# define INLINE_SYSCALL(name, nr, args...)				\
  ({ unsigned long _sys_result = INTERNAL_SYSCALL (name, , nr, args);	\
     if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (_sys_result, ), 0))\
       {								\
	 __set_errno (INTERNAL_SYSCALL_ERRNO (_sys_result, ));		\
	 _sys_result = (unsigned long) -1;				\
       }								\
     (long) _sys_result; })

# undef INTERNAL_SYSCALL_DECL
# define INTERNAL_SYSCALL_DECL(err) do { } while (0)

# undef INTERNAL_SYSCALL_RAW
# define INTERNAL_SYSCALL_RAW(name, err, nr, args...)		\
  ({ unsigned long _sys_result;					\
     {								\
       LOAD_ARGS_##nr (args)					\
       register long _x8 asm ("x8") = (name);			\
       asm volatile ("svc	0	// syscall " # name     \
		     : "+r" (_x0), "+r" (_x8)			\
		     : ASM_ARGS_##nr				\
		     : "memory", CLOBBER_ARGS_##nr);		\
       _sys_result = _x0;					\
     }								\
     (long) _sys_result; })

# undef INTERNAL_SYSCALL
# define INTERNAL_SYSCALL(name, err, nr, args...)		\
	INTERNAL_SYSCALL_RAW(SYS_ify(name), err, nr, args)

# undef INTERNAL_SYSCALL_AARCH64
# define INTERNAL_SYSCALL_AARCH64(name, err, nr, args...)	\
	INTERNAL_SYSCALL_RAW(__ARM_NR_##name, err, nr, args)

# undef INTERNAL_SYSCALL_ERROR_P
# define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((unsigned long) (val) >= (unsigned long) -4095)

# undef INTERNAL_SYSCALL_ERRNO
# define INTERNAL_SYSCALL_ERRNO(val, err)	(-(val))

# define CLOBBER_ARGS_0       CLOBBER_ARGS_1
# define CLOBBER_ARGS_1 "x1", CLOBBER_ARGS_2
# define CLOBBER_ARGS_2 "x2", CLOBBER_ARGS_3
# define CLOBBER_ARGS_3 "x3", CLOBBER_ARGS_4
# define CLOBBER_ARGS_4 "x4", CLOBBER_ARGS_5
# define CLOBBER_ARGS_5 "x5", CLOBBER_ARGS_6
# define CLOBBER_ARGS_6 "x6", CLOBBER_ARGS_7
# define CLOBBER_ARGS_7 \
  "x7", "x9", "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18"

# define LOAD_ARGS_0()				\
  register long _x0 asm ("x0");

# define ASM_ARGS_0
# define LOAD_ARGS_1(x0)			\
  long _x0tmp = (long) (x0);			\
  LOAD_ARGS_0 ()				\
  _x0 = _x0tmp;
# define ASM_ARGS_1	"r" (_x0)
# define LOAD_ARGS_2(x0, x1)			\
  long _x1tmp = (long) (x1);			\
  LOAD_ARGS_1 (x0)				\
  register long _x1 asm ("x1") = _x1tmp;
# define ASM_ARGS_2	ASM_ARGS_1, "r" (_x1)
# define LOAD_ARGS_3(x0, x1, x2)		\
  long _x2tmp = (long) (x2);			\
  LOAD_ARGS_2 (x0, x1)				\
  register long _x2 asm ("x2") = _x2tmp;
# define ASM_ARGS_3	ASM_ARGS_2, "r" (_x2)
# define LOAD_ARGS_4(x0, x1, x2, x3)		\
  long _x3tmp = (long) (x3);			\
  LOAD_ARGS_3 (x0, x1, x2)			\
  register long _x3 asm ("x3") = _x3tmp;
# define ASM_ARGS_4	ASM_ARGS_3, "r" (_x3)
# define LOAD_ARGS_5(x0, x1, x2, x3, x4)	\
  long _x4tmp = (long) (x4);			\
  LOAD_ARGS_4 (x0, x1, x2, x3)			\
  register long _x4 asm ("x4") = _x4tmp;
# define ASM_ARGS_5	ASM_ARGS_4, "r" (_x4)
# define LOAD_ARGS_6(x0, x1, x2, x3, x4, x5)	\
  long _x5tmp = (long) (x5);			\
  LOAD_ARGS_5 (x0, x1, x2, x3, x4)		\
  register long _x5 asm ("x5") = _x5tmp;
# define ASM_ARGS_6	ASM_ARGS_5, "r" (_x5)
# define LOAD_ARGS_7(x0, x1, x2, x3, x4, x5, x6)\
  long _x6tmp = (long) (x6);			\
  LOAD_ARGS_6 (x0, x1, x2, x3, x4, x5)		\
  register long _x6 asm ("x6") = _x6tmp;
# define ASM_ARGS_7	ASM_ARGS_6, "r" (_x6)

# undef INTERNAL_SYSCALL_NCS
# define INTERNAL_SYSCALL_NCS(number, err, nr, args...)	\
	INTERNAL_SYSCALL_RAW (number, err, nr, args)

#endif	/* __ASSEMBLER__ */

/* Pointer mangling is not yet supported for AArch64.  */
#define PTR_MANGLE(var) (void) (var)
#define PTR_DEMANGLE(var) (void) (var)

#endif /* linux/aarch64/sysdep.h */
