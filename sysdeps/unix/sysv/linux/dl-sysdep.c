/* Dynamic linker system dependencies for Linux.
   Copyright (C) 1995, 1997, 2001, 2004 Free Software Foundation, Inc.
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

/* Linux needs some special initialization, but otherwise uses
   the generic dynamic linker system interface code.  */

#include <unistd.h>
#include <ldsodefs.h>
#include "kernel-features.h"

#define DL_SYSDEP_INIT frob_brk ()

static inline void
frob_brk (void)
{
  __brk (0);			/* Initialize the break.  */

#if ! __ASSUME_BRK_PAGE_ROUNDED
  /* If the dynamic linker was executed as a program, then the break may
     start immediately after our data segment.  However, dl-minimal.c has
     already stolen the remainder of the page for internal allocations.
     If we don't adjust the break location recorded by the kernel, the
     normal program startup will inquire, find the value at our &_end,
     and start allocating its own data there, clobbering dynamic linker
     data structures allocated there during startup.

     Later Linux kernels have changed this behavior so that the initial
     break value is rounded up to the page boundary before we start.  */

  extern void *__curbrk attribute_hidden;
  extern void _end attribute_hidden;
  void *const endpage = (void *) 0 + (((__curbrk - (void *) 0)
				       + GLRO(dl_pagesize) - 1)
				      & -GLRO(dl_pagesize));
  if (__builtin_expect (__curbrk >= &_end && __curbrk < endpage, 0))
    __brk (endpage);
#endif
}

#include <sysdeps/generic/dl-sysdep.c>
