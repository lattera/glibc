# Copyright (C) 1993, 1994, 1995 Free Software Foundation, Inc.
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

# Get mach-srcdir defined.
old-in-Makerules := $(in-Makerules)
in-Makerules :=
include $(..)sysdeps/mach/Makefile
in-Makerules := $(old-in-Makerules)


ifndef hurd-srcdir-computed
hurd-srcdir-computed := yes

# If we were configured with `--with-hurd=DIR', then config.make sets
# hurd-srcdir to DIR.  Otherwise guess we are in a big source tree.
ifndef hurd-srcdir
hurd-srcdir = ../hurd
endif

# hurd-srcdir is now set to the logical directory name.  This name might be
# relative to $(objdir), might be relative to the source directory $(..), or
# might be absolute.  We choose among these possibilities by looking for a
# canonical file in each of those places (in that order).
f := hurd/hurd_types.defs # Random file that identifies the Hurd source dir.
f := $(strip $f)
hurd-srcdir := $(firstword $(patsubst %/$f,%,$(wildcard $(addsuffix /$f,\
	       $(objpfx)$(hurd-srcdir) $(..)$(hurd-srcdir)))) $(hurd-srcdir))

endif	# ! hurd-srcdir-computed

ifdef in-Makerules

# For the time being, elide directories where nothing at all works.
subdirs := $(filter-out sunrpc manual,$(subdirs)) # XXX

# Look for header files in hurd/ under the top-level library source directory.
includes += -I$(..)hurd

# When compiling, use the Hurd header files directly from the sources.
includes += -I$(hurd-srcdir)

# When compiling, find cthreads.h in the Hurd cthreads source.
# This directory has some other (private) header file with
# conflicting names, so we put it last.
last-includes += -I$(hurd-srcdir)/libthreads

# Find the Hurd header files in the Hurd source.
vpath hurd/%.h $(hurd-srcdir)

# Do not use any assembly code from sysdeps/unix (and subdirectories).
# This bypasses all the system call stubs and uses any existing posix or
# generic C files instead.
inhibit-sysdep-asm += unix*

# Don't try to generate anything from the installed Unix system and its
# libraries.  That is only of use when building for a Unix system, so as to
# be compatible with some existing binaries for that system.
inhibit-glue = yes


ifeq (,$(filter mach hurd,$(subdir)))
# Subdirectories other than hurd/ might use the generated Hurd headers.
# So make sure we get a chance to run in hurd/ to make them before all else.
# (But we don't want to do this in mach/, because hurd/ needs some things
# there, and we know mach/ doesn't need anything from hurd/.)

ifdef objpfx
hurd-objpfx = $(objpfx)
else
hurd-objpfx = $(..)hurd/
endif

# These are all the generated headers that <hurd.h> includes.
before-compile += $(patsubst %,$(hurd-objpfx)hurd/%.h,io fs process)
$(patsubst %,$(hurd-objpfx)hurd/%.%,io fs process):
	$(MAKE) -C $(..)hurd generated no_deps=t
endif

# Generate errnos.h and sys_errlist.c from the section of the manual that
# lists all the errno codes.

errno.texinfo = $(..)manual/errno.texi

hurd = $(..)sysdeps/mach/hurd

$(hurd)/errnos.h: $(objpfx)stamp-errnos ;
$(objpfx)stamp-errnos: $(hurd)/errnos.awk $(errno.texinfo) \
		       $(mach-srcdir)/mach/message.h \
		       $(mach-srcdir)/mach/kern_return.h \
		       $(mach-srcdir)/mach/mig_errors.h \
		       $(mach-srcdir)/device/device_types.h
	gawk -f $^ > $(hurd)/errnos.h-tmp
# Make it unwritable so noone will edit it by mistake.
	-chmod a-w $(hurd)/errnos.h-tmp
	./$(..)move-if-change $(hurd)/errnos.h-tmp $(hurd)/errnos.h
	test -d CVS && \
	  (cd $(hurd); cvs commit -m'Regenerated from $^' errnos.h)
	touch $@

$(hurd)/errlist.c: $(hurd)/errlist.awk $(errno.texinfo)
	gawk -f $^ > $@-tmp
# Make it unwritable so noone will edit it by mistake.
	-chmod a-w $@-tmp
	mv -f $@-tmp $@
	test -d CVS && cvs commit -m'Regenerated from $^' $@

# We install the real libc.a as libcrt.a and as libc.a we install a linker
# script which does -( -lcrt -lmachuser -lhurduser -).

libc-name = crt

ifeq (,$(subdir))
install-others += $(libdir)/libc.a
$(libdir)/libc.a: $(hurd)/libc-ldscript; $(do-install)
endif

# For the shared library, we don't need to do the linker script machination.
# Instead, we specify the required libraries when building the shared object.
LDLIBS-c.so = -lmachuser -lhurduser


endif	# in-Makerules
