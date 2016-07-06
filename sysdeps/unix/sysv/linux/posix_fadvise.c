/* Copyright (C) 2003-2016 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <sysdep.h>

/* Advice the system about the expected behaviour of the application with
   respect to the file associated with FD.  */

#ifndef __OFF_T_MATCHES_OFF64_T

/* Both arm and powerpc implements fadvise64_64 with last 'advise' argument
   just after 'fd' to avoid the requirement of implementing 7-arg syscalls.
   ARM also defines __NR_fadvise64_64 as __NR_arm_fadvise64_64.

   tile requires __ASSUME_ALIGNED_REGISTER_PAIRS but implements the 32-bit
   fadvise64_64 without the padding 0 after fd.

   s390 implements fadvice64_64 using a specific struct with arguments
   packed inside.  This is the only implementation handled in arch-specific
   code.  */

int
posix_fadvise (int fd, off_t offset, off_t len, int advise)
{
  INTERNAL_SYSCALL_DECL (err);
# ifdef __NR_fadvise64
  int ret = INTERNAL_SYSCALL_CALL (fadvise64, err, fd,
				   __ALIGNMENT_ARG SYSCALL_LL (offset),
				   len, advise);
# else
#  ifdef __ASSUME_FADVISE64_64_6ARG
  int ret = INTERNAL_SYSCALL_CALL (fadvise64_64, err, fd, advise,
				   __ALIGNMENT_ARG SYSCALL_LL (offset),
				   SYSCALL_LL (len));
#  else

#   ifdef __ASSUME_FADVISE64_64_NO_ALIGN
#    undef __ALIGNMENT_ARG
#    define __ALIGNMENT_ARG
#   endif

  int ret = INTERNAL_SYSCALL_CALL (fadvise64_64, err, fd,
				   __ALIGNMENT_ARG SYSCALL_LL (offset),
				   SYSCALL_LL (len), advise);
#  endif
# endif
  if (INTERNAL_SYSCALL_ERROR_P (ret, err))
    return INTERNAL_SYSCALL_ERRNO (ret, err);
  return 0;
}
#endif /* __OFF_T_MATCHES_OFF64_T  */
