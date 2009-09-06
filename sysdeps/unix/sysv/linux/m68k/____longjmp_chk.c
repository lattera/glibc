/* Copyright (C) 2009 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <stdio.h>
#include <signal.h>
#include <sysdep.h>
#define __longjmp ____longjmp_chk
#define CHECK_SP(sp)							      \
  do {									      \
    register unsigned long this_sp asm ("sp");				      \
    if ((unsigned long) (sp) < this_sp)					      \
      {									      \
	struct sigaltstack oss;						      \
	INTERNAL_SYSCALL_DECL (err);					      \
	int result = INTERNAL_SYSCALL (sigaltstack, err, 2, NULL, &oss);      \
	if (!INTERNAL_SYSCALL_ERROR_P (result, err)			      \
	    && ((oss.ss_flags & SS_ONSTACK) == 0			      \
		|| ((unsigned long) oss.ss_sp + oss.ss_size		      \
		    - (unsigned long) (sp)) < oss.ss_size))		      \
	  __fortify_fail ("longjmp causes uninitialized stack frame");	      \
      }									      \
  } while (0)

#include <__longjmp.c>
