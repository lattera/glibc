# Copyright (C) 1991, 92, 93, 94, 95, 96, 97, 98 Free Software Foundation, Inc.
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
#	Makefile for malloc routines
#
subdir	:= malloc

all:

dist-headers := malloc.h
headers := $(dist-headers) obstack.h mcheck.h
tests := mallocbug

distribute = thread-m.h mtrace.pl mcheck-init.c

# Things which get pasted together into gmalloc.c.
gmalloc-routines := malloc morecore
# Things to include in the standalone distribution.
dist-routines = $(gmalloc-routines) mcheck mtrace
routines = $(dist-routines) obstack

install-lib := libmcheck.a
non-lib.a := libmcheck.a

# These should be removed by `make clean'.
extra-objs = mcheck-init.o libmcheck.a

# Include the cleanup handler.
aux := set-freeres

# The AWK script to analyze the output of the mtrace functions.
ifneq ($(PERL),no)
install-bin = mtrace
generated = mtrace

# The Perl script will print addresses and to do this nicely we must know
# whether we are on a 32 or 64 bit machine.
ifneq ($(findstring wordsize-32,$(config-sysdirs)),)
address-width=10
else
address-width=18
endif
endif

include ../Rules

$(objpfx)libmcheck.a: $(objpfx)mcheck-init.o
	-rm -f $@
	ln $< $@

lib: $(objpfx)libmcheck.a


# Uncomment this for test releases.  For public releases it is too expensive.
#CPPFLAGS-malloc.o += -DMALLOC_DEBUG

$(objpfx)mtrace: mtrace.pl
	rm -f $@.new
	sed -e 's|@PERL@|$(PERL)|' -e 's|@XXX@|$(address-width)|' \
	    -e 's|@VERSION@|$(version)|' $^ > $@.new \
	&& rm -fr $@ && mv $@.new $@ && chmod +x $@
