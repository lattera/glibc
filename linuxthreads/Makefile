# Copyright (C) 1996-2003, 2004 Free Software Foundation, Inc.
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
#	Sub-makefile for linuxthreads portion of the library.
#
subdir	:= linuxthreads

all: # Make this the default target; it will be defined in Rules.

linuxthreads-version := $(shell sed -n 's/^.*$(subdir)-\([0-9.]*\).*$$/\1/p' \
				    Banner)
libpthread-abi-frozen := GLIBC_2.3.2

headers := pthread.h semaphore.h
distribute := internals.h queue.h restart.h spinlock.h smp.h tst-signal.sh \
	      tst-cancel-wrappers.sh libc-tsd.c

routines := forward alloca_cutoff libc-cancellation libc_pthread_init
shared-only-routines = forward

extra-libs := libpthread
extra-libs-others := $(extra-libs)
install-lib-ldscripts := libpthread.so

libpthread-routines := attr cancel condvar join manager mutex ptfork \
		       ptlongjmp pthread pt-sigsuspend signals specific errno \
		       lockfile semaphore spinlock rwlock pt-machine \
		       oldsemaphore events getcpuclockid pspinlock barrier \
		       ptclock_gettime ptclock_settime sighandler \
		       pthandles libc-tls-loc pt-allocrtsig \
		       ptw-write ptw-read ptw-close ptw-fcntl ptw-accept \
		       ptw-connect ptw-recv ptw-recvfrom ptw-recvmsg \
		       ptw-send ptw-sendmsg ptw-sendto ptw-fsync ptw-lseek \
		       ptw-lseek64 ptw-llseek ptw-msync ptw-nanosleep \
		       ptw-open ptw-open64 ptw-pause ptw-pread ptw-pread64 \
		       ptw-pwrite ptw-pwrite64 ptw-tcdrain ptw-wait \
		       ptw-waitpid pt-system old_pthread_atfork pthread_atfork \
		       ptcleanup
# Don't generate deps for calls with no sources.  See sysdeps/unix/Makefile.
omit-deps = $(unix-syscalls:%=ptw-%)

libpthread-shared-only-routines = pt-allocrtsig
libpthread-static-only-routines = pthread_atfork

libpthread-nonshared = pthread_atfork

CFLAGS-pthread_atfork.c = -DNOT_IN_libc

nodelete-yes = -Wl,--enable-new-dtags,-z,nodelete
initfirst-yes = -Wl,--enable-new-dtags,-z,initfirst
LDFLAGS-pthread.so = $(nodelete-$(have-z-nodelete)) \
		     $(initfirst-$(have-z-initfirst))

vpath %.c Examples

tst-cancel-ARGS = "$(objpfx)"
CFLAGS-tst-cancel.c = -fno-inline -fno-inline-functions

include ../Makeconfig

ifeq ($(build-shared),yes)
others: $(objpfx)libpthread_nonshared.a
endif

$(objpfx)libpthread_nonshared.a: $(addprefix $(objpfx),$(addsuffix .os,$(libpthread-nonshared)))
	$(AR) $(ARFLAGS) $@ $^

ifeq ($(build-shared),yes)

# Set the `multidir' variable by grabbing the variable from the compiler.
# We do it once and save the result in a generated makefile.
-include $(objpfx)multidir.mk
$(objpfx)multidir.mk: $(common-objpfx)config.make
	$(make-target-directory)
	dir=`$(CC) $(CFLAGS) $(CPPFLAGS) -print-multi-directory`; \
	echo "multidir := $$dir" > $@T
	mv -f $@T $@
generated += multidir.mk

crti-objs := crti.o
crtn-objs := crtn.o
ifneq (,$(patsubst .,,$(multidir)))
generated-dirs := $(firstword $(subst /, , $(multidir)))
crti-objs += $(multidir)/crti.o
crtn-objs += $(multidir)/crtn.o
omit-deps += $(multidir)/crti $(multidir)/crtn
endif
extra-objs += $(crti-objs) $(crtn-objs)
omit-deps += crti crtn

CFLAGS-pt-initfini.s = -g0 -fPIC -fno-inline-functions $(fno-unit-at-a-time)
endif

librt-tests = ex10 ex11 tst-clock1
tests = ex1 ex2 ex3 ex4 ex5 ex6 ex7 ex8 ex9 $(librt-tests) ex12 ex13 joinrace \
	tststack $(tests-nodelete-$(have-z-nodelete)) ecmutex ex14 ex15 ex16 \
	ex17 ex18 tst-cancel tst-context bug-sleep \
	tst-cancel1 tst-cancel2 tst-cancel3 tst-cancel4 tst-cancel5 \
	tst-cancel6 tst-cancel7 tst-cancel8 tst-popen tst-popen2 tst-attr1 \
	tst-stack1
test-srcs = tst-signal
# These tests are linked with libc before libpthread
tests-reverse += tst-cancel5

ifeq ($(build-static),yes)
tests += tststatic tst-static-locale tst-cancel-static
tests-static += tststatic tst-static-locale tst-cancel-static
endif

ifeq (yes,$(build-shared))
tests-nodelete-yes = unload
tests += tst-tls1 tst-_res1
endif

modules-names = tst-_res1mod1 tst-_res1mod2 \
		tst-tls1mod tst-tls1moda tst-tls1modb tst-tls1modc \
		tst-tls1modd tst-tls1mode tst-tls1modf
extra-objs += $(addsuffix .os,$(strip $(modules-names)))
generated += $(addsuffix .so,$(strip $(modules-names)))
test-extras += $(modules-names)
test-modules = $(addprefix $(objpfx),$(addsuffix .so,$(modules-names)))

tst-tls1mod.so-no-z-defs = yes
tst-tls1moda.so-no-z-defs = yes
tst-tls1modb.so-no-z-defs = yes
tst-tls1modc.so-no-z-defs = yes
tst-tls1modd.so-no-z-defs = yes
tst-tls1mode.so-no-z-defs = yes
tst-tls1modf.so-no-z-defs = yes

$(test-modules): $(objpfx)%.so: $(objpfx)%.os $(common-objpfx)shlib.lds
	$(build-module)

ifeq ($(build-shared),yes)
# Build all the modules even when not actually running test programs.
tests: $(test-modules)
endif

# What we install as libpthread.so for programs to link against is in fact a
# link script.  It contains references for the various libraries we need.
# The libpthread.so object is not complete since some functions are only defined
# in libpthread_nonshared.a.
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


extra-B-pthread.so = -B$(common-objpfx)linuxthreads/
$(objpfx)libpthread.so: $(addprefix $(objpfx),$(crti-objs) $(crtn-objs))
$(objpfx)libpthread.so: +preinit += $(addprefix $(objpfx),$(crti-objs))
$(objpfx)libpthread.so: +postinit += $(addprefix $(objpfx),$(crtn-objs))

znodelete-yes = -DHAVE_Z_NODELETE
CFLAGS-mutex.c += -D__NO_WEAK_PTHREAD_ALIASES
CFLAGS-specific.c += -D__NO_WEAK_PTHREAD_ALIASES
CFLAGS-pthread.c += -D__NO_WEAK_PTHREAD_ALIASES $(znodelete-$(have-z-nodelete))
CFLAGS-ptfork.c += -D__NO_WEAK_PTHREAD_ALIASES
CFLAGS-cancel.c += -D__NO_WEAK_PTHREAD_ALIASES -D_RPC_THREAD_SAFE_
CFLAGS-unload.c += -DPREFIX=\"$(objpfx)\"
CFLAGS-mutex.c += $(uses-callbacks)
CFLAGS-sighandler.c += $(uses-callbacks)

ifeq (yes,$(versioning))
-include $(common-objpfx)tls.make
libc-ok-for-link = $(use-thread)
else
libc-ok-for-link = yes
endif

ifeq (no,$(libc-ok-for-link))
# These hacks are necessary to let us link against a libc.so that exports
# the symbols _errno, _h_errno, and _res.  Those symbols are accessible
# in libc at runtime (dynamic linkable), but are not exported at link time
# so that applications cannot link against them.  However, libpthread.so
# needs to link against them for its __errno_location et al functions to
# find the locations that libc's symbols resolve to.  We cannot do this
# with aliases in libc.so(GLIBC_PRIVATE), because we need to refer to an
# executable's symbols when it defines them with copy relocs.
libc-link.so = $(objpfx)libc.so

$(objpfx)libc_pic_lite.a: $(common-objpfx)libc_pic.a
	cp $< $@T
	$(AR) d $@T errno.os herrno.os res_libc.os
	mv -f $@T $@

extra-objs += libc-tsd.os
$(objpfx)libc_pic_lite.os: $(objpfx)libc_pic_lite.a $(objpfx)libc-tsd.os
	$(LINK.o) -nostdlib -nostartfiles -r -o $@ \
		  $(LDFLAGS-c_pic.os) -Wl,-d -Wl,--whole-archive $^

# This trick leaves errno and h_errno undefined.
libc.so-no-z-defs = yes

$(objpfx)libc.so: $(elfobjdir)/soinit.os \
		  $(objpfx)libc_pic_lite.os \
		  $(elfobjdir)/sofini.os \
		  $(elfobjdir)/interp.os $(elfobjdir)/ld.so
	$(build-shlib)

generated += libc_pic_lite.a libc_pic_lite.os libc.so libc-tsd.os
else
libc-link.so = $(common-objpfx)libc.so
endif

include ../Rules

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
# Depend on ld.so too to get proper versions of ld.so symbols.
$(objpfx)libpthread.so: $(libc-link.so) $(common-objpfx)libc_nonshared.a \
			$(if $(filter yes,$(elf)), $(elfobjdir)/ld.so)

# Make sure we link with the thread library.
ifeq ($(build-shared),yes)
$(addprefix $(objpfx), \
  $(filter-out $(tests-static) $(tests-reverse) unload, \
    $(tests) $(test-srcs))): $(objpfx)libpthread.so \
			     $(objpfx)libpthread_nonshared.a
# $(objpfx)../libc.so is used instead of $(common-objpfx)libc.so,
# since otherwise libpthread.so comes before libc.so when linking.
$(addprefix $(objpfx), $(tests-reverse)): \
  $(objpfx)../libc.so $(objpfx)libpthread.so \
  $(objpfx)libpthread_nonshared.a
$(objpfx)../libc.so: $(common-objpfx)libc.so ;
$(addprefix $(objpfx),$(librt-tests)): $(common-objpfx)rt/librt.so
$(objpfx)unload: $(common-objpfx)dlfcn/libdl.so
$(objpfx)unload.out: $(objpfx)libpthread.so $(objpfx)libpthread_nonshared.a
else
$(addprefix $(objpfx),$(tests) $(test-srcs)): $(objpfx)libpthread.a
$(addprefix $(objpfx),$(librt-tests)): $(common-objpfx)rt/librt.a
endif
ifeq ($(build-bounded),yes)
$(tests:%=$(objpfx)%-bp): $(objpfx)libpthread_b.a
$(librt-tests:%=$(objpfx)%-bp): $(common-objpfx)rt/librt_b.a
endif

ifeq ($(build-static),yes)
$(addprefix $(objpfx), $(tests-static)): $(objpfx)libpthread.a
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
$(objpfx)$(multidir):
	@mkdir -p $(objpfx)$(multidir)

$(objpfx)$(multidir)/crti.o: $(objpfx)crti.o $(objpfx)$(multidir)
	ln -f $< $@

$(objpfx)$(multidir)/crtn.o: $(objpfx)crtn.o $(objpfx)$(multidir)
	ln -f $< $@
endif

generated += crti.S crtn.S defs.h pt-initfini.s libpthread_nonshared.a
endif

ifeq (yes,$(build-static-nss))
otherlibs += $(nssobjdir)/libnss_files.a $(resolvobjdir)/libnss_dns.a \
             $(resolvobjdir)/libresolv.a
endif

ifeq (yes,$(build-shared))
$(objpfx)tst-_res1mod2.so: $(objpfx)tst-_res1mod1.so
$(objpfx)tst-_res1: $(objpfx)tst-_res1mod2.so $(shared-thread-library)

$(objpfx)tst-tls1: $(objpfx)tst-tls1mod.so $(shared-thread-library)

tests: $(objpfx)tst-tls2.out
$(objpfx)tst-tls2.out: tst-tls2.sh $(objpfx)tst-tls1 \
		       $(objpfx)tst-tls1moda.so $(objpfx)tst-tls1modb.so \
		       $(objpfx)tst-tls1modc.so $(objpfx)tst-tls1modd.so \
		       $(objpfx)tst-tls1mode.so $(objpfx)tst-tls1modf.so
	$(SHELL) -e tst-tls2.sh $(common-objpfx) $(elf-objpfx) \
		    $(rtld-installed-name)
generated += tst-tls2.out
endif

ifeq (no,$(cross-compiling))
ifeq (yes,$(build-shared))
tests: $(objpfx)tst-signal.out $(objpfx)tst-cancel-wrappers.out
$(objpfx)tst-signal.out: tst-signal.sh $(objpfx)tst-signal
	$(SHELL) -e $< $(common-objpfx) > $@
$(objpfx)tst-cancel-wrappers.out: tst-cancel-wrappers.sh
	$(SHELL) $< $(common-objpfx)/libc_pic.a \
		    $(common-objpfx)/libc.a \
		    $(objpfx)/libpthread_pic.a \
		    $(objpfx)/libpthread.a > $@
generated += tst-signal.out tst-cancel-wrappers.out
endif
endif
