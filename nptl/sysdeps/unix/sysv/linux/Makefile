# Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
# Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.  */

ifeq ($(subdir),nptl)
sysdep_routines += register-atfork unregister-atfork libc_pthread_init \
		   libc_multiple_threads

libpthread-sysdep_routines += pt-fork pthread_mutex_cond_lock

gen-as-const-headers += lowlevelcond.sym lowlevelrwlock.sym \
			lowlevelbarrier.sym unwindbuf.sym
endif

ifeq ($(subdir),posix)
CFLAGS-fork.c = -D_IO_MTSAFE_IO
CFLAGS-getpid.o = -fomit-frame-pointer
CFLAGS-getpid.os = -fomit-frame-pointer
endif
