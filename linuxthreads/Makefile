# Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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

linuxthreads-version=0.7

headers := pthread.h semaphore.h bits/semaphore.h
distribute := internals.h queue.h restart.h spinlock.h

routines := weaks

extra-libs := libpthread
extra-libs-others := $(extra-libs)

libpthread-routines := attr cancel condvar join manager mutex ptfork \
		       ptlongjmp pthread signals specific errno lockfile \
		       semaphore spinlock wrapsyscall rwlock
libpthread-map	:= libpthread.map

include ../Rules

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libpthread.so: $(common-objpfx)libc.so
