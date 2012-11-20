/* Copyright (C) 2000, 2002, 2003, 2004, 2005, 2006, 2009, 2012
   Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _LINUX_MIPS_SYSDEP_H
#define _LINUX_MIPS_SYSDEP_H 1

/* There is some commonality.  */
#include <sysdeps/unix/mips/mips64/n64/sysdep.h>

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
#define SYS_ify(syscall_name)	__NR_##syscall_name

#ifdef __ASSEMBLER__

/* We don't want the label for the error handler to be visible in the symbol
   table when we define it here.  */
#ifdef __PIC__
# define SYSCALL_ERROR_LABEL 99b
#endif

#else   /* ! __ASSEMBLER__ */

/* Define a macro which expands into the inline wrapper code for a system
   call.  */
#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)                               \
  ({ INTERNAL_SYSCALL_DECL(err);					\
     long result_var = INTERNAL_SYSCALL (name, err, nr, args);      	\
     if ( INTERNAL_SYSCALL_ERROR_P (result_var, err) )  		\
       {                                                                \
         __set_errno (INTERNAL_SYSCALL_ERRNO (result_var, err));      	\
         result_var = -1L;                               		\
       }                                                                \
     result_var; })

#undef INTERNAL_SYSCALL_DECL
#define INTERNAL_SYSCALL_DECL(err) long err __attribute__ ((unused))

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err)   ((void) (val), (long) (err))

#undef INTERNAL_SYSCALL_ERRNO
#define INTERNAL_SYSCALL_ERRNO(val, err)     ((void) (err), val)

#undef INTERNAL_SYSCALL
#define INTERNAL_SYSCALL(name, err, nr, args...) \
	internal_syscall##nr (, "li\t$2, %2\t\t\t# " #name "\n\t",	\
			      "i" (SYS_ify (name)), err, args)

#undef INTERNAL_SYSCALL_NCS
#define INTERNAL_SYSCALL_NCS(number, err, nr, args...) \
	internal_syscall##nr (= number, , "r" (__v0), err, args)

#define internal_syscall0(ncs_init, cs_init, input, err, dummy...)	\
({ 									\
	long _sys_result;						\
									\
	{								\
	register long __v0 asm("$2") ncs_init;				\
	register long __a3 asm("$7");					\
	__asm__ volatile ( 						\
	".set\tnoreorder\n\t" 						\
	cs_init								\
	"syscall\n\t" 							\
	".set reorder" 							\
	: "=r" (__v0), "=r" (__a3) 					\
	: input								\
	: __SYSCALL_CLOBBERS); 						\
	err = __a3;							\
	_sys_result = __v0;						\
	}								\
	_sys_result;							\
})

#define internal_syscall1(ncs_init, cs_init, input, err, arg1)		\
({ 									\
	long _sys_result;						\
									\
	{								\
	register long __v0 asm("$2") ncs_init;				\
	register long __a0 asm("$4") = (long) (arg1); 			\
	register long __a3 asm("$7"); 					\
	__asm__ volatile ( 						\
	".set\tnoreorder\n\t" 						\
	cs_init								\
	"syscall\n\t" 							\
	".set reorder" 							\
	: "=r" (__v0), "=r" (__a3) 					\
	: input, "r" (__a0)		 				\
	: __SYSCALL_CLOBBERS); 						\
	err = __a3;							\
	_sys_result = __v0;						\
	}								\
	_sys_result;							\
})

#define internal_syscall2(ncs_init, cs_init, input, err, arg1, arg2)	\
({ 									\
	long _sys_result;						\
									\
	{								\
	register long __v0 asm("$2") ncs_init;				\
	register long __a0 asm("$4") = (long) (arg1); 			\
	register long __a1 asm("$5") = (long) (arg2); 			\
	register long __a3 asm("$7"); 					\
	__asm__ volatile ( 						\
	".set\tnoreorder\n\t" 						\
	cs_init								\
	"syscall\n\t" 							\
	".set\treorder" 						\
	: "=r" (__v0), "=r" (__a3) 					\
	: input, "r" (__a0), "r" (__a1)					\
	: __SYSCALL_CLOBBERS); 						\
	err = __a3;							\
	_sys_result = __v0;						\
	}								\
	_sys_result;							\
})

#define internal_syscall3(ncs_init, cs_init, input, err, arg1, arg2, arg3) \
({ 									\
	long _sys_result;						\
									\
	{								\
	register long __v0 asm("$2") ncs_init;				\
	register long __a0 asm("$4") = (long) (arg1); 			\
	register long __a1 asm("$5") = (long) (arg2); 			\
	register long __a2 asm("$6") = (long) (arg3); 			\
	register long __a3 asm("$7"); 					\
	__asm__ volatile ( 						\
	".set\tnoreorder\n\t" 						\
	cs_init								\
	"syscall\n\t" 							\
	".set\treorder" 						\
	: "=r" (__v0), "=r" (__a3) 					\
	: input, "r" (__a0), "r" (__a1), "r" (__a2)			\
	: __SYSCALL_CLOBBERS); 						\
	err = __a3;							\
	_sys_result = __v0;						\
	}								\
	_sys_result;							\
})

#define internal_syscall4(ncs_init, cs_init, input, err, arg1, arg2, arg3, arg4) \
({ 									\
	long _sys_result;						\
									\
	{								\
	register long __v0 asm("$2") ncs_init;				\
	register long __a0 asm("$4") = (long) (arg1); 			\
	register long __a1 asm("$5") = (long) (arg2); 			\
	register long __a2 asm("$6") = (long) (arg3); 			\
	register long __a3 asm("$7") = (long) (arg4); 			\
	__asm__ volatile ( 						\
	".set\tnoreorder\n\t" 						\
	cs_init								\
	"syscall\n\t" 							\
	".set\treorder" 						\
	: "=r" (__v0), "+r" (__a3) 					\
	: input, "r" (__a0), "r" (__a1), "r" (__a2)		 	\
	: __SYSCALL_CLOBBERS); 						\
	err = __a3;							\
	_sys_result = __v0;						\
	}								\
	_sys_result;							\
})

#define internal_syscall5(ncs_init, cs_init, input, err, arg1, arg2, arg3, arg4, arg5) \
({ 									\
	long _sys_result;						\
									\
	{								\
	register long __v0 asm("$2") ncs_init;				\
	register long __a0 asm("$4") = (long) (arg1); 			\
	register long __a1 asm("$5") = (long) (arg2); 			\
	register long __a2 asm("$6") = (long) (arg3); 			\
	register long __a3 asm("$7") = (long) (arg4); 			\
	register long __a4 asm("$8") = (long) (arg5); 			\
	__asm__ volatile ( 						\
	".set\tnoreorder\n\t" 						\
	cs_init								\
	"syscall\n\t" 							\
	".set\treorder" 						\
	: "=r" (__v0), "+r" (__a3) 					\
	: input, "r" (__a0), "r" (__a1), "r" (__a2), "r" (__a4)		\
	: __SYSCALL_CLOBBERS); 						\
	err = __a3;							\
	_sys_result = __v0;						\
	}								\
	_sys_result;							\
})

#define internal_syscall6(ncs_init, cs_init, input, err, arg1, arg2, arg3, arg4, arg5, arg6) \
({ 									\
	long _sys_result;						\
									\
	{								\
	register long __v0 asm("$2") ncs_init;				\
	register long __a0 asm("$4") = (long) (arg1); 			\
	register long __a1 asm("$5") = (long) (arg2); 			\
	register long __a2 asm("$6") = (long) (arg3); 			\
	register long __a3 asm("$7") = (long) (arg4); 			\
	register long __a4 asm("$8") = (long) (arg5); 			\
	register long __a5 asm("$9") = (long) (arg6); 			\
	__asm__ volatile ( 						\
	".set\tnoreorder\n\t" 						\
	cs_init								\
	"syscall\n\t" 							\
	".set\treorder" 						\
	: "=r" (__v0), "+r" (__a3) 					\
	: input, "r" (__a0), "r" (__a1), "r" (__a2), "r" (__a4),	\
	  "r" (__a5)							\
	: __SYSCALL_CLOBBERS); 						\
	err = __a3;							\
	_sys_result = __v0;						\
	}								\
	_sys_result;							\
})

#define __SYSCALL_CLOBBERS "$1", "$3", "$10", "$11", "$12", "$13", \
	"$14", "$15", "$24", "$25", "hi", "lo", "memory"
#endif /* __ASSEMBLER__ */

/* Pointer mangling is not yet supported for MIPS.  */
#define PTR_MANGLE(var) (void) (var)
#define PTR_DEMANGLE(var) (void) (var)

#endif /* linux/mips/sysdep.h */
