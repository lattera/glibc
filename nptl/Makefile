# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

#
#	Sub-makefile for NPTL portion of the library.
#
subdir	:= nptl

include ../Makeconfig

headers := pthread.h semaphore.h bits/semaphore.h threads.h

extra-libs := libpthread
extra-libs-others := $(extra-libs)

routines = alloca_cutoff forward libc-lowlevellock libc-cancellation \
	   libc-cleanup libc_pthread_init libc_multiple_threads \
	   register-atfork pthread_atfork pthread_self thrd_current \
	   thrd_equal thrd_sleep thrd_yield
shared-only-routines = forward
static-only-routines = pthread_atfork

# We need to provide certain routines for compatibility with existing
# binaries.
pthread-compat-wrappers = \
		      write read close accept \
		      connect recv recvfrom send \
		      sendto fsync lseek lseek64 \
		      msync nanosleep open open64 pause \
		      pread pread64 pwrite pwrite64 \
		      tcdrain wait waitpid msgrcv msgsnd \
		      sigwait sigsuspend \
		      recvmsg sendmsg

libpthread-routines = nptl-init nptlfreeres vars events version pt-interp \
		      pthread_create pthread_exit pthread_detach \
		      pthread_join pthread_tryjoin pthread_timedjoin \
		      pthread_join_common \
		      compat-pthread_self pthread_equal pthread_yield \
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
		      pthread_mutex_cond_lock \
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
		      pthread_cond_wait \
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
		      old_pthread_atfork \
		      pthread_getcpuclockid \
		      pthread_clock_gettime pthread_clock_settime \
		      shm-directory \
		      sem_init sem_destroy \
		      sem_open sem_close sem_unlink \
		      sem_getvalue \
		      sem_wait sem_timedwait sem_post \
		      cleanup cleanup_defer cleanup_compat \
		      cleanup_defer_compat unwind \
		      pt-longjmp pt-cleanup\
		      cancellation \
		      lowlevellock \
		      lll_timedlock_wait lll_timedwait_tid \
		      pt-fork pt-vfork pt-fcntl \
		      $(pthread-compat-wrappers) \
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
		      pthread_setname pthread_getname \
		      pthread_setattr_default_np pthread_getattr_default_np \
		      thrd_create thrd_detach thrd_exit thrd_join \
		      mtx_destroy mtx_init mtx_lock mtx_timedlock \
		      mtx_trylock mtx_unlock call_once cnd_broadcast \
		      cnd_destroy cnd_init cnd_signal cnd_timedwait cnd_wait \
		      tss_create tss_delete tss_get tss_set
#		      pthread_setuid pthread_seteuid pthread_setreuid \
#		      pthread_setresuid \
#		      pthread_setgid pthread_setegid pthread_setregid \
#		      pthread_setresgid

libpthread-shared-only-routines = version pt-interp pt-allocrtsig \
				  unwind-forcedunwind

# Since cancellation handling is in large parts handled using exceptions
# we have to compile some files with exception handling enabled, some
# even with asynchronous unwind tables.

# nptl-init.c contains sigcancel_handler().
CFLAGS-nptl-init.c += -fexceptions -fasynchronous-unwind-tables
# The unwind code itself,
CFLAGS-unwind.c += -fexceptions
CFLAGS-unwind-forcedunwind.c += -fexceptions -fasynchronous-unwind-tables

# The following three functions must be async-cancel safe.
CFLAGS-pthread_cancel.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-pthread_setcancelstate.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-pthread_setcanceltype.c += -fexceptions -fasynchronous-unwind-tables

# These are internal functions which similar functionality as setcancelstate
# and setcanceltype.
CFLAGS-cancellation.c += -fasynchronous-unwind-tables
CFLAGS-libc-cancellation.c += -fasynchronous-unwind-tables

# Calling pthread_exit() must cause the registered cancel handlers to
# be executed.  Therefore exceptions have to be thrown through this
# function.
CFLAGS-pthread_exit.c += -fexceptions

# Among others, __pthread_unwind is forwarded.  This function must handle
# exceptions.
CFLAGS-forward.c += -fexceptions

# The following are cancellation points.  Some of the functions can
# block and therefore temporarily enable asynchronous cancellation.
# Those must be compiled asynchronous unwind tables.
CFLAGS-pthread_testcancel.c += -fexceptions
CFLAGS-pthread_join.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-pthread_timedjoin.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-pthread_once.c += $(uses-callbacks) -fexceptions \
			-fasynchronous-unwind-tables
CFLAGS-pthread_cond_wait.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-sem_wait.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-sem_timedwait.c += -fexceptions -fasynchronous-unwind-tables

# These are the function wrappers we have to duplicate here.
CFLAGS-fcntl.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-fcntl64.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-lockf.c += -fexceptions
CFLAGS-pread.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-pread64.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-pwrite.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-pwrite64.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-wait.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-waitpid.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-sigwait.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-msgrcv.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-msgsnd.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-tcdrain.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-open.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-open64.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-pause.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-recv.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-send.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-accept.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-sendto.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-connect.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-recvfrom.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-recvmsg.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-sendmsg.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-close.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-read.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-write.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-nanosleep.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-sigsuspend.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-msync.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-fdatasync.c += -fexceptions -fasynchronous-unwind-tables
CFLAGS-fsync.c += -fexceptions -fasynchronous-unwind-tables

CFLAGS-pt-system.c += -fexceptions

LDLIBS-tst-once5 = -lstdc++
CFLAGS-tst-thread_local1.o = -std=gnu++11
LDLIBS-tst-thread_local1 = -lstdc++
CFLAGS-tst-thread-exit-clobber.o = -std=gnu++11
LDLIBS-tst-thread-exit-clobber = -lstdc++
CFLAGS-tst-minstack-throw.o = -std=gnu++11
LDLIBS-tst-minstack-throw = -lstdc++

tests = tst-attr1 tst-attr2 tst-attr3 tst-default-attr \
	tst-mutex1 tst-mutex2 tst-mutex3 tst-mutex4 tst-mutex5 tst-mutex6 \
	tst-mutex7 tst-mutex9 tst-mutex5a tst-mutex7a tst-mutex7robust \
	tst-mutexpi1 tst-mutexpi2 tst-mutexpi3 tst-mutexpi4 tst-mutexpi5 \
	tst-mutexpi5a tst-mutexpi6 tst-mutexpi7 tst-mutexpi7a \
	tst-mutexpi9 \
	tst-spin1 tst-spin2 tst-spin3 tst-spin4 \
	tst-cond1 tst-cond2 tst-cond3 tst-cond4 tst-cond5 tst-cond6 tst-cond7 \
	tst-cond8 tst-cond9 tst-cond10 tst-cond11 tst-cond12 tst-cond13 \
	tst-cond14 tst-cond15 tst-cond16 tst-cond17 tst-cond18 tst-cond19 \
	tst-cond20 tst-cond21 tst-cond22 tst-cond23 tst-cond24 tst-cond25 \
	tst-cond-except \
	tst-robust1 tst-robust2 tst-robust3 tst-robust4 tst-robust5 \
	tst-robust6 tst-robust7 tst-robust8 tst-robust9 \
	tst-robustpi1 tst-robustpi2 tst-robustpi3 tst-robustpi4 tst-robustpi5 \
	tst-robustpi6 tst-robustpi7 tst-robustpi8 tst-robustpi9 \
	tst-rwlock1 tst-rwlock2 tst-rwlock2a tst-rwlock2b tst-rwlock3 \
	tst-rwlock4 tst-rwlock5 tst-rwlock6 tst-rwlock7 tst-rwlock8 \
	tst-rwlock9 tst-rwlock10 tst-rwlock11 tst-rwlock12 tst-rwlock13 \
	tst-rwlock14 tst-rwlock15 tst-rwlock16 tst-rwlock17 tst-rwlock18 \
	tst-once1 tst-once2 tst-once3 tst-once4 tst-once5 \
	tst-key1 tst-key2 tst-key3 tst-key4 \
	tst-sem1 tst-sem2 tst-sem3 tst-sem4 tst-sem5 tst-sem6 tst-sem7 \
	tst-sem8 tst-sem9 tst-sem10 tst-sem14 \
	tst-sem15 tst-sem16 \
	tst-barrier1 tst-barrier2 tst-barrier3 tst-barrier4 \
	tst-align tst-align3 \
	tst-basic1 tst-basic2 tst-basic3 tst-basic4 tst-basic5 tst-basic6 \
	tst-basic7 \
	tst-kill1 tst-kill2 tst-kill3 tst-kill4 tst-kill5 tst-kill6 \
	tst-raise1 \
	tst-join1 tst-join2 tst-join3 tst-join4 tst-join5 tst-join6 tst-join7 \
	tst-detach1 \
	tst-eintr1 tst-eintr2 tst-eintr3 tst-eintr4 tst-eintr5 \
	tst-tsd1 tst-tsd2 tst-tsd3 tst-tsd4 tst-tsd5 tst-tsd6 \
	tst-tls1 tst-tls2 \
	tst-fork1 tst-fork2 tst-fork3 tst-fork4 \
	tst-atfork1 \
	tst-cancel1 tst-cancel2 tst-cancel3 tst-cancel4 tst-cancel4_1 \
	tst-cancel4_2 tst-cancel5 \
	tst-cancel6 tst-cancel7 tst-cancel8 tst-cancel9 tst-cancel10 \
	tst-cancel11 tst-cancel12 tst-cancel13 tst-cancel14 tst-cancel15 \
	tst-cancel16 tst-cancel17 tst-cancel18 tst-cancel19 tst-cancel20 \
	tst-cancel21 tst-cancel22 tst-cancel23 tst-cancel24 tst-cancel25 \
	tst-cancel26 tst-cancel27 \
	tst-cancel-self tst-cancel-self-cancelstate \
	tst-cancel-self-canceltype tst-cancel-self-testcancel \
	tst-cleanup0 tst-cleanup1 tst-cleanup2 tst-cleanup3 tst-cleanup4 \
	tst-flock1 tst-flock2 \
	tst-signal1 tst-signal2 tst-signal3 tst-signal4 tst-signal5 \
	tst-signal6 \
	tst-exec1 tst-exec2 tst-exec3 tst-exec4 tst-exec5 \
	tst-exit1 tst-exit2 tst-exit3 \
	tst-stdio1 tst-stdio2 \
	tst-stack1 tst-stack2 tst-stack3 tst-stack4 tst-pthread-getattr \
	tst-pthread-attr-affinity tst-pthread-mutexattr \
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
	tst-abstime \
	tst-vfork1 tst-vfork2 tst-vfork1x tst-vfork2x \
	tst-getpid3 \
	tst-setuid3 \
	tst-initializers1 $(addprefix tst-initializers1-,\
			    c89 gnu89 c99 gnu99 c11 gnu11) \
	tst-bad-schedattr \
	tst-thread_local1 tst-mutex-errorcheck tst-robust10 \
	tst-robust-fork tst-create-detached tst-memstream \
	tst-thread-exit-clobber tst-minstack-cancel tst-minstack-exit \
	tst-minstack-throw \
	tst-cnd-basic tst-mtx-trylock tst-cnd-broadcast \
	tst-cnd-timedwait tst-thrd-detach tst-mtx-basic tst-thrd-sleep \
	tst-mtx-recursive tst-tss-basic tst-call-once tst-mtx-timedlock

tests-internal := tst-rwlock19 tst-rwlock20 \
		  tst-sem11 tst-sem12 tst-sem13 \
		  tst-barrier5 tst-signal7 tst-mutex8 tst-mutex8-static \
		  tst-mutexpi8 tst-mutexpi8-static

xtests = tst-setuid1 tst-setuid1-static tst-setuid2 \
	tst-mutexpp1 tst-mutexpp6 tst-mutexpp10
test-srcs = tst-oddstacklimit

# Test expected to fail on most targets (except x86_64) due to bug
# 18435 - pthread_once hangs when init routine throws an exception.
test-xfail-tst-once5 = yes

# Files which must not be linked with libpthread.
tests-nolibpthread = tst-unload

gen-as-const-headers = pthread-errnos.sym \
		       unwindbuf.sym \
		       pthread-pi-defines.sym

gen-py-const-headers := nptl_lock_constants.pysym
pretty-printers := nptl-printers.py
tests-printers := test-mutexattr-printers test-mutex-printers \
		  test-condattr-printers test-cond-printers \
		  test-rwlockattr-printers test-rwlock-printers

# We must specify both CFLAGS and CPPFLAGS to override any
# compiler options the user might have provided that conflict
# with what we need e.g. user specifies CPPFLAGS with -O2 and
# we need -O0.
CFLAGS-test-mutexattr-printers.c := $(CFLAGS-printers-tests)
CFLAGS-test-mutex-printers.c := $(CFLAGS-printers-tests)
CFLAGS-test-condattr-printers.c := $(CFLAGS-printers-tests)
CFLAGS-test-cond-printers.c := $(CFLAGS-printers-tests)
CFLAGS-test-rwlockattr-printers.c := $(CFLAGS-printers-tests)
CFLAGS-test-rwlock-printers.c := $(CFLAGS-printers-tests)
CPPFLAGS-test-mutexattr-printers.c := $(CFLAGS-printers-tests)
CPPFLAGS-test-mutex-printers.c := $(CFLAGS-printers-tests)
CPPFLAGS-test-condattr-printers.c := $(CFLAGS-printers-tests)
CPPFLAGS-test-cond-printers.c := $(CFLAGS-printers-tests)
CPPFLAGS-test-rwlockattr-printers.c := $(CFLAGS-printers-tests)
CPPFLAGS-test-rwlock-printers.c := $(CFLAGS-printers-tests)

ifeq ($(build-shared),yes)
tests-printers-libs := $(shared-thread-library)
else
tests-printers-libs := $(static-thread-library)
endif

LDFLAGS-pthread.so = -Wl,--enable-new-dtags,-z,nodelete,-z,initfirst

# GCC-4.9 compiles 'sprintf(NULL, ...)' into UD2 on x86_64 without -fno-builtin
CFLAGS-tst-cleanup2.c += -fno-builtin
CFLAGS-tst-cleanupx2.c += -fno-builtin

tests += tst-cancelx2 tst-cancelx3 tst-cancelx4 tst-cancelx5 \
	 tst-cancelx6 tst-cancelx7 tst-cancelx8 tst-cancelx9 tst-cancelx10 \
	 tst-cancelx11 tst-cancelx12 tst-cancelx13 tst-cancelx14 tst-cancelx15 \
	 tst-cancelx16 tst-cancelx17 tst-cancelx18 tst-cancelx20 tst-cancelx21 \
	 tst-cleanupx0 tst-cleanupx1 tst-cleanupx2 tst-cleanupx3 tst-cleanupx4 \
	 tst-oncex3 tst-oncex4
ifeq ($(build-shared),yes)
tests += tst-atfork2 tst-tls4 tst-_res1 tst-fini1 tst-compat-forwarder
tests-internal += tst-tls3 tst-tls3-malloc tst-tls5 tst-stackguard1
tests-nolibpthread += tst-fini1
ifeq ($(have-z-execstack),yes)
tests += tst-execstack
endif
endif

modules-names = tst-atfork2mod tst-tls3mod tst-tls4moda tst-tls4modb \
		tst-tls5mod tst-tls5moda tst-tls5modb tst-tls5modc \
		tst-tls5modd tst-tls5mode tst-tls5modf tst-stack4mod \
		tst-_res1mod1 tst-_res1mod2 tst-execstack-mod tst-fini1mod \
		tst-join7mod tst-compat-forwarder-mod
extra-test-objs += $(addsuffix .os,$(strip $(modules-names))) \
		   tst-cleanup4aux.o tst-cleanupx4aux.o
test-extras += tst-cleanup4aux tst-cleanupx4aux
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
generated-dirs += $(firstword $(subst /, , $(multidir)))
crti-objs += $(multidir)/crti.o
crtn-objs += $(multidir)/crtn.o
$(objpfx)$(multidir):
	mkdir -p $@
endif
extra-objs += $(crti-objs) $(crtn-objs)
extra-objs += pt-crti.o
endif

CFLAGS-flockfile.c += $(libio-mtsafe)
CFLAGS-ftrylockfile.c += $(libio-mtsafe)
CFLAGS-funlockfile.c += $(libio-mtsafe)

link-libc-static := $(common-objpfx)libc.a $(static-gnulib) \
		    $(common-objpfx)libc.a

tests-static += tst-locale1 tst-locale2 tst-stackguard1-static \
		tst-cancel21-static tst-cancel24-static tst-cond8-static \
		tst-mutex8-static tst-mutexpi8-static tst-sem11-static \
		tst-sem12-static
tests += tst-cancel21-static tst-cancel24-static \
	 tst-cond8-static
tests-internal += tst-sem11-static tst-sem12-static tst-stackguard1-static
xtests-static += tst-setuid1-static

# These tests are linked with libc before libpthread
tests-reverse += tst-cancel5 tst-cancel23 tst-vfork1x tst-vfork2x

ifeq ($(run-built-tests),yes)
tests-special += $(objpfx)tst-stack3-mem.out $(objpfx)tst-oddstacklimit.out
ifeq ($(build-shared),yes)
tests-special += $(objpfx)tst-tls6.out $(objpfx)tst-cleanup0-cmp.out \
		 $(objpfx)tst-cancel-wrappers.out
endif
endif

ifeq (,$(CXX))
# These tests require a C++ compiler and runtime.
tests-unsupported += tst-cancel24 tst-cancel24-static tst-once5 \
  tst-thread-exit-clobber tst-minstack-throw
endif
# These tests require a C++ compiler and runtime with thread_local support.
ifneq ($(have-cxx-thread_local),yes)
tests-unsupported += tst-thread_local1
endif

include ../Rules

ifeq (yes,$(build-shared))
# Make sure these things are built in the `make lib' pass so they can be used
# to run programs during the `make others' pass.
lib-noranlib: $(addprefix $(objpfx),$(extra-objs))
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
CFLAGS-tst-cleanupx4aux.c += -fexceptions
CFLAGS-tst-oncex3.c += -fexceptions
CFLAGS-tst-oncex4.c += -fexceptions
CFLAGS-tst-align.c += $(stack-align-test-flags)
CFLAGS-tst-align3.c += $(stack-align-test-flags)
CFLAGS-tst-initializers1.c += -W -Wall -Werror
CFLAGS-tst-initializers1-< = $(CFLAGS-tst-initializers1.c) \
			     $(patsubst tst-initializers1-%.c,-std=%,$<)
CFLAGS-tst-initializers1-c89.c += $(CFLAGS-tst-initializers1-<)
CFLAGS-tst-initializers1-c99.c += $(CFLAGS-tst-initializers1-<)
CFLAGS-tst-initializers1-c11.c += $(CFLAGS-tst-initializers1-<)
CFLAGS-tst-initializers1-gnu89.c += $(CFLAGS-tst-initializers1-<)
CFLAGS-tst-initializers1-gnu99.c += $(CFLAGS-tst-initializers1-<)
CFLAGS-tst-initializers1-gnu11.c += $(CFLAGS-tst-initializers1-<)

tst-cancel7-ARGS = --command "exec $(host-test-program-cmd)"
tst-cancelx7-ARGS = $(tst-cancel7-ARGS)
tst-umask1-ARGS = $(objpfx)tst-umask1.temp

$(objpfx)tst-atfork2: $(libdl) $(shared-thread-library)
LDFLAGS-tst-atfork2 = -rdynamic
tst-atfork2-ENV = MALLOC_TRACE=$(objpfx)tst-atfork2.mtrace
$(objpfx)tst-atfork2mod.so: $(shared-thread-library)

tst-stack3-ENV = MALLOC_TRACE=$(objpfx)tst-stack3.mtrace
$(objpfx)tst-stack3-mem.out: $(objpfx)tst-stack3.out
	$(common-objpfx)malloc/mtrace $(objpfx)tst-stack3.mtrace > $@; \
	$(evaluate-test)
generated += tst-stack3-mem.out tst-stack3.mtrace

$(objpfx)tst-stack4: $(libdl) $(shared-thread-library)
tst-stack4mod.sos=$(shell for i in 0 1 2 3 4 5 6 7 8 9 10 \
				   11 12 13 14 15 16 17 18 19; do \
			    for j in 0 1 2 3 4 5 6 7 8 9 10 \
				     11 12 13 14 15 16 17 18 19; do \
			      echo $(objpfx)tst-stack4mod-$$i-$$j.so; \
			    done; done)
$(objpfx)tst-stack4.out: $(tst-stack4mod.sos)
$(tst-stack4mod.sos): $(objpfx)tst-stack4mod.so
	cp -f $< $@
clean:
	rm -f $(tst-stack4mod.sos)

$(objpfx)tst-cleanup4: $(objpfx)tst-cleanup4aux.o $(shared-thread-library)
$(objpfx)tst-cleanupx4: $(objpfx)tst-cleanupx4aux.o $(shared-thread-library)

$(objpfx)tst-tls3: $(libdl) $(shared-thread-library)
LDFLAGS-tst-tls3 = -rdynamic
$(objpfx)tst-tls3.out: $(objpfx)tst-tls3mod.so
$(objpfx)tst-tls3mod.so: $(shared-thread-library)

$(objpfx)tst-tls3-malloc: $(libdl) $(shared-thread-library)
LDFLAGS-tst-tls3-malloc = -rdynamic
$(objpfx)tst-tls3-malloc.out: $(objpfx)tst-tls3mod.so

$(objpfx)tst-tls4: $(libdl) $(shared-thread-library)
$(objpfx)tst-tls4.out: $(objpfx)tst-tls4moda.so $(objpfx)tst-tls4modb.so

$(objpfx)tst-tls5: $(objpfx)tst-tls5mod.so $(shared-thread-library)
LDFLAGS-tst-tls5 = $(no-as-needed)
LDFLAGS-tst-tls5mod.so = -Wl,-soname,tst-tls5mod.so

ifeq ($(build-shared),yes)
$(objpfx)tst-tls6.out: tst-tls6.sh $(objpfx)tst-tls5 \
		       $(objpfx)tst-tls5moda.so $(objpfx)tst-tls5modb.so \
		       $(objpfx)tst-tls5modc.so $(objpfx)tst-tls5modd.so \
		       $(objpfx)tst-tls5mode.so $(objpfx)tst-tls5modf.so
	$(BASH) $< $(common-objpfx) '$(test-via-rtld-prefix)' \
	  '$(test-wrapper-env)' '$(run-program-env)' > $@; \
	$(evaluate-test)
endif

$(objpfx)tst-join7: $(libdl) $(shared-thread-library)
$(objpfx)tst-join7.out: $(objpfx)tst-join7mod.so
$(objpfx)tst-join7mod.so: $(shared-thread-library)
LDFLAGS-tst-join7mod.so = -Wl,-soname,tst-join7mod.so

$(objpfx)tst-dlsym1: $(libdl) $(shared-thread-library)

$(objpfx)tst-fini1: $(shared-thread-library) $(objpfx)tst-fini1mod.so

ifeq (yes,$(build-shared))
librt = $(common-objpfx)rt/librt.so
else
librt = $(common-objpfx)rt/librt.a
endif

# `make check' sometimes triggers a rebuild of librt.so using this Makefile,
# which ignores librt's dependence on libpthread
$(common-objpfx)rt/librt.so: $(shared-thread-library)

$(objpfx)tst-cancel17: $(librt)
$(objpfx)tst-cancelx17: $(librt)
$(objpfx)tst-_res1mod2.so: $(objpfx)tst-_res1mod1.so
LDFLAGS-tst-_res1mod1.so = -Wl,-soname,tst-_res1mod1.so
LDFLAGS-tst-_res1mod2.so = -Wl,-soname,tst-_res1mod2.so
$(objpfx)tst-_res1: $(objpfx)tst-_res1mod1.so $(objpfx)tst-_res1mod2.so \
		    $(shared-thread-library)

LDLIBS-tst-cancel24 = $(no-as-needed) -lstdc++
LDLIBS-tst-cancel24-static = $(LDLIBS-tst-cancel24)

extra-B-pthread.so = -B$(common-objpfx)nptl/
$(objpfx)libpthread.so: $(addprefix $(objpfx),$(crti-objs) $(crtn-objs))
$(objpfx)libpthread.so: +preinit += $(addprefix $(objpfx),$(crti-objs))
$(objpfx)libpthread.so: +postinit += $(addprefix $(objpfx),$(crtn-objs))

# Make sure we link with the thread library.
ifeq ($(build-shared),yes)
$(addprefix $(objpfx), \
  $(filter-out $(tests-static) $(xtests-static) $(tests-reverse) \
    $(tests-nolibpthread), \
    $(tests) $(tests-internal) $(xtests) $(test-srcs))): \
	$(objpfx)libpthread.so
$(objpfx)tst-unload: $(libdl)
# $(objpfx)../libc.so is used instead of $(common-objpfx)libc.so,
# since otherwise libpthread.so comes before libc.so when linking.
$(addprefix $(objpfx), $(tests-reverse)): \
  $(objpfx)../libc.so $(objpfx)libpthread.so
$(objpfx)../libc.so: $(common-objpfx)libc.so ;
$(addprefix $(objpfx),$(tests-static) $(xtests-static)): $(objpfx)libpthread.a

$(objpfx)tst-atfork2.out: $(objpfx)tst-atfork2mod.so
else
$(addprefix $(objpfx),$(tests) $(test-srcs)): $(objpfx)libpthread.a
endif

ifeq ($(build-shared),yes)

$(objpfx)tst-cleanup0.out: /dev/null $(objpfx)tst-cleanup0
	$(make-test-out) > $@ 2>&1; \
	$(evaluate-test)

$(objpfx)tst-cleanup0-cmp.out: tst-cleanup0.expect $(objpfx)tst-cleanup0.out
	cmp $^ > $@; \
	$(evaluate-test)

$(objpfx)crti.o: $(objpfx)pt-crti.o
	ln -f $< $@

ifneq ($(multidir),.)
$(objpfx)$(multidir)/crti.o: $(objpfx)crti.o $(objpfx)$(multidir)/
	ln -f $< $@

$(objpfx)$(multidir)/crtn.o: $(objpfx)crtn.o $(objpfx)$(multidir)/
	ln -f $< $@
endif

generated += multidir.mk tst-atfork2.mtrace tst-cancel-wrappers.out \
	     tst-tls6.out

generated += $(objpfx)tst-atfork2.mtrace \
	     $(addsuffix .so,$(strip $(modules-names)))

# Give libpthread.so an entry point and make it directly runnable itself.
LDFLAGS-pthread.so += -e __nptl_main
# pt-interp.c exists just to get the runtime linker path into libpthread.so.
$(objpfx)pt-interp.os: $(common-objpfx)runtime-linker.h
endif

ifeq ($(run-built-tests),yes)
ifeq (yes,$(build-shared))
$(objpfx)tst-cancel-wrappers.out: tst-cancel-wrappers.sh
	$(SHELL) $< '$(NM)' \
		    $(common-objpfx)libc_pic.a \
		    $(common-objpfx)libc.a \
		    $(objpfx)libpthread_pic.a \
		    $(objpfx)libpthread.a > $@; \
	$(evaluate-test)
endif
endif

tst-exec4-ARGS = $(host-test-program-cmd)

$(objpfx)tst-execstack: $(libdl)
$(objpfx)tst-execstack.out: $(objpfx)tst-execstack-mod.so
LDFLAGS-tst-execstack = -Wl,-z,noexecstack
CFLAGS-tst-execstack-mod.c += -Wno-trampolines

$(objpfx)tst-fini1mod.so: $(shared-thread-library)

tst-stackguard1-ARGS = --command "$(host-test-program-cmd) --child"
tst-stackguard1-static-ARGS = --command "$(objpfx)tst-stackguard1-static --child"

ifeq ($(run-built-tests),yes)
$(objpfx)tst-oddstacklimit.out: $(objpfx)tst-oddstacklimit $(objpfx)tst-basic1
	$(test-program-prefix) $< --command '$(host-test-program-cmd)' > $@; \
	$(evaluate-test)
endif

$(objpfx)tst-compat-forwarder: $(objpfx)tst-compat-forwarder-mod.so

# The tests here better do not run in parallel
ifneq ($(filter %tests,$(MAKECMDGOALS)),)
.NOTPARALLEL:
endif
