/* Copyright (C) 1998, 1999, 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _SYS_SENDFILE_H
#define _SYS_SENDFILE_H	1

#include <features.h>
#include <sys/types.h>

#ifdef __USE_FILE_OFFSET64
# error "<sys/sendfile.h> cannot be used with _FILE_OFFSET_BITS=64"
#endif

__BEGIN_DECLS

/* Send COUNT bytes from file associated with IN_FD starting at OFFSET to
   descriptor OUT_FD.  */
extern ssize_t sendfile (int __out_fd, int __in_fd, off_t *offset,
			 size_t __count) __THROW;

__END_DECLS

#endif	/* sys/sendfile.h */
