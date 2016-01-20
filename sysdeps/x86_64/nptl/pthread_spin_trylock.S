/* Copyright (C) 2002-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <pthread-errnos.h>
#include <sysdep.h>


#ifdef UP
# define LOCK
#else
# define LOCK lock
#endif

ENTRY(pthread_spin_trylock)
	movl	$1, %eax
	xorl	%ecx, %ecx
	LOCK
	cmpxchgl %ecx, (%rdi)
	movl	$EBUSY, %eax
	cmovel	%ecx, %eax
	retq
END(pthread_spin_trylock)
