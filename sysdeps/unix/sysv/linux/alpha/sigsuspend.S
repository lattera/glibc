/* Copyright (C) 1993-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger <davidm@cs.arizona.edu>, 1995.

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

/* sigsuspend is a special syscall since it needs to dereference the
   sigset.  This will have to change when we have more than 64 signals.  */

#include <sysdep-cancel.h>

#undef PSEUDO_PREPARE_ARGS
#define PSEUDO_PREPARE_ARGS	ldq	a0, 0(a0);

PSEUDO(__sigsuspend, sigsuspend, 1)
	ret
PSEUDO_END(__sigsuspend)
libc_hidden_def (__sigsuspend)
weak_alias (__sigsuspend, sigsuspend)
strong_alias (__sigsuspend, __libc_sigsuspend)
