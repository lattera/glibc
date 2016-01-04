/* Copyright (C) 2012-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>

	.text
ENTRY(pthread_spin_lock)
1:	ldstub		[%o0], %g1
	brnz,pn		%g1, 2f
	 membar		#StoreLoad | #StoreStore
	retl
	 mov		0, %o0
2:	ldub		[%o0], %g1
	brnz,pt		%g1, 2b
	 membar		#LoadLoad
	ba,a,pt		%xcc, 1b
END(pthread_spin_lock)
