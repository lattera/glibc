/* Copyright (C) 1997 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <sys/param.h>

/* Return the system page size.  This value will either be 4k or 8k depending
   on whether or not we are running on Sparc v9 machine.  */

/* If we are not a static program, this value is collected from the system
   via the AT_PAGESZ auxiliary argument.  If we are a static program, we
   use the getpagesize system call.  */

size_t _dl_pagesize;

extern size_t __syscall_getpagesize(void);

int
__getpagesize ()
{
  if (_dl_pagesize == 0)
    _dl_pagesize = __syscall_getpagesize();
  return _dl_pagesize;
}

weak_alias (__getpagesize, getpagesize)
