/* Copyright (C) 1992, 1993, 1995, 1996, 1997, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>, August 1995.

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

#ifndef _LINUX_ALPHA_SYSDEP_H
#define _LINUX_ALPHA_SYSDEP_H 1

#ifdef __ASSEMBLER__

#include <asm/pal.h>
#include <alpha/regdef.h>

#endif

/* There is some commonality.  */
#include <sysdeps/unix/alpha/sysdep.h>

/* For Linux we can use the system call table in the header file
	/usr/include/asm/unistd.h
   of the kernel.  But these symbols do not follow the SYS_* syntax
   so we have to redefine the `SYS_ify' macro here.  */
#undef SYS_ify
#ifdef __STDC__
# define SYS_ify(syscall_name)	__NR_##syscall_name
#else
# define SYS_ify(syscall_name)	__NR_/**/syscall_name
#endif

/* Define some aliases to make automatic syscall generation work
   properly.  The SYS_* variants are for the benefit of the files in
   sysdeps/unix.  */
#define __NR_getpid	__NR_getxpid
#define __NR_getuid	__NR_getxuid
#define __NR_getgid	__NR_getxgid
#define SYS_getpid	__NR_getxpid
#define SYS_getuid	__NR_getxuid
#define SYS_getgid	__NR_getxgid

/*
 * Some syscalls no Linux program should know about:
 */
#define __NR_osf_sigprocmask	 48
#define __NR_osf_shmat		209
#define __NR_osf_getsysinfo	256
#define __NR_osf_setsysinfo	257

/*
 * In order to get the hidden arguments for rt_sigaction set up
 * properly, we need to call the assembly version.  Detect this in the
 * INLINE_SYSCALL macro, and fail to expand inline in that case.
 */

#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)	\
	(__NR_##name == __NR_rt_sigaction	\
	 ? __syscall_##name(args)		\
	 : INLINE_SYSCALL1(name, nr, args))

#define INLINE_SYSCALL1(name, nr, args...)	\
({						\
	long _sc_ret, _sc_err;			\
	inline_syscall##nr(name, args);		\
	if (_sc_err)				\
	  {					\
	    __set_errno (_sc_ret);		\
	    _sc_ret = -1L;			\
	  }					\
	_sc_ret;				\
})

#define inline_syscall_clobbers				\
	"$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8",	\
	"$22", "$23", "$24", "$25", "$27", "$28", "memory"

/* It is moderately important optimization-wise to limit the lifetime
   of the hard-register variables as much as possible.  Thus we copy
   in/out as close to the asm as possible.  */

#define inline_syscall0(name)				\
{							\
	register long _sc_19 __asm__("$19");		\
							\
	__asm__("callsys # %0 %1 <= %2"			\
		: "=v"(_sc_ret), "=r"(_sc_19)		\
		: "0"(__NR_##name)			\
		: inline_syscall_clobbers,		\
		  "$16", "$17", "$18", "$20", "$21");	\
	_sc_err = _sc_19;				\
}

#define inline_syscall1(name,arg1)			\
{							\
	register long _sc_16 __asm__("$16");		\
	register long _sc_19 __asm__("$19");		\
							\
	_sc_16 = (long) (arg1);				\
	__asm__("callsys # %0 %1 <= %2 %3"		\
		: "=v"(_sc_ret), "=r"(_sc_19),		\
		  "=r"(_sc_16)				\
		: "0"(__NR_##name), "2"(_sc_16)		\
		: inline_syscall_clobbers,		\
		  "$17", "$18", "$20", "$21");		\
	_sc_err = _sc_19;				\
}

#define inline_syscall2(name,arg1,arg2)				\
{								\
	register long _sc_16 __asm__("$16");			\
	register long _sc_17 __asm__("$17");			\
	register long _sc_19 __asm__("$19");			\
								\
	_sc_16 = (long) (arg1);					\
	_sc_17 = (long) (arg2);					\
	__asm__("callsys # %0 %1 <= %2 %3 %4"			\
		: "=v"(_sc_ret), "=r"(_sc_19),			\
		  "=r"(_sc_16), "=r"(_sc_17)			\
		: "0"(__NR_##name), "2"(_sc_16), "3"(_sc_17)	\
		: inline_syscall_clobbers,			\
		  "$18", "$20", "$21");				\
	_sc_err = _sc_19;					\
}

#define inline_syscall3(name,arg1,arg2,arg3)			\
{								\
	register long _sc_16 __asm__("$16");			\
	register long _sc_17 __asm__("$17");			\
	register long _sc_18 __asm__("$18");			\
	register long _sc_19 __asm__("$19");			\
								\
	_sc_16 = (long) (arg1);					\
	_sc_17 = (long) (arg2);					\
	_sc_18 = (long) (arg3);					\
	__asm__("callsys # %0 %1 <= %2 %3 %4 %5"		\
		: "=v"(_sc_ret), "=r"(_sc_19),			\
		  "=r"(_sc_16), "=r"(_sc_17), "=r"(_sc_18)	\
		: "0"(__NR_##name), "2"(_sc_16), "3"(_sc_17),	\
		  "4"(_sc_18)					\
		: inline_syscall_clobbers, "$20", "$21");	\
	_sc_err = _sc_19;					\
}

#define inline_syscall4(name,arg1,arg2,arg3,arg4)		\
{								\
	register long _sc_16 __asm__("$16");			\
	register long _sc_17 __asm__("$17");			\
	register long _sc_18 __asm__("$18");			\
	register long _sc_19 __asm__("$19");			\
								\
	_sc_16 = (long) (arg1);					\
	_sc_17 = (long) (arg2);					\
	_sc_18 = (long) (arg3);					\
	_sc_19 = (long) (arg4);					\
	__asm__("callsys # %0 %1 <= %2 %3 %4 %5 %6"		\
		: "=v"(_sc_ret), "=r"(_sc_19),			\
		  "=r"(_sc_16), "=r"(_sc_17), "=r"(_sc_18)	\
		: "0"(__NR_##name), "2"(_sc_16), "3"(_sc_17),	\
		  "4"(_sc_18), "1"(_sc_19)			\
		: inline_syscall_clobbers, "$20", "$21");	\
	_sc_err = _sc_19;					\
}

#define inline_syscall5(name,arg1,arg2,arg3,arg4,arg5)		\
{								\
	register long _sc_16 __asm__("$16");			\
	register long _sc_17 __asm__("$17");			\
	register long _sc_18 __asm__("$18");			\
	register long _sc_19 __asm__("$19");			\
	register long _sc_20 __asm__("$20");			\
								\
	_sc_16 = (long) (arg1);					\
	_sc_17 = (long) (arg2);					\
	_sc_18 = (long) (arg3);					\
	_sc_19 = (long) (arg4);					\
	_sc_20 = (long) (arg5);					\
	__asm__("callsys # %0 %1 <= %2 %3 %4 %5 %6 %7"		\
		: "=v"(_sc_ret), "=r"(_sc_19), 			\
		  "=r"(_sc_16), "=r"(_sc_17), "=r"(_sc_18),	\
		  "=r"(_sc_20)					\
		: "0"(__NR_##name), "2"(_sc_16), "3"(_sc_17),	\
		  "4"(_sc_18), "1"(_sc_19), "5"(_sc_20)		\
		: inline_syscall_clobbers, "$21");		\
	_sc_ret = _sc_0, _sc_err = _sc_19;			\
}

#define inline_syscall6(name,arg1,arg2,arg3,arg4,arg5,arg6)	\
{								\
	register long _sc_16 __asm__("$16");			\
	register long _sc_17 __asm__("$17");			\
	register long _sc_18 __asm__("$18");			\
	register long _sc_19 __asm__("$19");			\
	register long _sc_20 __asm__("$20");			\
	register long _sc_21 __asm__("$21");			\
								\
	_sc_16 = (long) (arg1);					\
	_sc_17 = (long) (arg2);					\
	_sc_18 = (long) (arg3);					\
	_sc_19 = (long) (arg4);					\
	_sc_20 = (long) (arg5);					\
	_sc_21 = (long) (arg6);					\
	__asm__("callsys # %0 %1 <= %2 %3 %4 %5 %6 %7 %8"	\
		: "=v"(_sc_ret), "=r"(_sc_19)			\
		  "=r"(_sc_16), "=r"(_sc_17), "=r"(_sc_18),	\
		  "=r"(_sc_20), "=r"(_sc_21)			\
		: "0"(__NR_##name), "2"(_sc_16), "3"(_sc_17),	\
		  "4"(_sc_18), "1"(_sc_19), "5"(_sc_20),	\
		  "6"(_sc_21)					\
		: inline_syscall_clobbers);			\
	_sc_err = _sc_19;					\
}

#endif /* _LINUX_ALPHA_SYSDEP_H */
