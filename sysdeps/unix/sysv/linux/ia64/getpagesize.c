/* Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.
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

#include <assert.h>
#include <unistd.h>
#include <sys/param.h>

#include <sysdep.h>
#include <sys/syscall.h>

/* Return the system page size.  The return value will depend on how
   the kernel is configured.  A program must use this call to
   determine the page size to ensure proper alignment for calls such
   as mmap and friends.  --davidm 99/11/30 */

extern size_t _dl_pagesize;

int
__getpagesize ()
{
  assert (_dl_pagesize != 0);
  return _dl_pagesize;
}

weak_alias (__getpagesize, getpagesize)
