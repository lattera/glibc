/* Copyright (C) 1992, 1997,1998,2000,2004,2009 Free Software Foundation, Inc.
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

#include <dirent.h>
#include <string.h>

int
__alphasort64 (const struct dirent64 **a, const struct dirent64 **b)
{
  return strcoll ((*a)->d_name, (*b)->d_name);
}

#include <shlib-compat.h>

versioned_symbol (libc, __alphasort64, alphasort64, GLIBC_2_2);

#if SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)

#include <sysdeps/unix/sysv/linux/i386/olddirent.h>

int
__old_alphasort64 (const struct __old_dirent64 **a,
		   const struct __old_dirent64 **b);

int
attribute_compat_text_section
__old_alphasort64 (const struct __old_dirent64 **a,
		   const struct __old_dirent64 **b)
{
  return strcoll ((*a)->d_name, (*b)->d_name);
}

compat_symbol (libc, __old_alphasort64, alphasort64, GLIBC_2_1);
#endif
