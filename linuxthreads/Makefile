# Copyright (C) 1996,1997,1998,1999,2000,2001 Free Software Foundation, Inc.
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
		       oldsemaphore events getcpuclockid pspinlock barrier

nodelete-yes = -Wl,--enable-new-dtags,-z,nodelete
initfirst-yes = -Wl,--enable-new-dtags,-z,initfirst
LDFLAGS-pthread.so = $(nodelete-$(have-z-nodelete)) \
		     $(initfirst-$(have-z-initfirst))

vpath %.c Examples

include ../Makeconfig

ifeq ($(build-shared),yes)
before-compile := $(objpfx)crti.o
extra-objs = crti.o

CFLAGS-pt-initfini.s = -g0 -fPIC -fno-inline-functions
endif

librt-tests = ex10 ex11
tests = ex1 ex2 ex3 ex4 ex5 ex6 ex7 ex8 ex9 $(librt-tests) ex12 ex13 joinrace \
	tststack $(tests-nodelete-$(have-z-nodelete)) ecmutex ex14 ex15 ex16

ifeq (yes,$(build-shared))
tests-nodelete-yes = unload
endif

include ../Rules

extra-B-pthread.so = -B$(common-objpfx)linuxthreads/

znodelete-yes = -DHAVE_Z_NODELETE
CFLAGS-mutex.c += -D__NO_WEAK_PTHREAD_ALIASES
CFLAGS-specific.c += -D__NO_WEAK_PTHREAD_ALIASES
CFLAGS-pthread.c += -D__NO_WEAK_PTHREAD_ALIASES $(znodelete-$(have-z-nodelete))
CFLAGS-ptfork.c += -D__NO_WEAK_PTHREAD_ALIASES
CFLAGS-cancel.c += -D__NO_WEAK_PTHREAD_ALIASES
CFLAGS-unload.c += -DPREFIX=\"$(objpfx)\"

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libpthread.so: $(common-objpfx)libc.so \
			$(common-objpfx)libc_nonshared.a

# Make sure we link with the thread library.
ifeq ($(build-shared),yes)
$(addprefix $(objpfx),$(tests)): $(objpfx)libpthread.so
$(addprefix $(objpfx),$(librt-tests)): $(common-objpfx)rt/librt.so
$(objpfx)unload: $(common-objpfx)dlfcn/libdl.so
else
$(addprefix $(objpfx),$(tests)): $(objpfx)libpthread.a
$(addprefix $(objpfx),$(librt-tests)): $(common-objpfx)rt/librt.a
endif
ifeq ($(build-bounded),yes)
$(tests:%=$(objpfx)%-bp): $(objpfx)libpthread_b.a
$(librt-tests:%=$(objpfx)%-bp): $(common-objpfx)rt/librt_b.a
endif

ifeq ($(build-shared),yes)
vpath pt-initfini.c $(full_config_sysdirs)

$(objpfx)pt-initfini.s: pt-initfini.c
	$(compile.c) -S $(CFLAGS-pt-initfini.s) -finhibit-size-directive \
		$(patsubst -f%,-fno-%,$(exceptions)) -o $@

# We only have one kind of startup code files.  Static binaries and
# shared libraries are build using the PIC version.
$(objpfx)crti.S: $(objpfx)pt-initfini.s
	sed -n -e '1,/@HEADER_ENDS/p' \
	       -e '/@_.*_PROLOG_BEGINS/,/@_.*_PROLOG_ENDS/p' \
	       -e '/@TRAILER_BEGINS/,$$p' $< > $@

$(objpfx)defs.h: $(objpfx)pt-initfini.s
	sed -n -e '/@TESTS_BEGIN/,/@TESTS_END/p' $< | \
		$(AWK) -f ../csu/defs.awk > $@

$(objpfx)crti.o: $(objpfx)crti.S $(objpfx)defs.h
	$(compile.S) -g0 $(ASFLAGS-.os) -o $@

generated += crti.S defs.h pt-initfini.s
endif
