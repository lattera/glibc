# Copyright (C) 2002-2008,2009,2010,2012 Free Software Foundation, Inc.
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

headers := pthread.h semaphore.h bits/semaphore.h

extra-libs := libpthread
extra-libs-others := $(extra-libs)
install-lib-ldscripts := libpthread.so

routines = alloca_cutoff forward libc-lowlevellock libc-cancellation \
	   libc-cleanup
shared-only-routines = forward

libpthread-routines = nptl-init vars events version \
		      pthread_create pthread_exit pthread_detach \
		      pthread_join pthread_tryjoin pthread_timedjoin \
		      pthread_self pthread_equal pthread_yield \
		      pthread_getconcurrency pthread_setconcurrency \
		      pthread_getschedparam pthread_setschedparam \
		      pthread_setschedprio \
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
		      pthread_condattr_getclock pthread_condattr_setclock \
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
		      pthread_sigmask pthread_kill pthread_sigqueue \
		      pthread_cancel pthread_testcancel \
		      pthread_setcancelstate pthread_setcanceltype \
		      pthread_once \
		      old_pthread_atfork pthread_atfork \
		      pthread_getcpuclockid \
		      pthread_clock_gettime pthread_clock_settime \
		      sem_init sem_destroy \
		      sem_open sem_close sem_unlink \
		      sem_getvalue \
		      sem_wait sem_trywait sem_timedwait sem_post \
		      cleanup cleanup_defer cleanup_compat \
		      cleanup_defer_compat unwind \
		      pt-longjmp pt-cleanup\
		      cancellation \
		      lowlevellock lowlevelrobustlock \
		      pt-vfork \
		      ptw-write ptw-read ptw-close ptw-fcntl ptw-accept \
		      ptw-connect ptw-recv ptw-recvfrom ptw-recvmsg ptw-send \
		      ptw-sendmsg ptw-sendto ptw-fsync ptw-lseek ptw-llseek \
		      ptw-msync ptw-nanosleep ptw-open ptw-open64 ptw-pause \
		      ptw-pread ptw-pread64 ptw-pwrite ptw-pwrite64 \
		      ptw-tcdrain ptw-wait ptw-waitpid ptw-msgrcv ptw-msgsnd \
		      ptw-sigwait ptw-sigsuspend \
		      pt-raise pt-system \
		      flockfile ftrylockfile funlockfile \
		      sigaction \
		      herrno res pt-allocrtsig \
		      pthread_kill_other_threads \
		      pthread_getaffinity pthread_setaffinity \
		      pthread_attr_getaffinity pthread_attr_setaffinity \
		      pthread_mutexattr_getrobust pthread_mutexattr_setrobust \
		      pthread_mutex_consistent \
		      cleanup_routine unwind-forcedunwind \
		      pthread_mutexattr_getprotocol \
		      pthread_mutexattr_setprotocol \
		      pthread_mutexattr_getprioceiling \
		      pthread_mutexattr_setprioceiling tpp \
		      pthread_mutex_getprioceiling \
		      pthread_mutex_setprioceiling \
		      pthread_setname pthread_getname
#		      pthread_setuid pthread_seteuid pthread_setreuid \
#		      pthread_setresuid \
#		      pthread_setgid pthread_setegid pthread_setregid \
#		      pthread_setresgid

libpthread-shared-only-routines = version pt-allocrtsig unwind-forcedunwind
libpthread-static-only-routines = pthread_atfork

CFLAGS-pthread_atfork.c = -DNOT_IN_libc

# Since cancellation handling is in large parts handled using exceptions
# we have to compile some files with exception handling enabled, some
# even with asynchronous unwind tables.

# nptl-init.c contains sigcancel_handler().
CFLAGS-nptl-init.c = -fexceptions -fasynchronous-unwind-tables
# The unwind code itself,
CFLAGS-unwind.c = -fexceptions
CFLAGS-unwind-forcedunwind.c = -fexceptions -fasynchronous-unwind-tables

# The following three functions must be async-cancel safe.
CFLAGS-pthread_cancel.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-pthread_setcancelstate.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-pthread_setcanceltype.c = -fexceptions -fasynchronous-unwind-tables

# These are internal functions which similar functionality as setcancelstate
# and setcanceltype.
CFLAGS-cancellation.c = -fasynchronous-unwind-tables
CFLAGS-libc-cancellation.c = -fasynchronous-unwind-tables

# Calling pthread_exit() must cause the registered cancel handlers to
# be executed.  Therefore exceptions have to be thrown through this
# function.
CFLAGS-pthread_exit.c = -fexceptions

# Among others, __pthread_unwind is forwarded.  This function must handle
# exceptions.
CFLAGS-forward.c = -fexceptions

# The following are cancellation points.  Some of the functions can
# block and therefore temporarily enable asynchronous cancellation.
# Those must be compiled asynchronous unwind tables.
CFLAGS-pthread_testcancel.c = -fexceptions
CFLAGS-pthread_join.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-pthread_timedjoin.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-pthread_once.c = $(uses-callbacks) -fexceptions \
			-fasynchronous-unwind-tables
CFLAGS-pthread_cond_wait.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-pthread_cond_timedwait.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-sem_wait.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-sem_timedwait.c = -fexceptions -fasynchronous-unwind-tables

# These are the function wrappers we have to duplicate here.
CFLAGS-fcntl.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-lockf.c = -fexceptions
CFLAGS-pread.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-pread64.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-pwrite.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-pwrite64.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-wait.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-waitpid.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-sigwait.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-msgrcv.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-msgsnd.c = -fexceptions -fasynchronous-unwind-tables
CFLAGS-tcdrain.c = -fexceptions -fasynchronous-unwind-tables

CFLAGS-pt-system.c = -fexceptions


tests = tst-typesizes \
	tst-attr1 tst-attr2 tst-attr3 \
	tst-mutex1 tst-mutex2 tst-mutex3 tst-mutex4 tst-mutex5 tst-mutex6 \
	tst-mutex7 tst-mutex8 tst-mutex9 tst-mutex5a tst-mutex7a \
	tst-mutexpi1 tst-mutexpi2 tst-mutexpi3 tst-mutexpi4 tst-mutexpi5 \
	tst-mutexpi5a tst-mutexpi6 tst-mutexpi7 tst-mutexpi7a tst-mutexpi8 \
	tst-mutexpi9 \
	tst-spin1 tst-spin2 tst-spin3 \
	tst-cond1 tst-cond2 tst-cond3 tst-cond4 tst-cond5 tst-cond6 tst-cond7 \
	tst-cond8 tst-cond9 tst-cond10 tst-cond11 tst-cond12 tst-cond13 \
	tst-cond14 tst-cond15 tst-cond16 tst-cond17 tst-cond18 tst-cond19 \
	tst-cond20 tst-cond21 tst-cond22 tst-cond23 \
	tst-robust1 tst-robust2 tst-robust3 tst-robust4 tst-robust5 \
	tst-robust6 tst-robust7 tst-robust8 tst-robust9 \
	tst-robustpi1 tst-robustpi2 tst-robustpi3 tst-robustpi4 tst-robustpi5 \
	tst-robustpi6 tst-robustpi7 tst-robustpi8 tst-robustpi9 \
	tst-rwlock1 tst-rwlock2 tst-rwlock2a tst-rwlock3 tst-rwlock4 \
	tst-rwlock5 tst-rwlock6 tst-rwlock7 tst-rwlock8 tst-rwlock9 \
	tst-rwlock10 tst-rwlock11 tst-rwlock12 tst-rwlock13 tst-rwlock14 \
	tst-once1 tst-once2 tst-once3 tst-once4 \
	tst-key1 tst-key2 tst-key3 tst-key4 \
	tst-sem1 tst-sem2 tst-sem3 tst-sem4 tst-sem5 tst-sem6 tst-sem7 \
	tst-sem8 tst-sem9 tst-sem10 tst-sem11 tst-sem12 tst-sem13 \
	tst-barrier1 tst-barrier2 tst-barrier3 tst-barrier4 \
	tst-align tst-align2 tst-align3 \
	tst-basic1 tst-basic2 tst-basic3 tst-basic4 tst-basic5 tst-basic6 \
	tst-basic7 \
	tst-kill1 tst-kill2 tst-kill3 tst-kill4 tst-kill5 tst-kill6 \
	tst-raise1 \
	tst-join1 tst-join2 tst-join3 tst-join4 tst-join5 tst-join6 \
	tst-detach1 \
	tst-eintr1 tst-eintr2 tst-eintr3 tst-eintr4 tst-eintr5 \
	tst-tsd1 tst-tsd2 tst-tsd3 tst-tsd4 tst-tsd5 tst-tsd6 \
	tst-tls1 tst-tls2 \
	tst-fork1 tst-fork2 tst-fork3 tst-fork4 \
	tst-atfork1 \
	tst-cancel1 tst-cancel2 tst-cancel3 tst-cancel4 tst-cancel5 \
	tst-cancel6 tst-cancel7 tst-cancel8 tst-cancel9 tst-cancel10 \
	tst-cancel11 tst-cancel12 tst-cancel13 tst-cancel14 tst-cancel15 \
	tst-cancel16 tst-cancel17 tst-cancel18 tst-cancel19 tst-cancel20 \
	tst-cancel21 tst-cancel22 tst-cancel23 tst-cancel24 tst-cancel25 \
	tst-cleanup0 tst-cleanup1 tst-cleanup2 tst-cleanup3 tst-cleanup4 \
	tst-flock1 tst-flock2 \
	tst-signal1 tst-signal2 tst-signal3 tst-signal4 tst-signal5 \
	tst-signal6 tst-signal7 \
	tst-exec1 tst-exec2 tst-exec3 tst-exec4 \
	tst-exit1 tst-exit2 tst-exit3 \
	tst-stdio1 tst-stdio2 \
	tst-stack1 tst-stack2 tst-stack3 \
	tst-unload \
	tst-dlsym1 \
	tst-sysconf \
	tst-locale1 tst-locale2 \
	tst-umask1 \
	tst-popen1 \
	tst-clock1 \
	tst-context1 \
	tst-sched1 \
	tst-backtrace1 \
	tst-oddstacklimit \
	tst-abstime \
	tst-vfork1 tst-vfork2 tst-vfork1x tst-vfork2x \
	tst-getpid1 tst-getpid2 tst-getpid3 \
	tst-initializers1 $(patsubst %,tst-initializers1-%,c89 gnu89 c99 gnu99)
xtests = tst-setuid1 tst-setuid1-static tst-mutexpp1 tst-mutexpp6 tst-mutexpp10

# Files which must not be linked with libpthread.
tests-nolibpthread = tst-unload

# This sets the stack resource limit to 1023kb, which is not a multiple
# of the page size since every architecture's page size is > 1k.
tst-oddstacklimit-ENV = ; ulimit -s 1023;

distribute = eintr.c tst-cleanup4aux.c

gen-as-const-headers = pthread-errnos.sym

LDFLAGS-pthread.so = -Wl,--enable-new-dtags,-z,nodelete,-z,initfirst


include ../Makeconfig

ifeq ($(have-forced-unwind),yes)
tests += tst-cancelx2 tst-cancelx3 tst-cancelx4 tst-cancelx5 \
	 tst-cancelx6 tst-cancelx7 tst-cancelx8 tst-cancelx9 tst-cancelx10 \
	 tst-cancelx11 tst-cancelx12 tst-cancelx13 tst-cancelx14 tst-cancelx15 \
	 tst-cancelx16 tst-cancelx17 tst-cancelx18 tst-cancelx20 tst-cancelx21 \
	 tst-cleanupx0 tst-cleanupx1 tst-cleanupx2 tst-cleanupx3 tst-cleanupx4 \
	 tst-oncex3 tst-oncex4
endif
ifeq ($(build-shared),yes)
tests += tst-atfork2 tst-tls3 tst-tls4 tst-tls5 tst-_res1 tst-fini1 \
	 tst-stackguard1
tests-nolibpthread += tst-fini1
ifeq ($(have-z-execstack),yes)
tests += tst-execstack
endif
endif

modules-names = tst-atfork2mod tst-tls3mod tst-tls4moda tst-tls4modb \
		tst-tls5mod tst-tls5moda tst-tls5modb tst-tls5modc \
		tst-tls5modd tst-tls5mode tst-tls5modf \
		tst-_res1mod1 tst-_res1mod2 tst-execstack-mod tst-fini1mod
extra-test-objs += $(addsuffix .os,$(strip $(modules-names))) tst-cleanup4aux.o
test-extras += $(modules-names) tst-cleanup4aux
test-modules = $(addprefix $(objpfx),$(addsuffix .so,$(modules-names)))

tst-atfork2mod.so-no-z-defs = yes
tst-tls3mod.so-no-z-defs = yes
tst-tls5mod.so-no-z-defs = yes
tst-tls5moda.so-no-z-defs = yes
tst-tls5modb.so-no-z-defs = yes
tst-tls5modc.so-no-z-defs = yes
tst-tls5modd.so-no-z-defs = yes
tst-tls5mode.so-no-z-defs = yes
tst-tls5modf.so-no-z-defs = yes

ifeq ($(build-shared),yes)
# Build all the modules even when not actually running test programs.
tests: $(test-modules)
endif

ifeq ($(build-shared),yes)

# Set the `multidir' variable by grabbing the variable from the compiler.
# We do it once and save the result in a generated makefile.
-include $(objpfx)multidir.mk
$(objpfx)multidir.mk: $(common-objpfx)config.make
	$(make-target-directory)
	dir=`$(CC) $(CFLAGS) $(CPPFLAGS) -print-multi-directory`; \
	echo "multidir := $$dir" > $@T
	mv -f $@T $@

crti-objs := crti.o
crtn-objs := crtn.o
ifneq (,$(patsubst .,,$(multidir)))
generated-dirs := $(firstword $(subst /, , $(multidir)))
crti-objs += $(multidir)/crti.o
crtn-objs += $(multidir)/crtn.o
omit-deps += $(multidir)/crti $(multidir)/crtn
$(objpfx)$(multidir):
	mkdir -p $@
endif
extra-objs += $(crti-objs) $(crtn-objs)
omit-deps += crti crtn

CFLAGS-pt-initfini.s = -g0 -fPIC -fno-inline-functions $(fno-unit-at-a-time)
endif

CFLAGS-flockfile.c = -D_IO_MTSAFE_IO
CFLAGS-ftrylockfile.c = -D_IO_MTSAFE_IO
CFLAGS-funlockfile.c = -D_IO_MTSAFE_IO

link-libc-static := $(common-objpfx)libc.a $(static-gnulib) \
		    $(common-objpfx)libc.a

tests-static += tst-locale1 tst-locale2 tst-stackguard1-static
tests += tst-stackguard1-static
xtests-static += tst-setuid1-static

# These tests are linked with libc before libpthread
tests-reverse += tst-cancel5 tst-cancel23 tst-vfork1x tst-vfork2x

include ../Rules

ifeq (yes,$(build-shared))
# Make sure these things are built in the `make lib' pass so they can be used
# to run programs during the `make others' pass.
lib-noranlib: $(addprefix $(objpfx),$(extra-objs))

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
endif


# 'pthread_self' is a simple memory or register load.  Setting up the
# stack frame is more work than the actual operation.  Disable the
# frame creation entirely.  This will help applications which call the
# function frequently to get a thread-specific handle.
CFLAGS-pthread_self.os += -fomit-frame-pointer

# Run the cancellation and cleanup tests also for the modern, exception-based
# implementation.  For this we have to pass the -fexceptions parameter.
CFLAGS-tst-cancelx2.c += -fexceptions
CFLAGS-tst-cancelx3.c += -fexceptions
CFLAGS-tst-cancelx4.c += -fexceptions
CFLAGS-tst-cancelx5.c += -fexceptions
CFLAGS-tst-cancelx6.c += -fexceptions
CFLAGS-tst-cancelx7.c += -fexceptions
CFLAGS-tst-cancelx8.c += -fexceptions
CFLAGS-tst-cancelx9.c += -fexceptions
CFLAGS-tst-cancelx10.c += -fexceptions
CFLAGS-tst-cancelx11.c += -fexceptions
CFLAGS-tst-cancelx12.c += -fexceptions
CFLAGS-tst-cancelx13.c += -fexceptions
CFLAGS-tst-cancelx14.c += -fexceptions
CFLAGS-tst-cancelx15.c += -fexceptions
CFLAGS-tst-cancelx16.c += -fexceptions
CFLAGS-tst-cancelx17.c += -fexceptions
CFLAGS-tst-cancelx18.c += -fexceptions
CFLAGS-tst-cancelx20.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-tst-cancelx21.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-tst-cleanupx0.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-tst-cleanupx1.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-tst-cleanupx2.c += -fexceptions
CFLAGS-tst-cleanupx3.c += -fexceptions
CFLAGS-tst-cleanupx4.c += -fexceptions
CFLAGS-tst-oncex3.c += -fexceptions
CFLAGS-tst-oncex4.c += -fexceptions
CFLAGS-tst-align.c += $(stack-align-test-flags)
CFLAGS-tst-align3.c += $(stack-align-test-flags)
CFLAGS-tst-initializers1.c = -W -Wall -Werror
CFLAGS-tst-initializers1-< = $(CFLAGS-tst-initializers1.c) \
			     $(patsubst tst-initializers1-%.c,-std=%,$<)
CFLAGS-tst-initializers1-c89.c = $(CFLAGS-tst-initializers1-<)
CFLAGS-tst-initializers1-c99.c = $(CFLAGS-tst-initializers1-<)
CFLAGS-tst-initializers1-gnu89.c = $(CFLAGS-tst-initializers1-<)
CFLAGS-tst-initializers1-gnu99.c = $(CFLAGS-tst-initializers1-<)

tst-cancel7-ARGS = --command "$(built-program-cmd)"
tst-cancelx7-ARGS = $(tst-cancel7-ARGS)
tst-umask1-ARGS = $(objpfx)tst-umask1.temp

$(objpfx)tst-atfork2: $(libdl) $(shared-thread-library)
LDFLAGS-tst-atfork2 = -rdynamic
tst-atfork2-ENV = MALLOC_TRACE=$(objpfx)tst-atfork2.mtrace
$(objpfx)tst-atfork2mod.so: $(shared-thread-library)

tests: $(objpfx)tst-stack3-mem
tst-stack3-ENV = MALLOC_TRACE=$(objpfx)tst-stack3.mtrace
$(objpfx)tst-stack3-mem: $(objpfx)tst-stack3.out
	$(common-objpfx)malloc/mtrace $(objpfx)tst-stack3.mtrace > $@
generated += tst-stack3-mem tst-stack3.mtrace

$(objpfx)tst-cleanup4: $(objpfx)tst-cleanup4aux.o $(shared-thread-library)
$(objpfx)tst-cleanupx4: $(objpfx)tst-cleanup4aux.o $(shared-thread-library)

$(objpfx)tst-tls3: $(libdl) $(shared-thread-library)
LDFLAGS-tst-tls3 = -rdynamic
$(objpfx)tst-tls3.out: $(objpfx)tst-tls3mod.so
$(objpfx)tst-tls3mod.so: $(shared-thread-library)

$(objpfx)tst-tls4: $(libdl) $(shared-thread-library)
$(objpfx)tst-tls4.out: $(objpfx)tst-tls4moda.so $(objpfx)tst-tls4modb.so

$(objpfx)tst-tls5: $(objpfx)tst-tls5mod.so $(shared-thread-library)
LDFLAGS-tst-tls5mod.so = -Wl,-soname,tst-tls5mod.so

ifeq ($(build-shared),yes)
tests: $(objpfx)tst-tls6.out
$(objpfx)tst-tls6.out: tst-tls6.sh $(objpfx)tst-tls5 \
		       $(objpfx)tst-tls5moda.so $(objpfx)tst-tls5modb.so \
		       $(objpfx)tst-tls5modc.so $(objpfx)tst-tls5modd.so \
		       $(objpfx)tst-tls5mode.so $(objpfx)tst-tls5modf.so
	$(SHELL) -e tst-tls6.sh $(common-objpfx) $(elf-objpfx) \
		    $(rtld-installed-name)
endif

$(objpfx)tst-dlsym1: $(libdl) $(shared-thread-library)

$(objpfx)tst-fini1: $(shared-thread-library) $(objpfx)tst-fini1mod.so

ifeq (yes,$(build-shared))
$(objpfx)tst-cond11: $(common-objpfx)rt/librt.so
$(objpfx)tst-cond19: $(common-objpfx)rt/librt.so
$(objpfx)tst-sem5: $(common-objpfx)rt/librt.so
$(objpfx)tst-cancel17: $(common-objpfx)rt/librt.so
$(objpfx)tst-cancelx17: $(common-objpfx)rt/librt.so
$(objpfx)tst-cancel18: $(common-objpfx)rt/librt.so
$(objpfx)tst-cancelx18: $(common-objpfx)rt/librt.so
$(objpfx)tst-clock2: $(common-objpfx)rt/librt.so
$(objpfx)tst-rwlock14: $(common-objpfx)rt/librt.so
$(objpfx)tst-_res1mod2.so: $(objpfx)tst-_res1mod1.so
LDFLAGS-tst-_res1mod1.so = -Wl,-soname,tst-_res1mod1.so
LDFLAGS-tst-_res1mod2.so = -Wl,-soname,tst-_res1mod2.so
$(objpfx)tst-_res1: $(objpfx)tst-_res1mod1.so $(objpfx)tst-_res1mod2.so \
		    $(shared-thread-library)
else
$(objpfx)tst-cond11: $(common-objpfx)rt/librt.a
$(objpfx)tst-cond19: $(common-objpfx)rt/librt.a
$(objpfx)tst-sem5: $(common-objpfx)rt/librt.a
$(objpfx)tst-cancel17: $(common-objpfx)rt/librt.a
$(objpfx)tst-cancelx17: $(common-objpfx)rt/librt.a
$(objpfx)tst-cancel18: $(common-objpfx)rt/librt.a
$(objpfx)tst-cancelx18: $(common-objpfx)rt/librt.a
$(objpfx)tst-clock2: $(common-objpfx)rt/librt.a
$(objpfx)tst-rwlock14: $(common-objpfx)rt/librt.a
endif

LDFLAGS-tst-cancel24 = -lstdc++

extra-B-pthread.so = -B$(common-objpfx)nptl/
$(objpfx)libpthread.so: $(addprefix $(objpfx),$(crti-objs) $(crtn-objs))
$(objpfx)libpthread.so: +preinit += $(addprefix $(objpfx),$(crti-objs))
$(objpfx)libpthread.so: +postinit += $(addprefix $(objpfx),$(crtn-objs))

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
# Depend on ld.so too to get proper versions of ld.so symbols.
$(objpfx)libpthread.so: $(common-objpfx)libc.so \
			$(common-objpfx)libc_nonshared.a \
			$(elfobjdir)/ld.so

# Make sure we link with the thread library.
ifeq ($(build-shared),yes)
$(addprefix $(objpfx), \
  $(filter-out $(tests-static) $(xtests-static) $(tests-reverse) \
    $(tests-nolibpthread), \
    $(tests) $(xtests) $(test-srcs))): $(objpfx)libpthread.so \
				       $(objpfx)libpthread_nonshared.a
$(objpfx)tst-unload: $(common-objpfx)dlfcn/libdl.so
# $(objpfx)../libc.so is used instead of $(common-objpfx)libc.so,
# since otherwise libpthread.so comes before libc.so when linking.
$(addprefix $(objpfx), $(tests-reverse)): \
  $(objpfx)../libc.so $(objpfx)libpthread.so \
  $(objpfx)libpthread_nonshared.a
$(objpfx)../libc.so: $(common-objpfx)libc.so ;
$(addprefix $(objpfx),$(tests-static) $(xtests-static)): $(objpfx)libpthread.a

$(objpfx)tst-atfork2.out: $(objpfx)tst-atfork2mod.so
else
$(addprefix $(objpfx),$(tests) $(test-srcs)): $(objpfx)libpthread.a
endif

ifeq ($(build-shared),yes)
vpath pt-initfini.c $(sysdirs)

$(objpfx)pt-initfini.s: pt-initfini.c
	$(compile.c) -S $(CFLAGS-pt-initfini.s) -finhibit-size-directive \
		$(patsubst -f%,-fno-%,$(exceptions)) -o $@

$(objpfx)tst-cleanup0.out: /dev/null $(objpfx)tst-cleanup0
	$(make-test-out) 2>&1 | cmp - tst-cleanup0.expect > $@

# We only have one kind of startup code files.  Static binaries and
# shared libraries are build using the PIC version.
$(objpfx)crti.S: $(objpfx)pt-initfini.s
	sed -n -e '1,/@HEADER_ENDS/p' \
	       -e '/@_.*_PROLOG_BEGINS/,/@_.*_PROLOG_ENDS/p' \
	       -e '/@TRAILER_BEGINS/,$$p' $< > $@
$(objpfx)crtn.S: $(objpfx)pt-initfini.s
	sed -n -e '1,/@HEADER_ENDS/p' \
	       -e '/@_.*_EPILOG_BEGINS/,/@_.*_EPILOG_ENDS/p' \
	       -e '/@TRAILER_BEGINS/,$$p' $< > $@

$(objpfx)defs.h: $(objpfx)pt-initfini.s
	sed -n -e '/@TESTS_BEGIN/,/@TESTS_END/p' $< | \
		$(AWK) -f ../csu/defs.awk > $@

$(objpfx)crti.o: $(objpfx)crti.S $(objpfx)defs.h
	$(compile.S) -g0 $(ASFLAGS-.os) -o $@

$(objpfx)crtn.o: $(objpfx)crtn.S $(objpfx)defs.h
	$(compile.S) -g0 $(ASFLAGS-.os) -o $@

ifneq ($(multidir),.)
$(objpfx)$(multidir)/crti.o: $(objpfx)crti.o $(objpfx)$(multidir)/
	ln -f $< $@

$(objpfx)$(multidir)/crtn.o: $(objpfx)crtn.o $(objpfx)$(multidir)/
	ln -f $< $@
endif

generated += crti.S crtn.S defs.h pt-initfini.s  libpthread_nonshared.a \
	     multidir.mk tst-atfork2.mtrace tst-cancel-wrappers.out \
	     tst-tls6.out

generated += $(objpfx)tst-atfork2.mtrace \
	     $(addsuffix .so,$(strip $(modules-names)))

$(objpfx)version.d: $(objpfx)banner.h
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

tst-exec4-ARGS = $(built-program-cmd)

$(objpfx)tst-execstack: $(libdl)
$(objpfx)tst-execstack.out: $(objpfx)tst-execstack-mod.so
LDFLAGS-tst-execstack = -Wl,-z,noexecstack

$(objpfx)tst-fini1mod.so: $(shared-thread-library)

tst-stackguard1-ARGS = --command "$(built-program-cmd) --child"
tst-stackguard1-static-ARGS = --command "$(objpfx)tst-stackguard1-static --child"

# The tests here better do not run in parallel
ifneq ($(filter %tests,$(MAKECMDGOALS)),)
.NOTPARALLEL:
endif
