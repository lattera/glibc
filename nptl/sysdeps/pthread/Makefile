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
# 02111-1307 USA.

ifeq ($(subdir),csu)
CFLAGS-libc-start.c += -I../nptl
routines += unwind-resume
shared-only-routines += unwind-resume
CFLAGS-unwind-resume.c += -fexceptions -fasynchronous-unwind-tables
endif

ifeq ($(subdir),nptl)
libpthread-sysdep_routines += errno-loc
endif

ifeq ($(subdir),rt)
librt-sysdep_routines += timer_routines librt-cancellation rt-unwind-resume
librt-shared-only-routines += rt-unwind-resume
CPPFLAGS-timer_routines.c = -I../nptl
CFLAGS-librt-cancellation.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-rt-unwind-resume.c += -fexceptions -fasynchronous-unwind-tables

ifeq (yes,$(build-shared))
$(objpfx)tst-timer: $(objpfx)librt.so $(shared-thread-library)
else
$(objpfx)tst-timer: $(objpfx)librt.a $(static-thread-library)
endif

ifeq ($(have-forced-unwind),yes)
tests += tst-mqueue8x
CFLAGS-tst-mqueue8x.c += -fexceptions
endif
endif

ifeq ($(subdir),posix)
CFLAGS-confstr.c += -DLIBPTHREAD_VERSION='"NPTL $(version)"'
endif
