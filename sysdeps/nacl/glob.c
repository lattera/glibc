/* Do glob searching.  NaCl version.
   Copyright (C) 2015 Free Software Foundation, Inc.
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

#include <unistd.h>			/* Declares getlogin_r.  */

/* We do not have getlogin_r in the library at all for NaCl.
   Define it away so the glob code does not try to use it.  */
#define getlogin_r(name, len)		(ENOSYS)

/* Fetch the version that defines glob64 as an alias.  */
#include <sysdeps/wordsize-64/glob.c>
