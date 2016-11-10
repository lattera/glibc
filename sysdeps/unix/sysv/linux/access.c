/* Linux implementation for access function.
   Copyright (C) 2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <fcntl.h>
#include <unistd.h>
#include <sysdep-cancel.h>

int
__access_noerrno (const char *file, int type)
{
  int res;
  INTERNAL_SYSCALL_DECL (err);
#ifdef __NR_access
  res = INTERNAL_SYSCALL_CALL (access, err, file, type);
#else
  res = INTERNAL_SYSCALL_CALL (faccessat, err, AT_FDCWD, file, type);
#endif
  if (INTERNAL_SYSCALL_ERROR_P (res, err))
    return INTERNAL_SYSCALL_ERRNO (res, err);
  return 0;
}

int
__access (const char *file, int type)
{
#ifdef __NR_access
  return INLINE_SYSCALL_CALL (access, file, type);
#else
  return INLINE_SYSCALL_CALL (faccessat, AT_FDCWD, file, type);
#endif
}
weak_alias (__access, access)
