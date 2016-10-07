/* time -- Get number of seconds since Epoch.  Linux/i386 version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#ifdef SHARED
# define time __redirect_time
#endif

#include <time.h>

#ifdef SHARED
# undef time
# define time_type __redirect_time

# undef libc_hidden_def
# define libc_hidden_def(name)  \
  __hidden_ver1 (__time_syscall, __GI_time, __time_syscall);
#endif

#include <sysdeps/unix/sysv/linux/x86/time.c>
