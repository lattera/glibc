/* Bounded-pointer syscall thunk support.
   Copyright (C) 2000, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Greg McGary <greg@mcgary.org>

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

#ifndef _bpthunks_h_
#define _bpthunks_h_

#ifndef __ASSEMBLER__

/* This header is included by the syscall BP thunks defined in
   sysd-syscalls, as created by sysdeps/unix/make-syscalls.sh.  It
   includes all headers that contain prototype declarations for system
   call functions.  */

#include <libc-symbols.h>
#include <bp-sym.h>
#include <bp-checks.h>

/* Get `struct timeval' definition for select.  */
#define __need_timeval
#include <bits/time.h>

#include <stddef.h>
#include <unistd.h>
#include <sched.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <utime.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/klog.h>
#include <sys/mman.h>
#include <sys/mount.h>
#include <sys/quota.h>
#include <sys/resource.h>
#include <sys/select.h>
#include <io/sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/swap.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/timex.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#endif /* Not __ASSEMBLER__.  */

#endif /* _bpthunks_h_ */
