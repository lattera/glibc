/* Copyright (C) 2003-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

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
#include <tls.h>
#include <nptl/pthreadP.h>

#if IS_IN (libc) || IS_IN (libpthread) || IS_IN (librt)

# define SINGLE_THREAD_P \
  __glibc_likely (THREAD_GETMEM (THREAD_SELF, header.multiple_threads) == 0)

#else

# define SINGLE_THREAD_P (1)
# define NO_CANCELLATION 1

#endif

#define RTLD_SINGLE_THREAD_P \
  __glibc_likely (THREAD_GETMEM (THREAD_SELF, header.multiple_threads) == 0)
