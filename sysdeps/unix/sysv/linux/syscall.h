/* Copyright (C) 1995 Free Software Foundation, Inc.
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

#ifndef _SYSCALL_H
#define _SYSCALL_H

/* This file should list the numbers of the system the system knows.
   But instead of duplicating this we use the information available
   from the kernel sources.  */
#include <asm/unistd.h>

/* Among the system calls defined there are
	_dup2	
	_mkdir	
	_rmdir	
	_readlink	
	_symlink	
	_rename		
	_swapon		
	_access		
	_select		
	_getgroups	
	_setgroups	
	_getitimer	
	_setitimer	
	_setdomainname	
	_getrlimit	
	_setrlimit	
	_getrusage	
	_getpriority	
	_setpriority	
	_truncate	
	_ftruncate	
	_setpgid	
	_fchmod	
	_fchdir	
	_fchown	
	_setregid	
	_setreuid	
	_vhangup	
   (This is a hack for the autoconf mechanism.  Don't change a single
    character, esp white spaces, unless you know what you are doing!)  */

#endif
