/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger <davidm@hpl.hp.com>

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* __pipe is a special syscall since it returns two values.  */

#include <sysdep.h>

ENTRY(__pipe)
       st8 [sp]=r32		// save ptr across system call
       DO_CALL (SYS_ify (pipe))
       ld8 r2=[sp]
       cmp.ne p6,p0=-1,r10
       ;;
(p6)   st4 [r2]=r8,4
       mov ret0=0
       ;;
(p6)   st4 [r2]=r9
(p6)   ret
       br.cond.spnt.few __syscall_error
PSEUDO_END(__pipe)

weak_alias (__pipe, pipe)
