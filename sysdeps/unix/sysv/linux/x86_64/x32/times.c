/* Linux times.  X32 version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

/* Linux times system call returns 64-bit integer.  */
#undef INTERNAL_SYSCALL_NCS
#define INTERNAL_SYSCALL_NCS(name, err, nr, args...) \
  ({									      \
    unsigned long long int resultvar;					      \
    LOAD_ARGS_##nr (args)						      \
    LOAD_REGS_##nr							      \
    asm volatile (							      \
    "syscall\n\t"							      \
    : "=a" (resultvar)							      \
    : "0" (name) ASM_ARGS_##nr : "memory", REGISTERS_CLOBBERED_BY_SYSCALL);   \
    (long long int) resultvar; })

#undef INTERNAL_SYSCALL_ERROR_P
#define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((unsigned long long int) (val) >= -4095LL)

#include <sysdeps/unix/sysv/linux/times.c>
