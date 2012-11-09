/* Copyright (C) 2009-2012 Free Software Foundation, Inc.

   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _SYS_USER_H
#define _SYS_USER_H	1

/* We need to see the definition of struct pt_regs but do not want the
   linux PTRACE_* defines since they conflict with the generic glibc
   definitions in sys/ptrace.h Hence the undef's below.  */
#include <asm/ptrace.h>

#undef PTRACE_GET_THREAD_AREA
#undef PTRACE_GETHBPREGS
#undef PTRACE_SETHBPREGS

#endif
