/* Extendible version of getcontext for System z
   Copyright (C) 2013 Free Software Foundation, Inc.
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

versioned_symbol (libc, __v2getcontext, getcontext, GLIBC_2_19)
#define __getcontext __v2getcontext

#include "getcontext-common.S"

#undef __getcontext

libc_hidden_ver (__v2getcontext, getcontext)

#if defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_1, GLIBC_2_19)
# define __V1_UCONTEXT
compat_symbol (libc, __v1getcontext, getcontext, GLIBC_2_1)
# define __getcontext __v1getcontext
# include "getcontext-common.S"
# undef __getcontext

#endif
