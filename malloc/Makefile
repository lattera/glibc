# Copyright (C) 1991, 92, 93, 94, 95, 96, 97 Free Software Foundation, Inc.
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
headers := $(dist-headers) obstack.h
tests := mallocbug

distribute = thread-m.h mtrace.awk mcheck-init.c mcheck.h

# Things which get pasted together into gmalloc.c.
gmalloc-routines := malloc morecore
# Things to include in the standalone distribution.
dist-routines = $(gmalloc-routines) mcheck mtrace
routines = $(dist-routines) obstack

install-lib := libmcheck.a
non-lib.a := libmcheck.a

# These should be removed by `make clean'.
extra-objs = mcheck-init.o libmcheck.a

include ../Rules

$(objpfx)libmcheck.a: $(objpfx)mcheck-init.o
	-rm -f $@
	ln $< $@

lib: $(objpfx)libmcheck.a


CPPFLAGS-malloc.o += -DMALLOC_DEBUG
