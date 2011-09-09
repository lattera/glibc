# Copyright (C) 1991-2002,2003,2004,2005,2006,2008,2009,2011
#	Free Software Foundation, Inc.
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
#	Master Makefile for the GNU C library
#
ifneq (,)
This makefile requires GNU Make.
endif

include Makeconfig


# This is the default target; it makes everything except the tests.
.PHONY: all
all: lib others

ifneq ($(AUTOCONF),no)

define autoconf-it
@-rm -f $@.new
$(AUTOCONF) $(ACFLAGS) $< > $@.new
chmod a-w$(patsubst %,$(comma)a+x,$(filter .,$(@D))) $@.new
mv -f $@.new $@
endef

configure: configure.in aclocal.m4; $(autoconf-it)
%/configure: %/configure.in aclocal.m4; $(autoconf-it)
%/preconfigure: %/preconfigure.in aclocal.m4; $(autoconf-it)

endif # $(AUTOCONF) = no


# We don't want to run anything here in parallel.
.NOTPARALLEL:

# These are the targets that are made by making them in each subdirectory.
+subdir_targets	:= subdir_lib objects objs others subdir_mostlyclean	\
		   subdir_clean subdir_distclean subdir_realclean	\
		   tests xtests subdir_lint.out				\
		   subdir_update-abi subdir_check-abi			\
		   subdir_echo-headers					\
		   subdir_install					\
		   subdir_objs subdir_stubs subdir_testclean		\
		   $(addprefix install-, no-libc.a bin lib data headers others)

headers := limits.h values.h features.h gnu-versions.h bits/libc-lock.h \
	   bits/xopen_lim.h gnu/libc-version.h

echo-headers: subdir_echo-headers

# The headers are in the include directory.
subdir-dirs = include
vpath %.h $(subdir-dirs)

# What to install.
install-others = $(inst_includedir)/gnu/stubs.h
install-bin-script =

ifeq (yes,$(build-shared))
headers += gnu/lib-names.h
endif

include Makerules

ifeq ($(build-programs),yes)
others: $(addprefix $(objpfx),$(install-bin-script))
endif

# Install from subdirectories too.
install: subdir_install

# Explicit dependency so that `make install-headers' works
install-headers: install-headers-nosubdir

# Make sure that the dynamic linker is installed before libc.
$(inst_slibdir)/libc-$(version).so: elf/ldso_install

.PHONY: elf/ldso_install
elf/ldso_install:
	$(MAKE) -C $(@D) $(@F)

# Create links for shared libraries using the `ldconfig' program if possible.
# Ignore the error if we cannot update /etc/ld.so.cache.
ifeq (no,$(cross-compiling))
ifeq (yes,$(build-shared))
install: install-symbolic-link
.PHONY: install-symbolic-link
install-symbolic-link: subdir_install
	$(symbolic-link-prog) $(symbolic-link-list)
	rm -f $(symbolic-link-list)

install:
	-test ! -x $(common-objpfx)elf/ldconfig || LC_ALL=C LANGUAGE=C \
	  $(common-objpfx)elf/ldconfig $(addprefix -r ,$(install_root)) \
				       $(slibdir) $(libdir)
ifneq (no,$(PERL))
ifeq (/usr,$(prefix))
ifeq (,$(install_root))
	CC="$(CC)" $(PERL) scripts/test-installation.pl $(common-objpfx)
endif
endif
endif
endif
endif

# Build subdirectory lib objects.
lib-noranlib: subdir_lib

ifeq (yes,$(build-shared))
# Build the shared object from the PIC object library.
lib: $(common-objpfx)libc.so

lib: $(common-objpfx)linkobj/libc.so

$(common-objpfx)linkobj/libc.so: $(elfobjdir)/soinit.os \
				 $(common-objpfx)linkobj/libc_pic.a \
				 $(elfobjdir)/sofini.os \
				 $(elfobjdir)/interp.os \
				 $(elfobjdir)/ld.so \
				 $(shlib-lds)
	$(build-shlib)

$(common-objpfx)linkobj/libc_pic.a: $(common-objpfx)libc_pic.a \
				    $(common-objpfx)sunrpc/librpc_compat_pic.a
	$(..)./scripts/mkinstalldirs $(common-objpfx)linkobj
	(cd $(common-objpfx)linkobj; \
	 $(AR) x ../libc_pic.a; \
	 rm $$($(AR) t ../sunrpc/librpc_compat_pic.a | sed 's/^compat-//'); \
	 $(AR) x ../sunrpc/librpc_compat_pic.a; \
	 $(AR) cr libc_pic.a *.os; \
	 rm *.os)
endif


# This is a handy script for running any dynamically linked program against
# the current libc build for testing.
$(common-objpfx)testrun.sh: $(common-objpfx)config.make \
			    $(..)Makeconfig $(..)Makefile
	(echo '#!/bin/sh'; \
	 echo 'builddir=`dirname "$$0"`'; \
	 echo 'GCONV_PATH="$${builddir}/iconvdata" \'; \
	 echo 'exec $(subst $(common-objdir),"$${builddir}",\
			    $(run-program-prefix)) $${1+"$$@"}'; \
	) > $@T
	chmod a+x $@T
	mv -f $@T $@
postclean-generated += testrun.sh

others: $(common-objpfx)testrun.sh

# Makerules creates a file `stubs' in each subdirectory, which
# contains `#define __stub_FUNCTION' for each function defined in that
# directory which is a stub.
# Here we paste all of these together into <gnu/stubs.h>.

subdir-stubs := $(foreach dir,$(subdirs),$(common-objpfx)$(dir)/stubs)

ifeq ($(biarch),no)
installed-stubs = $(inst_includedir)/gnu/stubs.h
else
installed-stubs = $(inst_includedir)/gnu/stubs-$(biarch).h

$(inst_includedir)/gnu/stubs.h: include/stubs-biarch.h $(+force)
	$(make-target-directory)
	$(INSTALL_DATA) $< $@

install-others-nosubdir: $(installed-stubs)
endif


# Since stubs.h is never needed when building the library, we simplify the
# hairy installation process by producing it in place only as the last part
# of the top-level `make install'.  It depends on subdir_install, which
# iterates over all the subdirs; subdir_install in each subdir depends on
# the subdir's stubs file.  Having more direct dependencies would result in
# extra iterations over the list for subdirs and many recursive makes.
$(installed-stubs): include/stubs-prologue.h subdir_install
	$(make-target-directory)
	@rm -f $(objpfx)stubs.h
	(sed '/^@/d' $<; LC_ALL=C sort $(subdir-stubs)) > $(objpfx)stubs.h
	if test -r $@ && cmp -s $(objpfx)stubs.h $@; \
	then echo 'stubs.h unchanged'; \
	else $(INSTALL_DATA) $(objpfx)stubs.h $@; fi
	rm -f $(objpfx)stubs.h

# This makes the Info or DVI file of the documentation from the Texinfo source.
.PHONY: info dvi pdf html
info dvi pdf html:
	$(MAKE) $(PARALLELMFLAGS) -C manual $@

# This makes all the subdirectory targets.

# For each target, make it depend on DIR/target for each subdirectory DIR.
$(+subdir_targets): %: $(addsuffix /%,$(subdirs))

# Compute a list of all those targets.
all-subdirs-targets := $(foreach dir,$(subdirs),\
				 $(addprefix $(dir)/,$(+subdir_targets)))

# The action for each of those is to cd into the directory and make the
# target there.
$(all-subdirs-targets):
	$(MAKE) $(PARALLELMFLAGS) $(subdir-target-args) $(@F)

define subdir-target-args
subdir=$(@D)$(if $($(@D)-srcdir),\
-C $($(@D)-srcdir) ..=`pwd`/,\
-C $(@D) ..=../)
endef

.PHONY: $(+subdir_targets) $(all-subdirs-targets)

# Targets to clean things up to various degrees.

.PHONY: clean realclean distclean distclean-1 parent-clean parent-mostlyclean \
	tests-clean

# Subroutines of all cleaning targets.
parent-mostlyclean: common-mostlyclean # common-mostlyclean is in Makerules.
	-rm -f $(foreach o,$(object-suffixes-for-libc),\
		   $(common-objpfx)$(patsubst %,$(libtype$o),c)) \
	       $(addprefix $(objpfx),$(install-lib))
parent-clean: parent-mostlyclean common-clean

postclean = $(addprefix $(common-objpfx),$(postclean-generated)) \
	    $(addprefix $(objpfx),sysd-dirs sysd-rules) \
	    $(addprefix $(objpfx),sysd-sorted soversions.mk soversions.i)

clean: parent-clean
# This is done this way rather than having `subdir_clean' be a
# dependency of this target so that libc.a will be removed before the
# subdirectories are dealt with and so they won't try to remove object
# files from it when it's going to be removed anyway.
	@$(MAKE) subdir_clean no_deps=t
	-rm -f $(postclean)
mostlyclean: parent-mostlyclean
	@$(MAKE) subdir_mostlyclean no_deps=t
	-rm -f $(postclean)

tests-clean:
	@$(MAKE) subdir_testclean no_deps=t

tests: $(objpfx)c++-types-check.out $(objpfx)check-local-headers.out
ifneq ($(CXX),no)
check-data := $(firstword $(wildcard \
		$(foreach D,$(add-ons) scripts,\
			  $(patsubst %,$D/data/c++-types-%.data,\
				     $(abi-name) \
				     $(addsuffix -$(config-os),\
						 $(config-machine) \
						 $(base-machine))))))
ifneq (,$(check-data))
$(objpfx)c++-types-check.out: $(check-data) scripts/check-c++-types.sh
	scripts/check-c++-types.sh $< $(CXX) $(filter-out -std=gnu99 -Wstrict-prototypes,$(CFLAGS)) $(CPPFLAGS) > $@
else
$(objpfx)c++-types-check.out:
	@echo 'WARNING C++ tests not run; create a c++-types-XXX file'
	@echo "not run" > $@
endif
endif

$(objpfx)check-local-headers.out: scripts/check-local-headers.sh
	AWK='$(AWK)' scripts/check-local-headers.sh \
	  "$(includedir)" "$(objpfx)" > $@

ifneq ($(PERL),no)
installed-headers = argp/argp.h assert/assert.h catgets/nl_types.h \
		    crypt/crypt.h ctype/ctype.h debug/execinfo.h \
		    dirent/dirent.h dlfcn/dlfcn.h elf/elf.h elf/link.h \
		    gmon/sys/gmon.h gmon/sys/gmon_out.h gmon/sys/profil.h \
		    grp/grp.h gshadow/gshadow.h iconv/iconv.h iconv/gconv.h \
		    $(wildcard inet/netinet/*.h) \
		    $(wildcard inet/arpa/*.h inet/protocols/*.h) \
		    inet/aliases.h inet/ifaddrs.h inet/netinet/ip6.h \
		    inet/netinet/icmp6.h intl/libintl.h io/sys/stat.h \
		    io/sys/statfs.h io/sys/vfs.h io/sys/statvfs.h \
		    io/fcntl.h io/sys/fcntl.h io/poll.h io/sys/poll.h \
		    io/utime.h io/ftw.h io/fts.h io/sys/sendfile.h \
		    libio/stdio.h libio/libio.h locale/locale.h \
		    locale/langinfo.h locale/xlocale.h login/utmp.h \
		    login/lastlog.h login/pty.h malloc/malloc.h \
		    malloc/obstack.h malloc/mcheck.h math/math.h \
		    math/complex.h math/fenv.h math/tgmath.h misc/sys/uio.h \
		    $(wildcard nis/rpcsvc/*.h) nptl_db/thread_db.h \
		    nptl/sysdeps/pthread/pthread.h nptl/semaphore.h \
		    nss/nss.h posix/sys/utsname.h posix/sys/times.h \
		    posix/sys/wait.h posix/sys/types.h posix/unistd.h \
		    posix/glob.h posix/regex.h posix/wordexp.h posix/fnmatch.h\
		    posix/getopt.h posix/tar.h posix/sys/unistd.h \
		    posix/sched.h posix/re_comp.h posix/wait.h \
		    posix/cpio.h posix/spawn.h pwd/pwd.h resolv/resolv.h \
		    resolv/netdb.h $(wildcard resolv/arpa/*.h) \
		    resource/sys/resource.h resource/sys/vlimit.h \
		    resource/sys/vtimes.h resource/ulimit.h rt/aio.h \
		    rt/mqueue.h setjmp/setjmp.h shadow/shadow.h \
		    signal/signal.h signal/sys/signal.h socket/sys/socket.h \
		    socket/sys/un.h stdio-common/printf.h \
		    stdio-common/stdio_ext.h stdlib/stdlib.h stdlib/alloca.h \
		    stdlib/monetary.h stdlib/fmtmsg.h stdlib/ucontext.h \
		    sysdeps/generic/inttypes.h sysdeps/generic/stdint.h \
		    stdlib/errno.h stdlib/sys/errno.h string/string.h \
		    string/strings.h string/memory.h string/endian.h \
		    string/argz.h string/envz.h string/byteswap.h \
		    $(wildcard sunrpc/rpc/*.h sunrpc/rpcsvc/*.h) \
		    sysvipc/sys/ipc.h sysvipc/sys/msg.h sysvipc/sys/sem.h \
		    sysvipc/sys/shm.h termios/termios.h \
		    termios/sys/termios.h termios/sys/ttychars.h time/time.h \
		    time/sys/time.h time/sys/timeb.h wcsmbs/wchar.h \
		    wctype/wctype.h

tests: $(objpfx)begin-end-check.out
$(objpfx)begin-end-check.out: scripts/begin-end-check.pl
	$(PERL) scripts/begin-end-check.pl $(installed-headers) > $@
endif

# The realclean target is just like distclean for the parent, but we want
# the subdirs to know the difference in case they care.
realclean distclean: parent-clean
# This is done this way rather than having `subdir_distclean' be a
# dependency of this target so that libc.a will be removed before the
# subdirectories are dealt with and so they won't try to remove object
# files from it when it's going to be removed anyway.
	@$(MAKE) distclean-1 no_deps=t distclean-1=$@ avoid-generated=yes \
		 sysdep-subdirs="$(sysdep-subdirs)"
	-rm -f $(postclean)

# Subroutine of distclean and realclean.
distclean-1: subdir_$(distclean-1)
	-rm -f $(config-generated)
	-rm -f $(addprefix $(objpfx),config.status config.cache config.log)
	-rm -f $(addprefix $(objpfx),config.make config-name.h config.h)
ifdef objdir
	-rm -f $(objpfx)Makefile
endif
	-rm -f $(sysdep-$(distclean-1))

# Make the TAGS file for Emacs users.

.PHONY: TAGS
TAGS:
	scripts/list-sources.sh | sed -n -e '/Makefile/p' \
	  $(foreach S,[chsSyl] cxx sh bash pl,\
		    $(subst .,\.,-e '/.$S\(.in\)*$$/p')) \
	| $(ETAGS) -o $@ -

# Make the distribution tarfile.
.PHONY: dist dist-prepare

generated := $(generated) stubs.h

files-for-dist := README FAQ INSTALL NOTES configure ChangeLog NEWS

# Regenerate stuff, then error if these things are not committed yet.
dist-prepare: $(files-for-dist)
	conf=`find sysdeps $(addsuffix /sysdeps,$(sysdeps-add-ons)) \
		   -name configure`; \
	$(MAKE) $$conf && \
	git diff --stat HEAD -- $^ $$conf \
	| $(AWK) '{ print; rc=1 } END { exit rc }'

%.tar: FORCE
	git archive --prefix=$*/ $* > $@.new
	mv -f $@.new $@

# Do `make dist dist-version=X.Y.Z' to make tar files of an older version.

ifneq (,$(strip $(dist-version)))
dist: $(foreach Z,.bz2 .gz .xz,$(dist-version).tar$Z)
	md5sum $^
else
dist: dist-prepare
	@if v=`git describe`; then \
	  echo Distribution version $$v; \
	  $(MAKE) dist dist-version=$$v; \
	else \
	  false; \
	fi
endif

define format-me
@rm -f $@
makeinfo --no-validate --plaintext --no-number-sections $< -o $@
-chmod a-w $@
endef
INSTALL: manual/install.texi; $(format-me)
NOTES: manual/creature.texi; $(format-me)
manual/dir-add.texi manual/dir-add.info: FORCE
	$(MAKE) $(PARALLELMFLAGS) -C $(@D) $(@F)
FAQ: scripts/gen-FAQ.pl FAQ.in
	$(PERL) $^ > $@.new && rm -f $@ && mv $@.new $@ && chmod a-w $@
FORCE:

iconvdata/% localedata/% po/% manual/%: FORCE
	$(MAKE) $(PARALLELMFLAGS) -C $(@D) $(@F)

# glibc 2.0 contains some header files which aren't used with glibc 2.1
# anymore.
# These rules should remove those headers
ifeq (,$(install_root))
ifeq ($(old-glibc-headers),yes)
install: remove-old-headers
endif
endif

headers2_0 :=	__math.h bytesex.h confname.h direntry.h elfclass.h	\
		errnos.h fcntlbits.h huge_val.h ioctl-types.h		\
		ioctls.h iovec.h jmp_buf.h libc-lock.h local_lim.h	\
		mathcalls.h mpool.h nan.h ndbm.h posix1_lim.h		\
		posix2_lim.h posix_opt.h resourcebits.h schedbits.h	\
		selectbits.h semaphorebits.h sigaction.h sigcontext.h	\
		signum.h sigset.h sockaddrcom.h socketbits.h stab.def	\
		statbuf.h statfsbuf.h stdio-lock.h stdio_lim.h		\
		syscall-list.h termbits.h timebits.h ustatbits.h	\
		utmpbits.h utsnamelen.h waitflags.h waitstatus.h	\
		xopen_lim.h gnu/types.h sys/ipc_buf.h			\
		sys/kernel_termios.h sys/msq_buf.h sys/sem_buf.h	\
		sys/shm_buf.h sys/socketcall.h sigstack.h

.PHONY: remove-old-headers
remove-old-headers:
	rm -f $(addprefix $(inst_includedir)/, $(headers2_0))
