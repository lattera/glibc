/* Copyright (C) 1991 Free Software Foundation, Inc.
   Contributed by Joel Sherrill (jsherril@redstone-emh2.army.mil),
     On-Line Applications Research Corporation.

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

#include <ansidecl.h>
#include <unistd.h>
#include <stdlib.h>

/* The function `_exit' should take a status argument and simply
   terminate program execution, using the low-order 8 bits of the
   given integer as status.  */

/* This returns control to Nindy.  */

__NORETURN void
DEFUN(_exit, (status), int status)
{
  /* status is ignored */

  asm volatile( "mov   0,g0; \
                 fmark ; \
           syncf ; \
           .word    0xfeedface ; \
                 bx       start" : : );
 /*  The constant 0xfeedface is a magic word for break which
  *  is defined by NINDY.  The branch extended restarts the
  *  application if the user types "go".
  */
}


#ifdef	 HAVE_GNU_LD

#include <gnu-stabs.h>

stub_warning(_exit);

#endif	/* GNU stabs.  */
