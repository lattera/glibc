/* BSD-compatible versions of functions where BSD and POSIX.1 conflict.

Copyright (C) 1991, 1992, 1994 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#define	_BSD_SOURCE

#include <ansidecl.h>
#include <sys/types.h>
#include <unistd.h>
#include <gnu-stabs.h>
#include <limits.h>
#include <setjmp.h>

#undef	getpgrp
function_alias(getpgrp, __getpgrp, pid_t, (pid),
	       DEFUN(getpgrp, (pid), pid_t pid))

/* These entry points allow for things compiled for another C library
   that want the BSD-compatible definitions.  (Of course, their jmp_buf
   must be big enough.)  */

#undef	longjmp
#ifdef __STDC__
#define void __NORETURN void
#endif
function_alias_void(longjmp, siglongjmp, (env, val),
		    DEFUN(longjmp, (env, val), CONST jmp_buf env AND int val))

#undef	setjmp
int
DEFUN(setjmp, (env), jmp_buf env)
{
  return sigsetjmp (env, 1);
}
