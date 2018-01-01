/* Syscall wrapper that do not set errno.  Linux version.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

/* This function is used on maybe_enable_malloc_check (elf/dl-tunables.c)
   and to avoid having to build/use multiple versions if stack protection
   in enabled it is defined as inline.  */
static inline int
__access_noerrno (const char *pathname, int mode)
{
  int res;
  INTERNAL_SYSCALL_DECL (err);
#ifdef __NR_access
  res = INTERNAL_SYSCALL_CALL (access, err, pathname, mode);
#else
  res = INTERNAL_SYSCALL_CALL (faccessat, err, AT_FDCWD, pathname, mode);
#endif
  if (INTERNAL_SYSCALL_ERROR_P (res, err))
    return INTERNAL_SYSCALL_ERRNO (res, err);
  return 0;
}
