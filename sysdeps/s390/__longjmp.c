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
   <http://www.gnu.org/licenses/>.  */

#include <libc-symbols.h>
#include <shlib-compat.h>

#define __longjmp  __v2__longjmp
#include "__longjmp-common.c"
#undef __longjmp
strong_alias (__v2__longjmp, __longjmp)

#if defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_19)
# undef __longjmp
# define __V1_JMPBUF
# define __longjmp  __v1__longjmp
# include "__longjmp-common.c"
#endif /* if defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_19) */
