/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* Default stack size.  */
#define ARCH_STACK_DEFAULT_SIZE	(2 * 1024 * 1024)

/* Required stack pointer alignment at beginning.  SSE requires 16
   bytes.  */
#define STACK_ALIGN		16

/* Minimal stack size after allocating thread descriptor and guard size.  */
#define MINIMAL_REST_STACK	2048

/* Alignment requirement for TCB.  */
#define TCB_ALIGNMENT		16


/* Location of current stack frame.  The frame pointer is not usable.  */
#define CURRENT_STACK_FRAME \
  ({ char *frame; asm ("movq %%rsp, %0" : "=r" (frame)); frame; })


/* We prefer to have the stack allocated in the low 4GB since this
   allows faster context switches.  */
#define ARCH_MAP_FLAGS MAP_32BIT

/* If it is not possible to allocate memory there retry without that
   flag.  */
#define ARCH_RETRY_MMAP(size) \
  mmap (NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC,			      \
	MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)


/* XXX Until we have a better place keep the definitions here.  */

/* While there is no such syscall.  */
#define __exit_thread_inline(val) \
  asm volatile ("syscall" :: "a" (__NR_exit), "D" (val))
