/* Optimized strrchr implementation for POWER8.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

#include <sysdep.h>

#undef ENTRY_TOCLESS
#ifndef PROF
#define ENTRY_TOCLESS(name, ...)				\
	ENTRY_3 __strrchr_power8, ## __VA_ARGS__;		\
	cfi_startproc
#else
#define ENTRY_TOCLESS(name, ...)				\
	ENTRY_3 __strrchr_power8, ## __VA_ARGS__;		\
	cfi_startproc;						\
	LOCALENTRY(__strrchr_power8)
#endif

#undef END
#define END(name)						\
  cfi_endproc;							\
  TRACEBACK(__strrchr_power8);					\
  END_2(__strrchr_power8)

#undef libc_hidden_builtin_def
#define libc_hidden_builtin_def(name)

#include <sysdeps/powerpc/powerpc64/power8/strrchr.S>
