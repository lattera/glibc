/* Copyright (C) 2008 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include_next <aio_misc.h>

#ifdef __thumb2__

#include <errno.h>

/* The Thumb-2 definition of INTERNAL_SYSCALL_RAW has to hide the use
   of r7 from the compiler because it cannot handle asm clobbering the
   hard frame pointer.  In aio_suspend, GCC does not eliminate the
   hard frame pointer because the function uses variable-length
   arrays, so it generates unwind information using r7 as virtual
   stack pointer.  During system calls, when r7 has been saved on the
   stack, this means the unwind information is invalid.  Without extra
   unwind directives, which would need to cause unwind information for
   the asm to be generated separately from that for the parts of the
   function before and after the asm (with three index table entries),
   it is not possible to represent any temporary change to the virtual
   stack pointer.  Instead, we move the problematic system calls out
   of line into a function that does not require a frame pointer.  */

static __attribute_noinline__ void
aio_misc_wait (int *resultp,
	       volatile int *futexp,
	       const struct timespec *timeout,
	       int cancel)
{
  AIO_MISC_WAIT (*resultp, *futexp, timeout, cancel);
}

#undef AIO_MISC_WAIT
#define AIO_MISC_WAIT(result, futex, timeout, cancel)	\
  aio_misc_wait (&result, &futex, timeout, cancel)

#endif
