/* Copyright (C) 1992, 1993, 1995, 1996, 1997 Free Software Foundation, Inc.
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
