# Copyright (C) 1998 Free Software Foundation, Inc.
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
distribute = sigcontextinfo.h

routines := backtrace backtracesyms backtracesymsfd

CFLAGS-backtrace.c = -fno-omit-frame-pointer

tests = backtrace-tst

extra-libs = libSegFault
extra-libs-others = $(extra-libs)

libSegFault-routines = segfault
libSegFault-inhibit-o = $(filter-out .os,$(object-suffixes))

distribute += catchsegv.sh
install-bin = catchsegv
generated = catchsegv

include ../Rules

$(objpfx)catchsegv: catchsegv.sh $(common-objpfx)soversions.mk \
		    $(common-objpfx)config.make
	sed -e 's|@VERSION@|$(version)|' -e 's|@SLIB@|$(slibdir)|' \
	    -e 's|@SOVER@|$(libSegFault.so-version)|' $< > $@.new
	chmod 555 $@.new
	mv -f $@.new $@

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libSegFault.so: $(common-objpfx)libc.so $(common-objpfx)elf/ld.so
