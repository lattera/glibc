/* Copyright (C) 1991, 1992, 1995 Free Software Foundation, Inc.
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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <sysdep.h>

/* Linux has two versions of the select system call.  The old one expected
   one argument which must be a pointer to a struct which contains the
   five values.  The new version expects the five arguments be given in the
   registers.  Of course we use the new version but the syscall name
   is `_newsyscall'.  So we do some name magic here.  */
#define ___newselect __select

SYSCALL__ (_newselect, 5)
	ret

weak_alias (__select, select)
