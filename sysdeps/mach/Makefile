# Copyright (C) 1993, 1994, 1996 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

ifdef in-Makerules

# Look for header files in mach/ under the top-level library source directory.
includes += -I$(..)mach

ifneq (mach,$(subdir))
# Subdirectories other than mach/ might use the generated Mach headers.
# So make sure we get a chance to run in mach/ to make them before all else.

ifdef objpfx
mach-objpfx = $(objpfx)
else
mach-objpfx = $(..)mach/
endif

# These are all the generated files that <mach.h> includes.
mach-before-compile := $(mach-objpfx)mach-shortcuts.h \
		       $(patsubst %,$(mach-objpfx)mach/mach_%.h,\
				  interface port host)

# This patsubst generates patterns like `m%h-shortcuts.h', which are damn
# likely to match just the corresponding particular file we want.
$(patsubst mach%,m\%h%,$(mach-before-compile)): # Run only if doesn't exist.
	$(MAKE) -C $(..)mach generated no_deps=t

before-compile += $(mach-before-compile)
endif

endif	# in-Makerules
