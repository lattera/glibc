/* Copyright (C) 1991, 1994, 1996, 1997, 1998 Free Software Foundation, Inc.
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

/* This file defines the `errno' constants for standalone ARM machines.
   These constants are essentially arbitrary.  */

#if !defined __Emath_defined && (defined _ERRNO_H || defined __need_Emath)
# undef	__need_Emath
# define __Emath_defined	1

# define EDOM		1
# define ERANGE		2
#endif

#ifdef	_ERRNO_H
# define ENOSYS		3
# define EINVAL		4
# define ESPIPE		5
# define EBADF		6
# define ENOMEM		7
# define EACCES		8
# define ENFILE		9
# define EMFILE		10
# define ENAMETOOLONG	11	/* File name too long */
# define ELOOP		12	/* Too many symbolic links encountered */
# define ENOMSG		13      /* No message of desired type */
# define E2BIG		14	/* Arg list too long */
# define EINTR		15
# define EILSEQ		16
# define ENOEXEC	17
# define ENOENT		18
# define EPROTOTYPE	19
# define ESRCH		20
# define EPERM		21
# define ENOTDIR	22
# define ESTALE		23
# define EISDIR		24
# define EOPNOTSUPP	25	/* Operation not supported.  */
# define ENOTTY		26
# define EAGAIN		27
# define EIO		28
# define ENOSPC		29
# define EEXIST		30
# define EBUSY		31
# define EOVERFLOW	32
#endif

#define __set_errno(val) errno = (val)

/* Function to get address of global `errno' variable.  */
extern int *__errno_location __P ((void)) __attribute__ ((__const__));
