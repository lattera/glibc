# Copyright (C) 1994, 1997 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
# Contributed by Joel Sherrill (jsherril@redstone-emh2.army.mil),
#    On-Line Applications Research Corporation.
#

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

subdir := bare

bare-routines := brdinit console strtsupp
routines = $(bare-routines)
elided-routines = $(bare-routines)
extra-objs = $(bare-routines:%=%.o)

install-lib = lib$(config-vendor).a

include ../Rules

#
#  For bare targets, the $(config-vendor) is the name of the board.
#  We will place the board dependent code ONLY in a library which
#  is board dependent.  This way many target boards can share a
#  single libc.a.  To resolve all symbols and successfully link
#  a program, the application must link against libc.a and libMY_TARGET.a.
#  For example, the target specific library for the Motorola MVME135
#  board will be named libmvme135.a.  To link a program for the
#  MVME135, one must link against -lc and -lmvme135.
#

lib: $(objpfx)lib$(config-vendor).a

$(objpfx)lib$(config-vendor).a: $(bare-routines:%=$(objpfx)%.o)
# This library is small enough that it's simplest to recreate the archive
# from scratch each time.
	rm -f $@
ifdef objdir
	cd $(objpfx); $(AR) cq$(verbose) $(@:$(objpfx)%=%) $(^:$(objpfx)%=%)
else
	$(AR) cq$(verbose) $@ $^
endif
	$(RANLIB) $@
