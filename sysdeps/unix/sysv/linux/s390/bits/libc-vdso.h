/* Resolve function pointers to VDSO functions.
   Copyright (C) 2008 Free Software Foundation, Inc.
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

extern long int (*__vdso_gettimeofday) (struct timeval *, void *)
  attribute_hidden;

extern long int (*__vdso_clock_gettime) (clockid_t, struct timespec *);

extern long int (*__vdso_clock_getres) (clockid_t, struct timespec *);

#endif

#endif /* _LIBC_VDSO_H */
