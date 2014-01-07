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

   This is a copy of debug/longjmp_chk.c extended for symbol
   versioning.  */

#include <shlib-compat.h>
#include <setjmp.h>

/* This place is the only user of these functions.  */
extern void ____v2__longjmp_chk (__jmp_buf __env, int __val)
     __attribute__ ((__noreturn__));

#if defined NOT_IN_libc

# define __v2__longjmp ____longjmp_chk
# define __v2__libc_siglongjmp __longjmp_chk

# include <longjmp.c>

#else

# define __v2__longjmp ____v2__longjmp_chk
# define __v2__libc_siglongjmp __v2__libc_siglongjmp_chk

# include <longjmp.c>

versioned_symbol (libc, __v2__libc_siglongjmp_chk, __longjmp_chk, GLIBC_2_19);

#endif
