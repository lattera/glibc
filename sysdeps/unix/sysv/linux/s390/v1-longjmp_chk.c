/* Copyright (C) 2013 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.

   This went into a separate source file since we would otherwise be
   needed to include two different versions of setjmp.h into the same
   file.  */

#include <shlib-compat.h>

#if !defined NOT_IN_libc && defined SHARED
# if SHLIB_COMPAT (libc, GLIBC_2_11, GLIBC_2_19)

#  define __v1__longjmp ____v1__longjmp_chk
#  define __v1__libc_siglongjmp __v1__libc_siglongjmp_chk

#  include <v1-longjmp.c>

compat_symbol (libc, __v1__libc_siglongjmp_chk, __longjmp_chk, GLIBC_2_11);

# endif
#endif
