/* Convert between the kernel's `struct stat' format, and libc's.
   Copyright (C) 2004, 2006 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <kernel-features.h>
#include <sys/syscall.h>

#ifdef __NR_stat64
# if __ASSUME_STAT64_SYSCALL == 0
extern int __libc_missing_axp_stat64 attribute_hidden;
# endif
#endif
extern int __xstat_conv (int vers, struct kernel_stat *kbuf, void *ubuf)
  attribute_hidden;
