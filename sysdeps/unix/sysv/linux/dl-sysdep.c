/* Dynamic linker system dependencies for Linux.
Copyright (C) 1995 Free Software Foundation, Inc.
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

/* Linux needs some special initialization, but otherwise uses
   the generic dynamic linker system interface code.  */

#include <unistd.h>

#define DL_SYSDEP_INIT frob_brk ()

static inline void
frob_brk (void)
{
  extern void _end;
  __brk (0);			/* Initialize the break.  */
  if (__sbrk (0) == &_end)
    {
      /* The dynamic linker was run as a program, and so the initial break
	 starts just after our bss, at &_end.  The malloc in dl-minimal.c
	 will consume the rest of this page, so tell the kernel to move the
	 break up that far.  When the user program examines its break, it
	 will see this new value and not clobber our data.  */
      size_t pg = __getpagesize ();
      __sbrk (pg - ((&_end - (void *) 0) & pg));
    }
}

#include <sysdeps/generic/dl-sysdep.c>
