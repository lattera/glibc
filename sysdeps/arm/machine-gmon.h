/* Machine-dependent definitions for profiling support.  ARM version.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* GCC for the ARM cannot compile __builtin_return_address(N) for N != 0, 
   so we must use an assembly stub.  */

#include <sysdep.h>
#ifndef NO_UNDERSCORES
/* The asm symbols for C functions are `_function'.
   The canonical name for the counter function is `mcount', no _.  */
void _mcount (void) asm ("mcount");
#else
/* The canonical name for the function is `_mcount' in both C and asm,
   but some old asm code might assume it's `mcount'.  */
void _mcount (void);
weak_alias (_mcount, mcount)
#endif

static void mcount_internal (u_long frompc, u_long selfpc);

#define _MCOUNT_DECL(frompc, selfpc) \
static void mcount_internal (u_long frompc, u_long selfpc)

#define MCOUNT \
void _mcount (void)							      \
{									      \
  register unsigned long int frompc, selfpc;				      \
  __asm__("movs fp, fp; "						      \
          "moveq %0, $0; "						      \
	  "ldrne %0, [fp, $-4]; "					      \
	  "ldrne %1, [fp, $-12]; "					      \
	  "movnes %1, %1; "						      \
	  "ldrne %1, [%1, $-4]; "					      \
	  : "=g" (selfpc), "=g" (frompc)				      \
	  : : "cc"							      \
	  );								      \
  if (selfpc)								      \
    mcount_internal(frompc, selfpc);					      \
}
