/* libc-internal interface for thread-specific data.
   Copyright (C) 1998,99,2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sys/cdefs.h>  /* for __const */
#include <bits/libc-tsd.h>

#if !(USE_TLS && HAVE___THREAD)

/* This file provides uinitialized (common) definitions for the
   hooks used internally by libc to access thread-specific data.

   When -lpthread is used, it provides initialized definitions for these
   variables (in specific.c), which override these uninitialized definitions.

   If -lpthread is not used, these uninitialized variables default to zero,
   which the __libc_tsd_* macros check for.   */

void *(*__libc_internal_tsd_get) (enum __libc_tsd_key_t);
int (*__libc_internal_tsd_set) (enum __libc_tsd_key_t,
				__const void *);
void **(*__libc_internal_tsd_address) (enum __libc_tsd_key_t)
     __THROW __attribute__ ((__const__));

#endif /* !(USE_TLS && HAVE___THREAD) */

int __libc_alloca_cutoff (size_t size)
{
  return size <= __MAX_ALLOCA_CUTOFF;
}
