/* Copyright (C) 1993, 1997 Free Software Foundation, Inc.
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

#ifndef	_DIRSTREAM_H

#define	_DIRSTREAM_H	1

#define __need_size_t
#include <stddef.h>

/* Directory stream type.

   The miscellaneous Unix `readdir' implementations read directory data
   into a buffer and fill in a `struct dirent' copy in the `DIR' object. */

struct __dirstream
  {
    int __fd;			/* File descriptor.  */

    char *__data;		/* Directory block.  */
    size_t __allocation;	/* Space allocated for the block.  */
    size_t __offset;		/* Current offset into the block.  */
    size_t __size;		/* Total valid data in the block.  */

    struct dirent __entry;	/* Returned by `readdir'.  */
  };

#endif	/* dirstream.h */
