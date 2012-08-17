# Copyright (C) 2002-2012 Free Software Foundation, Inc.
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
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

ifeq ($(subdir),nptl)
sysdep_routines += register-atfork unregister-atfork libc_pthread_init \
		   libc_multiple_threads

libpthread-sysdep_routines += pt-fork pthread_mutex_cond_lock

gen-as-const-headers += lowlevelcond.sym lowlevelrwlock.sym \
			lowlevelbarrier.sym unwindbuf.sym \
			lowlevelrobustlock.sym pthread-pi-defines.sym \
			structsem.sym
endif

ifeq ($(subdir),posix)
CFLAGS-fork.c = $(libio-mtsafe)
CFLAGS-getpid.o = -fomit-frame-pointer
CFLAGS-getpid.os = -fomit-frame-pointer
endif

# Needed in both the signal and nptl subdir.
CFLAGS-sigaction.c = -DWRAPPER_INCLUDE='<nptl/sigaction.c>'
