# Copyright (C) 2002, 2003 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

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

#
#	Sub-makefile for NPTL portion of the library.
#
subdir	:= nptl

nptl-version := $(shell sed -n 's/^.*$(subdir) \([0-9.]*\).*$$/\1/p' Banner)

headers := pthread.h semaphore.h bits/semaphore.h

extra-libs := libpthread
extra-libs-others := $(extra-libs)

routines = alloca_cutoff forward libc-lowlevellock libc-lowlevelmutex \
	   libc-cancellation
shared-only-routines = forward

libpthread-routines = init events version \
		      pthread_create pthread_exit pthread_detach \
		      pthread_join pthread_tryjoin pthread_timedjoin \
		      pthread_self pthread_equal pthread_yield \
		      pthread_getconcurrency pthread_setconcurrency \
		      pthread_getschedparam pthread_setschedparam \
		      pthread_attr_init pthread_attr_destroy \
		      pthread_attr_getdetachstate pthread_attr_setdetachstate \
		      pthread_attr_getguardsize pthread_attr_setguardsize \
		      pthread_attr_getschedparam pthread_attr_setschedparam \
		      pthread_attr_getschedpolicy pthread_attr_setschedpolicy \
		      pthread_attr_getinheritsched \
		      pthread_attr_setinheritsched \
		      pthread_attr_getscope pthread_attr_setscope \
		      pthread_attr_getstackaddr pthread_attr_setstackaddr \
		      pthread_attr_getstacksize pthread_attr_setstacksize \
		      pthread_attr_getstack pthread_attr_setstack \
		      pthread_getattr_np \
		      pthread_mutex_init pthread_mutex_destroy \
		      pthread_mutex_lock pthread_mutex_trylock \
		      pthread_mutex_timedlock pthread_mutex_unlock \
		      pthread_mutexattr_init pthread_mutexattr_destroy \
		      pthread_mutexattr_getpshared \
		      pthread_mutexattr_setpshared \
		      pthread_mutexattr_gettype pthread_mutexattr_settype \
		      pthread_rwlock_init pthread_rwlock_destroy \
		      pthread_rwlock_rdlock pthread_rwlock_timedrdlock \
		      pthread_rwlock_wrlock pthread_rwlock_timedwrlock \
		      pthread_rwlock_tryrdlock pthread_rwlock_trywrlock \
		      pthread_rwlock_unlock \
		      pthread_rwlockattr_init pthread_rwlockattr_destroy \
		      pthread_rwlockattr_getpshared \
		      pthread_rwlockattr_setpshared \
		      pthread_rwlockattr_getkind_np \
		      pthread_rwlockattr_setkind_np \
		      pthread_cond_init pthread_cond_destroy \
		      pthread_cond_wait pthread_cond_timedwait \
		      pthread_cond_signal pthread_cond_broadcast \
		      old_pthread_cond_init old_pthread_cond_destroy \
		      old_pthread_cond_wait old_pthread_cond_timedwait \
		      old_pthread_cond_signal old_pthread_cond_broadcast \
		      pthread_condattr_init pthread_condattr_destroy \
		      pthread_condattr_getpshared pthread_condattr_setpshared \
		      pthread_spin_init pthread_spin_destroy \
		      pthread_spin_lock pthread_spin_trylock \
		      pthread_spin_unlock \
		      pthread_barrier_init pthread_barrier_destroy \
		      pthread_barrier_wait \
		      pthread_barrierattr_init pthread_barrierattr_destroy \
		      pthread_barrierattr_getpshared \
		      pthread_barrierattr_setpshared \
		      pthread_key_create pthread_key_delete \
		      pthread_getspecific pthread_setspecific \
		      pthread_sigmask pthread_kill \
		      pthread_cancel pthread_testcancel \
		      pthread_setcancelstate pthread_setcanceltype \
		      pthread_once \
		      old_pthread_atfork\
		      pthread_getcpuclockid \
		      pthread_clock_gettime pthread_clock_settime \
		      sem_init sem_destroy \
		      sem_open sem_close sem_unlink \
		      sem_getvalue \
		      sem_wait sem_trywait sem_timedwait sem_post \
		      cleanup cleanup_defer \
		      pt-longjmp \
		      cancellation \
		      lowlevellock lowlevelmutex \
		      pt-vfork \
		      ptw-write ptw-read ptw-close ptw-fcntl ptw-accept \
		      ptw-connect ptw-recv ptw-recvfrom ptw-recvmsg ptw-send \
		      ptw-sendmsg ptw-sendto ptw-fsync ptw-lseek ptw-llseek \
		      ptw-msync ptw-nanosleep ptw-open ptw-open64 ptw-pause \
		      ptw-pread ptw-pread64 ptw-pwrite ptw-pwrite64 \
		      ptw-tcdrain ptw-wait ptw-waitpid ptw-msgrcv ptw-msgsnd \
		      ptw-sigwait \
		      pt-raise pt-system \
		      flockfile ftrylockfile funlockfile \
		      sigaction \
		      herrno res pt-allocrtsig \
		      pthread_kill_other_threads

libpthread-shared-only-routines = version pt-allocrtsig

libpthread-nonshared = pthread_atfork

CFLAGS-pthread_atfork.c = -DNOT_IN_libc

# Don't generate deps for calls with no sources.  See sysdeps/unix/Makefile.
omit-deps = $(unix-syscalls:%=ptw-%)


tests = tst-attr1 tst-attr2 \
	tst-mutex1 tst-mutex2 tst-mutex3 tst-mutex4 tst-mutex5 tst-mutex6 \
	tst-mutex7 \
	tst-spin1 tst-spin2 tst-spin3 \
	tst-cond1 tst-cond2 tst-cond3 tst-cond4 tst-cond5 tst-cond6 tst-cond7 \
	tst-cond8 tst-cond9 \
	tst-rwlock1 tst-rwlock2 tst-rwlock3 tst-rwlock4 tst-rwlock5 \
	tst-rwlock6 tst-rwlock7 \
	tst-once1 tst-once2 tst-once3 tst-once4 \
	tst-key1 tst-key2 tst-key3 \
	tst-sem1 tst-sem2 tst-sem3 tst-sem4 tst-sem5 \
	tst-barrier1 tst-barrier2 tst-barrier3 \
	tst-basic1 tst-basic2 tst-basic3 \
	tst-join1 tst-join2 tst-join3 tst-join4 tst-join5 \
	tst-tsd1 tst-tsd2 \
	tst-fork1 tst-fork2 tst-fork3 \
	tst-atfork1 \
	tst-cancel1 tst-cancel2 tst-cancel3 tst-cancel4 tst-cancel5 \
	tst-cancel6 tst-cancel7 tst-cancel8 \
	tst-cleanup1 tst-cleanup2 tst-cleanup3 \
	tst-flock1 tst-flock2 \
	tst-signal1 tst-signal2 tst-signal3 \
	tst-exec1 tst-exec2 tst-exec3 \
	tst-exit1 \
	tst-stdio1 tst-stdio2 \
	tst-stack1 \
	tst-unload \
	tst-sysconf \
	tst-locale1 tst-locale2 \
	tst-umask1 \
	tst-popen1

LDFLAGS-pthread.so = -Wl,--enable-new-dtags,-z,nodelete,-z,initfirst


include ../Makeconfig

ifeq ($(build-shared),yes)
tests += tst-atfork2
endif

modules-names = tst-atfork2mod
extra-objs += $(addsuffix .os,$(strip $(modules-names)))
test-extras += $(modules-names)
test-modules = $(addprefix $(objpfx),$(addsuffix .so,$(modules-names)))

$(test-modules): $(objpfx)%.so: $(objpfx)%.os
	$(build-module)

ifeq ($(build-shared),yes)
others: $(objpfx)libpthread_nonshared.a
endif

$(objpfx)libpthread_nonshared.a: $(addprefix $(objpfx),$(addsuffix .os,$(libpthread-nonshared)))
	$(AR) $(ARFLAGS) $@ $^

ifeq ($(build-shared),yes)
extra-objs += crti.o
omit-deps += crti

CFLAGS-pt-initfini.s = -g0 -fPIC -fno-inline-functions
endif

CFLAGS-flockfile.c = -D_IO_MTSAFE_IO
CFLAGS-ftrylockfile.c = -D_IO_MTSAFE_IO
CFLAGS-funlockfile.c = -D_IO_MTSAFE_IO

ifeq ($(build-static),yes)
tests-static += tst-locale1 tst-locale2
endif
# These tests are linked with libc before libpthread
tests-reverse += tst-cancel5

include ../Rules

ifeq (yes,$(build-shared))
# Make sure these things are built in the `make lib' pass so they can be used
# to run programs during the `make others' pass.
lib-noranlib: $(addprefix $(objpfx),$(extra-objs))
endif

# What we install as libpthread.so for programs to link against is in fact a
# link script.  It contains references for the various libraries we need.
# The libpthread.so object is not complete since some functions are only
# defined in libpthread_nonshared.a.
# We need to use absolute paths since otherwise local copies (if they exist)
# of the files are taken by the linker.
install: $(inst_libdir)/libpthread.so
$(inst_libdir)/libpthread.so: $(common-objpfx)format.lds \
			      $(objpfx)libpthread.so$(libpthread.so-version) \
			      $(inst_libdir)/$(patsubst %,$(libtype.oS),\
							$(libprefix)pthread) \
			      $(+force)
	(echo '/* GNU ld script';\
	 echo '   Use the shared library, but some functions are only in';\
	 echo '   the static library, so try that secondarily.  */';\
	 cat $<; \
	 echo 'GROUP ( $(slibdir)/libpthread.so$(libpthread.so-version)' \
	      '$(libdir)/$(patsubst %,$(libtype.oS),$(libprefix)pthread)'\
	      ')' \
	) > $@.new
	mv -f $@.new $@
$(inst_libdir)/libpthread_nonshared.a: $(objpfx)libpthread_nonshared.a
	$(do-install)

# 'pthread_self' is a simple memory or register load.  Setting up the
# stack frame is more work than the actual operation.  Disable the
# frame creation entirely.  This will help applications which call the
# function frequently to get a thread-specific handle.
CFLAGS-pthread_self.os += -fomit-frame-pointer

CFLAGS-tst-unload.c += -DPREFIX=\"$(objpfx)\"

tst-cancel7-ARGS = --command "$(built-program-cmd)"
tst-umask1-ARGS = $(objpfx)tst-umask1.temp

$(objpfx)tst-atfork2: $(libdl) $(shared-thread-library)
LDFLAGS-tst-atfork2 = -rdynamic
tst-atfork2-ENV = MALLOC_TRACE=$(objpfx)tst-atfork2.mtrace
$(objpfx)tst-atfork2mod.so: $(shared-thread-library)

extra-B-pthread.so = -B$(common-objpfx)nptl/
$(objpfx)libpthread.so: $(objpfx)crti.o
$(objpfx)libpthread.so: +preinit += $(objpfx)crti.o

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
# Depend on ld.so too to get proper versions of ld.so symbols.
$(objpfx)libpthread.so: $(common-objpfx)libc.so \
			$(common-objpfx)libc_nonshared.a \
			$(if $(filter yes,$(elf)), $(elfobjdir)/ld.so)

# Make sure we link with the thread library.
ifeq ($(build-shared),yes)
$(addprefix $(objpfx), \
  $(filter-out $(tests-static) $(tests-reverse), \
    $(tests) $(test-srcs))): $(objpfx)libpthread.so \
			     $(objpfx)libpthread_nonshared.a
$(objpfx)tst-unload: $(common-objpfx)dlfcn/libdl.so
# $(objpfx)../libc.so is used instead of $(common-objpfx)libc.so,
# since otherwise libpthread.so comes before libc.so when linking.
$(addprefix $(objpfx), $(tests-reverse)): \
  $(objpfx)../libc.so $(objpfx)libpthread.so \
  $(objpfx)libpthread_nonshared.a
$(addprefix $(objpfx),$(tests-static)): $(objpfx)libpthread.a

$(objpfx)tst-atfork2.out: $(objpfx)tst-atfork2mod.so
else
$(addprefix $(objpfx),$(tests) $(test-srcs)): $(objpfx)libpthread.a
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

generated += crti.S defs.h pt-initfini.s $(objpfx)tst-atfork2.mtrace \
	     $(addsuffix .so,$(strip $(modules-names)))

$(objpfx)version.os: $(objpfx)banner.h
$(objpfx)banner.h: Banner
	sed 's/\(.*\)/"\1\\n"/' $< > $@
generated += banner.h
# Give libpthread.so an entry point and make it directly runnable itself.
LDFLAGS-pthread.so += -e __nptl_main
endif

ifeq (no,$(cross-compiling))
ifeq (yes,$(build-shared))
tests: $(objpfx)tst-cancel-wrappers.out
$(objpfx)tst-cancel-wrappers.out: tst-cancel-wrappers.sh
	$(SHELL) $< $(common-objpfx)libc_pic.a \
		    $(common-objpfx)libc.a \
		    $(objpfx)libpthread_pic.a \
		    $(objpfx)libpthread.a > $@
endif
endif
