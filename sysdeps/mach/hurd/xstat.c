/* Copyright (C) 1992-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sys/stat.h>

#include "xstatconv.c"

/* Get file information about FILE in BUF.  */
int
__xstat (int vers, const char *file, struct stat *buf)
{
  struct stat64 buf64;
  return __xstat64 (vers, file, &buf64) ?: xstat64_conv (buf, &buf64);
}
hidden_def (__xstat)
weak_alias (__xstat, _xstat)
