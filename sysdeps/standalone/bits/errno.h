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

/* This file defines the `errno' constants.  */

#if !defined __Emath_defined && (defined _ERRNO_H || defined __need_Emath)
#undef	__need_Emath
#define	__Emath_defined	1

# define EDOM	1
# define EILSEQ 17
# define ERANGE	2
#endif

#ifdef	_ERRNO_H
# define ENOSYS	3
# define EINVAL	4
# define ESPIPE	5
# define EBADF	6
# define ENOMEM	7
# define EACCES	8
# define ENFILE  9
# define EMFILE  10
# define ENOMSG  11
# define ENAMETOOLONG 12
# define ELOOP 13
# define E2BIG 15
# define EINTR 16
# define ENOEXEC 18
# define ENOENT 19
# define EPROTOTYPE 20
# define ESRCH 21
# define EPERM 22
# define EEXIST 23
# define ENOTDIR 24
# define ESTALE 25
# define ENOTTY 26
# define EISDIR 27
# define EOPNOTSUPP 28
# define EAGAIN 29
# define EIO 30
# define ENOSPC 31
# define EBUSY 32
#endif

#define __set_errno(val) errno = (val)
