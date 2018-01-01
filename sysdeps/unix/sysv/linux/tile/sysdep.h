/* Copyright (C) 2011-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <asm/unistd.h>
#include <sysdeps/tile/sysdep.h>
#include <sysdeps/unix/sysv/linux/generic/sysdep.h>
#include <sys/syscall.h>

#undef SYS_ify
#define SYS_ify(syscall_name)	__NR_##syscall_name


#ifdef __ASSEMBLER__

/* The actual implementation of doing a syscall. */
#define DO_CALL(syscall_name, args)                     \
  moveli TREG_SYSCALL_NR_NAME, SYS_ify(syscall_name);	\
  swint1

/* TILE Linux returns the result in r0 (or a negative errno).
   The kernel "owns" the code to decide if a given value is an error,
   and puts errno in r1 if so, or otherwise zero.  */
#define	PSEUDO(name, syscall_name, args)		\
  ENTRY	(name);						\
  DO_CALL(syscall_name, args);				\
  BNEZ r1, 0f

#define ret  jrp lr

#ifndef SHARED
/* For static code, on error jump to __syscall_error directly. */
# define SYSCALL_ERROR_NAME __syscall_error
#elif IS_IN (libc) || IS_IN (libpthread)
/* Use the internal name for libc/libpthread shared objects. */
# define SYSCALL_ERROR_NAME __GI___syscall_error
#else
/* Otherwise, on error do a full PLT jump. */
# define SYSCALL_ERROR_NAME plt(__syscall_error)
#endif

#undef PSEUDO_END
#define	PSEUDO_END(name)				\
0:							\
  j SYSCALL_ERROR_NAME;					\
  END (name)

#undef PSEUDO_NOERRNO
#define	PSEUDO_NOERRNO(name, syscall_name, args)	\
  ENTRY	(name);						\
  DO_CALL(syscall_name, args)

#define ret_NOERRNO  jrp lr

#undef PSEUDO_END_NOERRNO
#define	PSEUDO_END_NOERRNO(name) \
  END (name)

/* Convenience wrappers. */
#define SYSCALL__(name, args)   PSEUDO (__##name, name, args)
#define SYSCALL(name, args)   PSEUDO (name, name, args)

#else /* not __ASSEMBLER__ */

#include <errno.h>

/* Define a macro which expands inline into the wrapper code for a system
   call.  */
# undef INLINE_SYSCALL
# define INLINE_SYSCALL(name, nr, args...)                              \
  ({                                                                    \
    INTERNAL_SYSCALL_DECL (_sc_err);                                    \
    unsigned long _sc_val = INTERNAL_SYSCALL (name, _sc_err, nr, args); \
    if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (_sc_val, _sc_err), 0)) \
    {                                                                   \
      __set_errno (INTERNAL_SYSCALL_ERRNO (_sc_val, _sc_err));          \
      _sc_val = -1;                                                     \
    }                                                                   \
    (long) _sc_val;                                                     \
  })

#undef INTERNAL_SYSCALL
#define INTERNAL_SYSCALL(name, err, nr, args...)        \
  internal_syscall##nr (SYS_ify (name), err, args)

#undef INTERNAL_SYSCALL_NCS
#define INTERNAL_SYSCALL_NCS(number, err, nr, args...)  \
  internal_syscall##nr (number, err, args)

#undef INTERNAL_SYSCALL_DECL
#define INTERNAL_SYSCALL_DECL(err) int err

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err) ({ (void) (val); (err) != 0; })

#undef INTERNAL_SYSCALL_ERRNO
#define INTERNAL_SYSCALL_ERRNO(val, err) ({ (void) (val); (err); })

#define internal_syscall0(num, err, dummy...)                           \
  ({                                                                    \
    long _sys_result, __SYSCALL_CLOBBER_DECLS;                          \
    __asm__ __volatile__ (                                              \
      "swint1"                                                          \
      : "=R00" (_sys_result), "=R01" (err), __SYSCALL_CLOBBER_OUTPUTS   \
      : "R10" (num)                                                     \
      : __SYSCALL_CLOBBERS);                                            \
    _sys_result;                                                        \
  })

#define internal_syscall1(num, err, arg0)                               \
  ({                                                                    \
    long _sys_result, __SYSCALL_CLOBBER_DECLS;                          \
    __asm__ __volatile__ (                                              \
      "swint1"                                                          \
      : "=R00" (_sys_result), "=R01" (err), __SYSCALL_CLOBBER_OUTPUTS   \
      : "R10" (num), "R00" (arg0)                                       \
      : __SYSCALL_CLOBBERS);                                            \
    _sys_result;                                                        \
  })

#define internal_syscall2(num, err, arg0, arg1)                         \
  ({                                                                    \
    long _sys_result, __SYSCALL_CLOBBER_DECLS;                          \
    __asm__ __volatile__ (                                              \
      "swint1"                                                          \
      : "=R00" (_sys_result), "=R01" (err), __SYSCALL_CLOBBER_OUTPUTS   \
      : "R10" (num), "R00" (arg0), "R01" (arg1)                         \
      : __SYSCALL_CLOBBERS);                                            \
    _sys_result;                                                        \
  })

#define internal_syscall3(num, err, arg0, arg1, arg2)                   \
  ({                                                                    \
    long _sys_result, __SYSCALL_CLOBBER_DECLS;                          \
    __asm__ __volatile__ (                                              \
      "swint1"                                                          \
      : "=R00" (_sys_result), "=R01" (err), __SYSCALL_CLOBBER_OUTPUTS   \
      : "R10" (num), "R00" (arg0), "R01" (arg1), "R02" (arg2)           \
      : __SYSCALL_CLOBBERS);                                            \
    _sys_result;                                                        \
  })

#define internal_syscall4(num, err, arg0, arg1, arg2, arg3)             \
  ({                                                                    \
    long _sys_result, __SYSCALL_CLOBBER_DECLS;                          \
    __asm__ __volatile__ (                                              \
      "swint1"                                                          \
      : "=R00" (_sys_result), "=R01" (err), __SYSCALL_CLOBBER_OUTPUTS   \
      : "R10" (num), "R00" (arg0), "R01" (arg1), "R02" (arg2),          \
        "R03" (arg3)                                                    \
      : __SYSCALL_CLOBBERS);                                            \
    _sys_result;                                                        \
  })

#define internal_syscall5(num, err, arg0, arg1, arg2, arg3, arg4)       \
  ({                                                                    \
    long _sys_result, __SYSCALL_CLOBBER_DECLS;                          \
    __asm__ __volatile__ (                                              \
      "swint1"                                                          \
      : "=R00" (_sys_result), "=R01" (err), __SYSCALL_CLOBBER_OUTPUTS   \
      : "R10" (num), "R00" (arg0), "R01" (arg1), "R02" (arg2),          \
        "R03" (arg3), "R04" (arg4)                                      \
      : __SYSCALL_CLOBBERS);                                            \
    _sys_result;                                                        \
  })

#define internal_syscall6(num, err, arg0, arg1, arg2, arg3, arg4, arg5) \
  ({                                                                    \
    long _sys_result, __SYSCALL_CLOBBER_DECLS;                          \
    __asm__ __volatile__ (                                              \
      "swint1"                                                          \
      : "=R00" (_sys_result), "=R01" (err), __SYSCALL_CLOBBER_OUTPUTS   \
      : "R10" (num), "R00" (arg0), "R01" (arg1), "R02" (arg2),          \
        "R03" (arg3), "R04" (arg4), "R05" (arg5)                        \
      : __SYSCALL_CLOBBERS);                                            \
    _sys_result;                                                        \
  })

#undef __SYSCALL_CLOBBERS
#define __SYSCALL_CLOBBERS                                      \
  "r6",  "r7",                                                  \
    "r8",  "r9",        "r11", "r12", "r13", "r14", "r15",      \
    "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23",     \
    "r24", "r25", "r26", "r27", "r28", "r29", "memory"

/* gcc doesn't seem to allow an input operand to be clobbered, so we
   fake it with dummy outputs. */
#define __SYSCALL_CLOBBER_DECLS                                         \
  _clobber_r2, _clobber_r3, _clobber_r4, _clobber_r5, _clobber_r10

#define __SYSCALL_CLOBBER_OUTPUTS                                       \
  "=R02" (_clobber_r2), "=R03" (_clobber_r3), "=R04" (_clobber_r4),     \
    "=R05" (_clobber_r5), "=R10" (_clobber_r10)


#define INTERNAL_VSYSCALL_CALL(funcptr, err, nr, args...)               \
  ({                                                                    \
    struct syscall_return_value _sc_rv = funcptr (args);                \
    err = _sc_rv.error;                                                 \
    _sc_rv.value;                                                       \
  })

/* List of system calls which are supported as vsyscalls.  */
#define HAVE_CLOCK_GETTIME_VSYSCALL	1
#define HAVE_GETTIMEOFDAY_VSYSCALL	1

/* Previously tile used the generic version without the libc_hidden_def
   which lead in a non existent __send symbol in libc.so.  */
#undef HAVE_INTERNAL_SEND_SYMBOL

#endif /* __ASSEMBLER__  */

/* Pointer mangling support.  */
#if IS_IN (rtld)
/* We cannot use the thread descriptor because in ld.so we use setjmp
   earlier than the descriptor is initialized.  */
#else
# ifdef __ASSEMBLER__
#  define PTR_MANGLE(reg, tmpreg) \
	ADDLI_PTR tmpreg, pt, POINTER_GUARD; \
	LD	tmpreg, tmpreg; \
	xor	reg, tmpreg, reg
#  define PTR_DEMANGLE(reg, tmpreg) PTR_MANGLE (reg, tmpreg)
# else
#  define PTR_MANGLE(var) \
  (var) = (__typeof (var)) ((uintptr_t) (var) ^ THREAD_GET_POINTER_GUARD ())
#  define PTR_DEMANGLE(var)	PTR_MANGLE (var)
# endif
#endif
