/* Low-level statistical profiling support function.  Mostly POSIX.1 version.
Copyright (C) 1996 Free Software Foundation, Inc.
Contributed by David Mosberger <davidm@azstarnet.com>
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

#include <asm/sigcontext.h>

void
profil_counter (int signal, long a1, long a2, long a3, long a4, long a5,
		struct sigcontext_struct sc)
{
  profil_count((void *) sc.sc_pc);
}
