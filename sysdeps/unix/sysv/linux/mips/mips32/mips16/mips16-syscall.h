/* MIPS16 syscall wrappers.
   Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

#ifndef MIPS16_SYSCALL_H
#define MIPS16_SYSCALL_H 1

long long __nomips16 __mips16_syscall0 (long number);
#define __mips16_syscall0(dummy, number)				\
	__mips16_syscall0 ((long) (number))

long long __nomips16 __mips16_syscall1 (long a0,
					long number);
#define __mips16_syscall1(a0, number)					\
	__mips16_syscall1 ((long) (a0),					\
			   (long) (number))

long long __nomips16 __mips16_syscall2 (long a0, long a1,
					long number);
#define __mips16_syscall2(a0, a1, number)				\
	__mips16_syscall2 ((long) (a0), (long) (a1),			\
			   (long) (number))

long long __nomips16 __mips16_syscall3 (long a0, long a1, long a2,
					long number);
#define __mips16_syscall3(a0, a1, a2, number)				\
	__mips16_syscall3 ((long) (a0), (long) (a1), (long) (a2),	\
			   (long) (number))

long long __nomips16 __mips16_syscall4 (long a0, long a1, long a2, long a3,
					long number);
#define __mips16_syscall4(a0, a1, a2, a3, number)			\
	__mips16_syscall4 ((long) (a0), (long) (a1), (long) (a2),	\
			   (long) (a3),					\
			   (long) (number))

/* The remaining ones use regular MIPS wrappers.  */

#define __mips16_syscall5(a0, a1, a2, a3, a4, number)			\
	__mips_syscall5 ((long) (a0), (long) (a1), (long) (a2),		\
			 (long) (a3), (long) (a4),			\
			 (long) (number))

#define __mips16_syscall6(a0, a1, a2, a3, a4, a5, number)		\
	__mips_syscall6 ((long) (a0), (long) (a1), (long) (a2),		\
			 (long) (a3), (long) (a4), (long) (a5),		\
			 (long) (number))

#define __mips16_syscall7(a0, a1, a2, a3, a4, a5, a6, number)		\
	__mips_syscall7 ((long) (a0), (long) (a1), (long) (a2),		\
			 (long) (a3), (long) (a4), (long) (a5),		\
			 (long) (a6),					\
			 (long) (number))

#endif
