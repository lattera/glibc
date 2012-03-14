# Copyright (C) 1999, 2000, 2012 Free Software Foundation, Inc.
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
#	Sub-makefile for conform portion of the library.
#
subdir	:= conform

include ../Rules

ifneq (yes,$(fast-check))
ifeq (no,$(cross-compiling))
tests: $(objpfx)run-conformtest.out
endif
endif

$(objpfx)run-conformtest.out: run-conformtest.sh conformtest.pl \
			      $(wildcard data/*.h-data) \
			      $(wildcard data/*/*.h-data)
	-$(SHELL) -e $< $(objpfx) $(PERL) '$(CC)' \
	  '-I../include $(+sysdep-includes) $(sysincludes) -I..'

generated = $(wildcard $(objpfx)conform-*.out)
