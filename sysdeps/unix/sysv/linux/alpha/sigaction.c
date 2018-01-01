/* Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <sys/cdefs.h>
#include <stddef.h>

/*
 * In order to get the hidden arguments for rt_sigaction set up
 * properly, we need to call the assembly version.  Detect this in the
 * INLINE_SYSCALL macro, and fail to expand inline in that case.
 */

#undef INLINE_SYSCALL
#define INLINE_SYSCALL(name, nr, args...)       \
        (__NR_##name == __NR_rt_sigaction       \
         ? __syscall_rt_sigaction(args)         \
         : INLINE_SYSCALL1(name, nr, args))

struct kernel_sigaction;
extern int __syscall_rt_sigaction (int, const struct kernel_sigaction *,
				   struct kernel_sigaction *, size_t);

#include <sysdeps/unix/sysv/linux/sigaction.c>
