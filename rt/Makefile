# Copyright (C) 1997, 1998, 1999, 2000, 2001 Free Software Foundation, Inc.
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
#	Sub-makefile for real-time portion of the library.
#
subdir	:= rt

headers	:= aio.h

librt-routines := aio_cancel aio_error aio_fsync aio_misc aio_read	\
		  aio_read64 aio_return aio_suspend aio_write		\
		  aio_write64 lio_listio lio_listio64 aio_sigqueue	\
		  aio_notify get_clockfreq clock_getcpuclockid		\
		  clock_getres clock_gettime clock_settime		\
		  clock_nanosleep					\
		  timer_create timer_delete timer_getoverr		\
		  timer_gettime timer_settime				\
		  shm_open shm_unlink

# Make sure that everything is distributed independent of configuration.
distribute := aio_misc.h $(librt-routines:=.c) $(librt-map)

include ../Makeconfig

ifeq ($(have-thread-library),yes)

tests := tst-aio tst-aio64 tst-clock tst-shm tst-timer tst-aio2 tst-aio3 \
	 tst-aio4 tst-aio5 tst-aio6

extra-libs := librt
extra-libs-others := $(extra-libs)

endif

include ../Rules

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)librt.so: $(common-objpfx)libc.so $(common-objpfx)libc_nonshared.a \
		   $(shared-thread-library)

ifeq (yes,$(build-shared))
$(addprefix $(objpfx),$(tests)): $(objpfx)librt.so $(shared-thread-library)
else
$(addprefix $(objpfx),$(tests)): $(objpfx)librt.a $(static-thread-library)
endif
ifeq (yes,$(build-bounded))
$(tests:%=$(objpfx)%-bp): $(objpfx)librt_b.a $(bounded-thread-library)
endif
