/* Machine-specific calling sequence for `mcount' profiling function.  Stub
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

#define _MCOUNT_DECL static inline void _mcount

/* Call _mcount with our the return PC for our caller,
   and the return PC our caller will return to.  */
#define MCOUNT								      \
void mcount (u_long arg)						      \
{									      \
  _mcount (caller return PC, my return PC);				      \
}

#error "sysdeps/MACHINE/machine-gmon.h missing"
