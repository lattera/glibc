/* Copyright (C) 1999-2012 Free Software Foundation, Inc.
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

#include <errno.h>
#include <sys/resource.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <shlib-compat.h>
#include <bp-checks.h>

extern int __new_getrlimit (enum __rlimit_resource resource,
			    struct rlimit *__unbounded rlimits);

/* Consider moving to syscalls.list.  */

int
__new_getrlimit (enum __rlimit_resource resource, struct rlimit *rlimits)
{
  return INLINE_SYSCALL (ugetrlimit, 2, resource, CHECK_1 (rlimits));
}

weak_alias (__new_getrlimit, __getrlimit);
versioned_symbol (libc, __new_getrlimit, getrlimit, GLIBC_2_2);
