# Copyright (C) 1998, 1999, 2000, 2001 Free Software Foundation, Inc.
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
#	Sub-makefile for debug portion of the library.
#
subdir	:= debug

headers	:= execinfo.h
distribute = sigcontextinfo.h register-dump.h frame.h

routines := backtrace backtracesyms backtracesymsfd noophooks

CFLAGS-backtrace.c = -fno-omit-frame-pointer

tests = backtrace-tst

extra-libs = libSegFault libpcprofile
extra-libs-others = $(extra-libs)

libSegFault-routines = segfault
libSegFault-inhibit-o = $(filter-out .os,$(object-suffixes))

libpcprofile-routines = pcprofile
libpcprofile-inhibit-o = $(filter-out .os,$(object-suffixes))

others = pcprofiledump
install-bin = pcprofiledump xtrace

include ../Makeconfig

distribute += catchsegv.sh xtrace.sh
ifeq ($(elf),yes)
ifeq ($(build-shared),yes)
install-bin += catchsegv
endif
endif
generated = catchsegv xtrace

include ../Rules

$(objpfx)catchsegv: catchsegv.sh $(common-objpfx)soversions.mk \
		    $(common-objpfx)config.make
	sed -e 's|@VERSION@|$(version)|' -e 's|@SLIB@|$(slibdir)|' $< > $@.new
	chmod 555 $@.new
	mv -f $@.new $@

$(objpfx)pcprofiledump: $(objpfx)pcprofiledump.o

$(objpfx)xtrace: xtrace.sh
	rm -f $@.new
	sed -e 's|@BASH@|$(BASH)|' -e 's|@VERSION@|$(version)|' \
	    -e 's|@LIBDIR@|$(libdir)|' -e 's|@BINDIR@|$(bindir)|' $^ > $@.new \
	&& rm -f $@ && mv $@.new $@ && chmod +x $@

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libSegFault.so: $(common-objpfx)libc.so \
			 $(common-objpfx)libc_nonshared.a \
			 $(common-objpfx)elf/ld.so
$(objpfx)libpcprofile.so: $(common-objpfx)libc.so \
			  $(common-objpfx)libc_nonshared.a \
			  $(common-objpfx)elf/ld.so
