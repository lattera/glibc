/* Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2002.

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

#include <sysdep.h>

#define ARCH_FORK() \
({									\
  register long __o0 __asm__ ("o0");					\
  register long __o1 __asm__ ("o1");					\
  register long __g1 __asm__ ("g1") = __NR_fork;			\
  __asm __volatile (__SYSCALL_STRING					\
		    : "=r" (__g1), "=r" (__o0), "=r" (__o1)		\
		    : "0" (__g1)					\
		    : __SYSCALL_CLOBBERS);				\
  __o0 == -1 ? __o0 : (__o0 & (__o1 - 1));				\
})

#include_next <fork.h>
