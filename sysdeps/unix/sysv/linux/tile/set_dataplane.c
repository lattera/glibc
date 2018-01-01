/* Copyright (C) 2011-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

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

#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <shlib-compat.h>

/* Request dataplane modes from the kernel (compatibility only). */
#if SHLIB_COMPAT (libc, GLIBC_2_12, GLIBC_2_25)
int
attribute_compat_text_section
__old_set_dataplane (int flags)
{
#ifdef __NR_set_dataplane
  return INLINE_SYSCALL (set_dataplane, 1, flags);
#else
  __set_errno (ENOSYS);
  return -1;
#endif
}

compat_symbol (libc, __old_set_dataplane, set_dataplane, GLIBC_2_12);
#endif
