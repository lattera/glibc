/* Machine-dependent definitions for profiling support.  Generic GCC 2 version.
Copyright (C) 1996 Free Software Foundation, Inc.
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

/* GCC version 2 gives us a perfect magical function to get
   just the information we need:
     void *__builtin_return_address (unsigned int N)
   returns the return address of the frame N frames up.  */

#if __GNUC__ < 2
 #error "This file uses __builtin_return_address, a GCC 2 extension."
#endif

#include <sysdep.h>
#ifndef NO_UNDERSCORES
/* The asm symbols for C functions are `_function'.
   The canonical name for the counter function is `mcount', no _.  */
void _mcount (void) asm ("mcount");
#endif

#define _MCOUNT_DECL(frompc, selfpc) \
static inline void mcount_internal (frompc, selfpc)

#define MCOUNT \
void _mcount (void)							      \
{									      \
  mcount_internal ((u_long) __builtin_return_address (0),		      \
		   (u_long) __builtin_return_address (1));		      \
}
