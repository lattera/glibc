# Copyright (C) 2002 Free Software Foundation, Inc.
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

headers := pthread.h semaphore.h

extra-libs := libpthread
extra-libs-others := $(extra-libs)

routines = alloca_cutoff forward
shared-only-routines = forward

libpthread-routines = init events \
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
		      pthread_rwlock_tryrdlock pthread_rwlock_trywrlock \
		      pthread_rwlockattr_init pthread_rwlockattr_destroy \
		      pthread_rwlockattr_getpshared \
		      pthread_rwlockattr_setpshared \
		      pthread_rwlockattr_getkind_np \
		      pthread_rwlockattr_setkind_np \
		      pthread_cond_init pthread_cond_destroy \
		      pthread_cond_wait pthread_cond_timedwait \
		      pthread_cond_signal pthread_cond_broadcast \
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
		      cleanup cleanup_defer \
		      pt-longjmp \
		      cancellation \
		      lowlevellock lowlevelmutex lowlevelcond lowlevelrwlock \
		      lowlevelsem \
		      pt-vfork \
		      pt-write pt-read pt-close pt-fcntl pt-accept pt-connect \
		      pt-recv pt-recvfrom pt-recvmsg pt-send pt-sendmsg \
		      pt-sendto pt-fsync pt-lseek pt-lseek64 pt-msync \
		      pt-nanosleep pt-open pt-open64 pt-pause pt-pread \
		      pt-pread64 pt-pwrite pt-pwrite64 pt-tcdrain pt-system \
		      pt-wait pt-waitpid pt-readv pt-writev pt-creat \
		      pt-msgrcv pt-msgsnd pt-poll pt-select pt-sigpause \
		      pt-sigsuspend pt-sigwait pt-sigwaitinfo pt-waitid \
		      pt-pselect pt-raise \
		      flockfile ftrylockfile funlockfile \
		      sigaction \
		      herrno res pt-allocrtsig \
		      pthread_kill_other_threads

libpthread-nonshared = pthread_atfork


tests = tst-mutex1 tst-mutex2 tst-mutex3 tst-mutex4 tst-mutex5 tst-mutex6 \
	tst-mutex7 \
	tst-spin1 tst-spin2 tst-spin3 \
	tst-cond1 tst-cond2 tst-cond3 tst-cond4 tst-cond5 tst-cond6 \
	tst-rwlock1 tst-rwlock2 tst-rwlock3 tst-rwlock4 tst-rwlock5 \
	tst-rwlock6 tst-rwlock7 \
	tst-once1 tst-once2 \
	tst-key1 tst-key2 tst-key3 \
	tst-sem1 tst-sem2 tst-sem3 tst-sem4 tst-sem5 \
	tst-barrier1 tst-barrier2 tst-barrier3 \
	tst-basic1 tst-basic2 \
	tst-join1 tst-join2 tst-join3 \
	tst-tsd1 tst-tsd2 \
	tst-fork1 tst-fork2 tst-fork3 \
	tst-atfork1 \
	tst-cancel1 tst-cancel2 tst-cancel3 tst-cancel4 \
	tst-flock1 tst-flock2 \
	tst-signal1 tst-signal2 tst-signal3 \
	tst-exec1 tst-exec2 tst-exec3 \
	tst-exit1 \
	tst-stack1 \
	tst-unload \
	tst-sysconf \
	tst-locale1 tst-locale2

LDFLAGS-pthread.so = -Wl,--enable-new-dtags,-z,nodelete,-z,initfirst


include ../Makeconfig

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

ifeq ($(build-static),yes)
tests-static += tst-locale1 tst-locale2
endif

include ../Rules

# What we install as libc.so for programs to link against is in fact a
# link script.  It contains references for the various libraries we need.
# The libc.so object is not complete since some functions are only defined
# in libc_nonshared.a.
# We need to use absolute paths since otherwise local copies (if they exist)
# of the files are taken by the linker.
install: $(inst_libdir)/libpthread.so
$(inst_libdir)/libpthread.so: $(objpfx)libpthread.so$(libpthread.so-version) \
			      $(inst_libdir)/$(patsubst %,$(libtype.oS),\
							$(libprefix)pthread) \
			      $(+force)
	(echo '/* GNU ld script';\
	 echo '   Use the shared library, but some functions are only in';\
	 echo '   the static library, so try that secondarily.  */';\
	 echo 'GROUP ( $(slibdir)/libpthread.so$(libpthread.so-version)' \
	      '$(libdir)/$(patsubst %,$(libtype.oS),$(libprefix)pthread)'\
	      ')' \
	) > $@.new
	mv -f $@.new $@
$(inst_libdir)/libpthread_nonshared.a: $(objpfx)libpthread_nonshared.a
	$(do-install)


CFLAGS-tst-unload.c += -DPREFIX=\"$(objpfx)\"

extra-B-pthread.so = -B$(common-objpfx)linuxthreads/
$(objpfx)libpthread.so: $(objpfx)crti.o
$(objpfx)libpthread.so: +preinit += $(objpfx)crti.o

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libpthread.so: $(common-objpfx)libc.so \
			$(common-objpfx)libc_nonshared.a

# Make sure we link with the thread library.
ifeq ($(build-shared),yes)
$(addprefix $(objpfx), \
  $(filter-out $(tests-static), \
    $(tests) $(test-srcs))): $(objpfx)libpthread.so \
			     $(objpfx)libpthread_nonshared.a
$(objpfx)tst-unload: $(common-objpfx)dlfcn/libdl.so
$(addprefix $(objpfx),$(tests-static)): $(objpfx)libpthread.a
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

generated += crti.S defs.h pt-initfini.s
endif
