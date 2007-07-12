# Copyright (C) 1999 Free Software Foundation, Inc.
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
#	Sub-makefile for the C stub add-on library.
#
subdir	:= c_stubs

tests-static := test-stdio
tests := $(tests-static)

libc_stubs-objs := gconv_stubs.o

install-lib := libc_stubs.a
non-lib.a := libc_stubs.a

extra-objs := gconv_stubs.o libc_stubs.a

include ../Makeconfig

CPPFLAGS += -I../iconv

include ../Rules

$(objpfx)libc_stubs.a: $(addprefix $(objpfx), $(libc_stubs-objs))
	-rm -f $@
	$(CC) -nostdlib -nostartfiles -r -o $@ $^

lib: $(objpfx)libc_stubs.a

$(objpfx)test-stdio: $(objpfx)libc_stubs.a
