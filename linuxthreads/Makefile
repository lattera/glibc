# Copyright (C) 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

#
#	Sub-makefile for linuxthreads portion of the library.
#
subdir	:= linuxthreads

linuxthreads-version := $(shell sed -n 's/^.*$(subdir)-\([0-9.]*\).*$$/\1/p' \
				    Banner)

headers := pthread.h semaphore.h
distribute := internals.h queue.h restart.h spinlock.h

routines := weaks no-tsd

extra-libs := libpthread
extra-libs-others := $(extra-libs)

libpthread-routines := attr cancel condvar join manager mutex ptfork \
		       ptlongjmp pthread signals specific errno lockfile \
		       semaphore spinlock wrapsyscall rwlock pt-machine \
		       oldsemaphore events

vpath %.c Examples
tests = ex1 ex2 ex3 ex4 ex5 ex6

include ../Rules

CFLAGS-mutex.c += -D__NO_WEAK_PTHREAD_ALIASES
CFLAGS-specific.c += -D__NO_WEAK_PTHREAD_ALIASES
CFLAGS-pthread.c += -D__NO_WEAK_PTHREAD_ALIASES
CFLAGS-ptfork.c += -D__NO_WEAK_PTHREAD_ALIASES
CFLAGS-cancel.c += -D__NO_WEAK_PTHREAD_ALIASES

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libpthread.so: $(common-objpfx)libc.so

# Make sure we link with the thread library.
ifeq ($(build-shared),yes)
libpthread = $(objpfx)libpthread.so
else
libpthread = $(objpfx)libpthread.a
endif

$(objpfx)ex1: $(libpthread)
$(objpfx)ex2: $(libpthread)
$(objpfx)ex3: $(libpthread)
$(objpfx)ex4: $(libpthread)
$(objpfx)ex5: $(libpthread)
$(objpfx)ex6: $(libpthread)
