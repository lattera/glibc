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

#include <sys/stat.h>

/* In Linux the `stat' call is actually done by emulating a `xstat' system
   call, which takes an additional first argument giving a version number
   for `struct stat'.  Likewise for `fstat' and `lstat' there are `fxstat'
   and `lxstat' emulations.  */

int
__fstat (int fd, struct stat *buf)
{
  return __fxstat (_STAT_VER, fd, buf);
}

weak_alias (__fstat, fstat)
