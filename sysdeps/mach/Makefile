# Copyright (C) 1993, 1994 Free Software Foundation, Inc.
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

ifndef mach-srcdir-computed
mach-srcdir-computed := yes

# If we were configured with `--with-mach=DIR', then config.make sets
# mach-srcdir to DIR.  Otherwise guess we are in a big source tree.
ifndef mach-srcdir
mach-srcdir = ../mach
endif

# mach-srcdir is now set to the logical directory name.  This name might be
# relative to $(objdir), might be relative to the source directory $(..), or
# might be absolute.  We choose among these possibilities by looking for a
# canonical file in each of those places (in that order).
f := mach/mach.defs # Random file that identifies the Mach source dir.
f := $(strip $f)
mach-srcdir := $(firstword $(patsubst %/$f,%,$(wildcard $(addsuffix /$f,\
	       $(objpfx)$(mach-srcdir) $(..)$(mach-srcdir)))) $(mach-srcdir))

endif	# ! mach-srcdir-computed

ifdef in-Makerules

# Look for header files in mach/ under the top-level library source directory.
includes += -I$(..)mach

# When compiling, use the Mach header files directly from the kernel sources.
includes += -I$(mach-srcdir)

# Find Mach header files in the kernel source.
vpath mach/%.h $(mach-srcdir)
vpath device/%.h $(mach-srcdir)

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
