/* Copyright 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>.
   Based on ../i386/sysdep.h.

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

#include <sysdeps/unix/sysdep.h>
#include "../../am33/sysdep.h"

#ifdef	__ASSEMBLER__

#define	r0		d0	/* Normal return-value register.  */
#define	r1		!!!!	/* Secondary return-value register.  */
#define scratch 	d1	/* Call-clobbered register for random use.  */
#define MOVE(x,y)	mov x, y

#define ret		ret [],0

#endif	/* __ASSEMBLER__ */
