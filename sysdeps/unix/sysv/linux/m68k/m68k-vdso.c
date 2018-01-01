/* Copyright (C) 2010-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Maxim Kuvyrkov <maxim@codesourcery.com>, 2010.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifdef SHARED

#include <m68k-vdso.h>

/* Because these pointers are used from other libraries than libc,
   they are exported at GLIBC_PRIVATE version.
   We initialize them to syscall implementation so that they will be ready
   to use from the very beginning.  */
void * M68K_VDSO_SYMBOL (__vdso_read_tp)
= (void *) __vdso_read_tp_stub;
void * M68K_VDSO_SYMBOL (__vdso_atomic_cmpxchg_32)
= (void *) __vdso_atomic_cmpxchg_32_stub;
void * M68K_VDSO_SYMBOL (__vdso_atomic_barrier)
= (void *) __vdso_atomic_barrier_stub;

#endif /* SHARED */
