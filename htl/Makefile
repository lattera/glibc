#
#   Copyright (C) 1994-2018 Free Software Foundation, Inc.
#
#   The GNU C Library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License as published by the Free Software Foundation; either
#   version 2.1 of the License, or (at your option) any later version.

#   The GNU C Library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.

#   You should have received a copy of the GNU Lesser General Public
#   License along with the GNU C Library; if not, see
#   <http://www.gnu.org/licenses/>.

subdir := htl

srcdir = .

MICROKERNEL := mach
SYSDEPS := lockfile

LCLHDRS :=

libpthread-routines := pt-attr pt-attr-destroy pt-attr-getdetachstate	    \
	pt-attr-getguardsize pt-attr-getinheritsched			    \
	pt-attr-getschedparam pt-attr-getschedpolicy pt-attr-getscope	    \
	pt-attr-getstack pt-attr-getstackaddr pt-attr-getstacksize	    \
	pt-attr-init pt-attr-setdetachstate pt-attr-setguardsize	    \
	pt-attr-setinheritsched pt-attr-setschedparam			    \
	pt-attr-setschedpolicy pt-attr-setscope pt-attr-setstack	    \
	pt-attr-setstackaddr pt-attr-setstacksize			    \
									    \
	pt-barrier-destroy pt-barrier-init pt-barrier-wait		    \
	pt-barrier pt-barrierattr-destroy pt-barrierattr-init		    \
	pt-barrierattr-getpshared pt-barrierattr-setpshared		    \
									    \
	pt-destroy-specific pt-init-specific				    \
	pt-key-create pt-key-delete					    \
	pt-getspecific pt-setspecific					    \
									    \
	pt-once								    \
									    \
	pt-alloc							    \
	pt-create							    \
	pt-getattr							    \
	pt-equal							    \
	pt-dealloc							    \
	pt-detach							    \
	pt-exit								    \
	pt-initialize							    \
	pt-join								    \
	pt-self								    \
	pt-sigmask							    \
	pt-spin-inlines							    \
	pt-cleanup							    \
	pt-setcancelstate						    \
	pt-setcanceltype						    \
	pt-testcancel							    \
	pt-cancel							    \
									    \
	pt-mutexattr							    \
	pt-mutexattr-destroy pt-mutexattr-init				    \
	pt-mutexattr-getprioceiling pt-mutexattr-getprotocol		    \
	pt-mutexattr-getpshared pt-mutexattr-gettype			    \
	pt-mutexattr-setprioceiling pt-mutexattr-setprotocol		    \
	pt-mutexattr-setpshared pt-mutexattr-settype			    \
	pt-mutexattr-getrobust pt-mutexattr-setrobust			    \
									    \
	pt-mutex-init pt-mutex-destroy					    \
	pt-mutex-lock pt-mutex-trylock pt-mutex-timedlock		    \
	pt-mutex-unlock							    \
	pt-mutex-transfer-np						    \
	pt-mutex-getprioceiling pt-mutex-setprioceiling			    \
	pt-mutex-consistent 						    \
									    \
	pt-rwlock-attr							    \
	pt-rwlockattr-init pt-rwlockattr-destroy			    \
	pt-rwlockattr-getpshared pt-rwlockattr-setpshared		    \
									    \
	pt-rwlock-init pt-rwlock-destroy				    \
	pt-rwlock-rdlock pt-rwlock-tryrdlock				    \
	pt-rwlock-trywrlock pt-rwlock-wrlock				    \
	pt-rwlock-timedrdlock pt-rwlock-timedwrlock			    \
	pt-rwlock-unlock						    \
									    \
	pt-cond								    \
	pt-condattr-init pt-condattr-destroy				    \
	pt-condattr-getclock pt-condattr-getpshared			    \
	pt-condattr-setclock pt-condattr-setpshared			    \
									    \
	pt-cond-destroy pt-cond-init					    \
	pt-cond-brdcast							    \
	pt-cond-signal							    \
	pt-cond-wait							    \
	pt-cond-timedwait						    \
	pt-hurd-cond-wait						    \
	pt-hurd-cond-timedwait						    \
									    \
	pt-stack-alloc							    \
	pt-thread-alloc							    \
	pt-thread-start							    \
	pt-thread-terminate						    \
	pt-startup							    \
									    \
	pt-getconcurrency pt-setconcurrency				    \
									    \
	pt-block							    \
	pt-timedblock							    \
	pt-wakeup							    \
	pt-docancel							    \
	pt-sysdep							    \
	pt-setup							    \
	pt-machdep							    \
	pt-spin								    \
									    \
	pt-sigstate-init						    \
	pt-sigstate-destroy						    \
	pt-sigstate							    \
									    \
	pt-atfork							    \
	old_pt-atfork							    \
	pt-kill								    \
	pt-getcpuclockid						    \
									    \
	pt-getschedparam pt-setschedparam pt-setschedprio		    \
	pt-yield							    \
									    \
	sem-close sem-destroy sem-getvalue sem-init sem-open		    \
	sem-post sem-timedwait sem-trywait sem-unlink			    \
	sem-wait							    \
									    \
	shm-directory							    \
									    \
	cthreads-compat							    \
	$(SYSDEPS)

libpthread-static-only-routines = pt-atfork

headers :=				\
              pthread.h				\
	      semaphore.h			\
						\
              bits/pthread.h			\
              bits/pthread-np.h			\
              bits/pthreadtypes.h		\
              bits/pthreadtypes-arch.h		\
              bits/thread-shared-types.h	\
              bits/types/struct___pthread_mutex.h	\
              bits/types/struct___pthread_cond.h	\
              bits/types/struct___pthread_condattr.h	\
              bits/types/__pthread_spinlock_t.h	\
              bits/spin-lock-inline.h		\
              bits/cancelation.h		\
              bits/types/struct___pthread_attr.h	\
              bits/types/struct___pthread_barrierattr.h	\
              bits/types/struct___pthread_barrier.h	\
              bits/types/__pthread_key.h		\
              bits/types/struct___pthread_once.h	\
              bits/types/struct___pthread_mutexattr.h	\
              bits/types/struct___pthread_rwlock.h	\
              bits/types/struct___pthread_rwlockattr.h	\
	      bits/semaphore.h

distribute :=

routines := forward libc_pthread_init alloca_cutoff
shared-only-routines = forward

extra-libs := libpthread
extra-libs-others := $(extra-libs)
install-lib := libpthread.so

include ../Makeconfig

CFLAGS-lockfile.c = -D_IO_MTSAFE_IO

all: # Make this the default target; it will be defined in Rules.

subdir_install: $(inst_libdir)/libpthread2.a

# XXX: If $(inst_libdir)/libpthread2.a is installed and
# $(inst_libdir)/libpthread is not, we can have some issues.
.PHONY: $(inst_libdir)/libpthread.a $(inst_libdir)/libpthread_pic.a

# XXX: These rules are a hack.  But it is better than messing with
# ../Makeconf at the moment.  Note that the linker scripts
# $(srcdir)/libpthread.a and $(srcdir)/libpthread_pic.a get overwritten
# when building in $(srcdir) and not a seperate build directory.
$(inst_libdir)/libpthread2.a: $(inst_libdir)/libpthread.a
	mv $< $@
	$(INSTALL_DATA) $(srcdir)/libpthread.a $<

$(inst_libdir)/libpthread2_pic.a: $(inst_libdir)/libpthread_pic.a
	mv $< $@
	$(INSTALL_DATA) $(srcdir)/libpthread_pic.a $<

libc-link.so = $(common-objpfx)libc.so

extra-B-pthread.so = -B$(common-objpfx)htl/

include ../Rules

ifeq (yes,$(build-shared))
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

$(addprefix $(objpfx), \
  $(filter-out $(tests-static) $(xtests-static) $(tests-reverse) \
    $(tests-nolibpthread), \
    $(tests) $(xtests) $(test-srcs))): $(objpfx)libpthread.so \
				       $(objpfx)libpthread_nonshared.a
endif

generated += libpthread_nonshared.a
