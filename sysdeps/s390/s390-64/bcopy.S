/* bcopy -- copy a block from source to destination.  64 bit S/390 version.
   This file is part of the GNU C Library.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).

   The GNU C Library is free software  ; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation   ; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY            ; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* INPUT PARAMETERS
     %r2 = address of source
     %r3 = address of destination
     %r4 = number of bytes to copy.  */

#include "sysdep.h"
#include "asm-syntax.h"

        .text
ENTRY(__bcopy)
        clgr    %r2,%r3             # check against destructive overlap
        jnl     .L0
        lgr     %r1,%r2
        algr    %r1,%r4
        clgr    %r1,%r3
        jh      .L2
.L0:
	lgr     %r5,%r4		    # source length
        lgr     %r4,%r2             # source address
        sgr     %r1,%r1             # set pad byte to zero
	lgr     %r2,%r3             # set destination
        lgr     %r3,%r5             # destination length = source length
.L1:    mvcle   %r2,%r4,0(%r1)      # thats it, MVCLE is your friend
        jo      .L1
        br      %r14
.L2:                                # destructive overlay, can not use mvcle
        lgr    %r1,%r2              # bcopy is called with source,dest
        lgr    %r2,%r3              # memmove with dest,source! Oh, well...
        lgr    %r3,%r1
#ifdef PIC
	jg     memmove@PLT
#else
	jg     memmove
#endif

END(__bcopy)

#ifndef NO_WEAK_ALIAS
weak_alias (__bcopy, bcopy)
#endif

