/* Change byte order in 16-bit value.  ia64 version.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Dan Pop <Dan.Pop@cern.ch>

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


#include <sysdep.h>
#undef ret

ENTRY(htons)
	alloc	ret0 = ar.pfs, 1, 0, 0, 0
	shl	in0 = in0, 48
	;;
	mux1	ret0 = in0, @rev
	br.ret.sptk.many b0
END(htons)

weak_alias (htons, ntohs)
