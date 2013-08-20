/* Resolve function pointers to VDSO functions.
   Copyright (C) 2005-2013 Free Software Foundation, Inc.
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


#ifndef _LIBC_VDSO_H
#define _LIBC_VDSO_H

#ifdef SHARED

extern void *__vdso_gettimeofday attribute_hidden;

extern void *__vdso_clock_gettime;

extern void *__vdso_clock_getres;

extern void *__vdso_get_tbfreq;

extern void *__vdso_getcpu;

extern void *__vdso_time;

#if defined(__PPC64__) || defined(__powerpc64__)
extern void *__vdso_sigtramp_rt64;
#else
extern void *__vdso_sigtramp32;
extern void *__vdso_sigtramp_rt32;
#endif

/* This macro is needed for PPC64 to return a skeleton OPD entry of a vDSO
   symbol.  This works because _dl_vdso_vsym always return the function
   address, and no vDSO symbols use the TOC or chain pointers from the OPD
   so we can allow them to be garbage.  */
#if defined(__PPC64__) || defined(__powerpc64__)
#define VDSO_IFUNC_RET(value)  ((void *) &(value))
#else
#define VDSO_IFUNC_RET(value)  ((void *) (value))
#endif

#endif

#endif /* _LIBC_VDSO_H */
