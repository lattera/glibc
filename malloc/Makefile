# Copyright (C) 1991-1999,2000,2001,2002,2003 Free Software Foundation, Inc.
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
#	Makefile for malloc routines
#
subdir	:= malloc

all:

dist-headers := malloc.h
headers := $(dist-headers) obstack.h mcheck.h
tests := mallocbug tst-malloc tst-valloc tst-calloc tst-obstack \
	 tst-mallocstate
test-srcs = tst-mtrace

distribute = thread-m.h mtrace.pl mcheck-init.c stackinfo.h memusage.h \
	     memusage.sh memusagestat.c tst-mtrace.sh arena.c hooks.c

# Things which get pasted together into gmalloc.c.
gmalloc-routines := malloc morecore
# Things to include in the standalone distribution.
dist-routines = $(gmalloc-routines) mcheck mtrace
routines = $(dist-routines) obstack

install-lib := libmcheck.a
non-lib.a := libmcheck.a

# Additional library.
extra-libs = libmemusage
extra-libs-others = $(extra-libs)

libmemusage-routines = memusage
libmemusage-inhibit-o = $(filter-out .os,$(object-suffixes))

# These should be removed by `make clean'.
extra-objs = mcheck-init.o libmcheck.a

# Include the cleanup handler.
aux := set-freeres thread-freeres

include ../Makeconfig

CPPFLAGS-memusagestat = -DNOT_IN_libc

# The Perl script to analyze the output of the mtrace functions.
ifneq ($(PERL),no)
install-bin-script = mtrace
generated = mtrace

# The Perl script will print addresses and to do this nicely we must know
# whether we are on a 32 or 64 bit machine.
ifneq ($(findstring wordsize-32,$(config-sysdirs)),)
address-width=10
else
address-width=18
endif
endif

# Unless we get a test for the availability of libgd which also works
# for cross-compiling we disable the memusagestat generation in this
# situation.
ifneq ($(cross-compiling),yes)
# If the gd library is available we build the `memusagestat' program.
ifneq ($(LIBGD),no)
install-bin = memusagestat
install-bin-script += memusage
generated += memusagestat memusage
extra-objs += memusagestat.o

# The configure.in check for libgd and its headers did not use $SYSINCLUDES.
# The directory specified by --with-headers usually contains only the basic
# kernel interface headers, not something like libgd.  So the simplest thing
# is to presume that the standard system headers will be ok for this file.
$(objpfx)memusagestat.o: sysincludes = # nothing
endif
endif

# Another goal which can be used to override the configure decision.
.PHONY: do-memusagestat
do-memusagestat: $(objpfx)memusagestat

memusagestat-modules = memusagestat
$(objpfx)memusagestat: $(memusagestat-modules:%=$(objpfx)%.o)
	$(LINK.o) -o $@ $^ $(libgd-LDFLAGS) -lgd -lpng -lz -lm

include ../Rules

CFLAGS-mcheck-init.c = $(PIC-ccflag)

$(objpfx)libmcheck.a: $(objpfx)mcheck-init.o
	-rm -f $@
	$(patsubst %/,cd % &&,$(objpfx)) \
	$(LN_S) $(<F) $(@F)

lib: $(objpfx)libmcheck.a

ifeq (no,$(cross-compiling))
ifeq (yes,$(build-shared))
ifneq ($(PERL),no)
tests: $(objpfx)tst-mtrace.out
$(objpfx)tst-mtrace.out: tst-mtrace.sh $(objpfx)tst-mtrace
	$(SHELL) -e $< $(common-objpfx)
endif
endif
endif

# Uncomment this for test releases.  For public releases it is too expensive.
#CPPFLAGS-malloc.o += -DMALLOC_DEBUG=1

$(objpfx)mtrace: mtrace.pl
	rm -f $@.new
	sed -e 's|@PERL@|$(PERL)|' -e 's|@XXX@|$(address-width)|' \
	    -e 's|@VERSION@|$(version)|' $^ > $@.new \
	&& rm -f $@ && mv $@.new $@ && chmod +x $@

$(objpfx)memusage: memusage.sh
	rm -f $@.new
	sed -e 's|@BASH@|$(BASH)|' -e 's|@VERSION@|$(version)|' \
	    -e 's|@SLIBDIR@|$(slibdir)|' -e 's|@BINDIR@|$(bindir)|' $^ > $@.new \
	&& rm -f $@ && mv $@.new $@ && chmod +x $@


# The implementation uses `dlsym'
$(objpfx)libmemusage.so: $(common-objpfx)dlfcn/libdl.so

# Extra dependencies
$(foreach o,$(all-object-suffixes),$(objpfx)malloc$(o)): arena.c hooks.c
