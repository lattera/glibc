# Copyright (C) 1993, 1994, 1995, 1996 Free Software Foundation, Inc.
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

subdirs := $(filter-out sunrpc,$(subdirs)) # XXX skip broken dirs

# Look for header files in hurd/ under the top-level library source directory.
includes += -I$(..)hurd

# Do not use any assembly code from sysdeps/unix (and subdirectories).
# This bypasses all the system call stubs and uses any existing posix or
# generic C files instead.
inhibit-sysdep-asm += unix*
inhibit-unix-syscalls = yes

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

# Generate errnos.h from the section of the manual that lists all the errno
# codes.

errno.texinfo = $(..)manual/errno.texi

hurd = $(..)sysdeps/mach/hurd

define mach-errno-h
($(foreach h,mach/message.h \
	     mach/kern_return.h \
	     mach/mig_errors.h \
	     device/device_types.h,\
 echo '#include <$h>';\
 ))
endef

# We use the compiler to generate a list of absolute file names for
# the headers we want to search for Mach error codes, listed above (and
# incidentally, all other headers those include).
-include $(common-objpfx)errnos.d
$(common-objpfx)errnos.d: $(mach-errnos-deps)
	$(mach-errno-h) | \
	$(CC) $(CPPFLAGS) -M -x c - | \
	sed $(sed-remove-objpfx) -e 's,- *:,mach-errnos-deps :=,' > $@t
	mv -f $@t $@

$(hurd)/errnos.h: $(common-objpfx)stamp-errnos ;
$(common-objpfx)stamp-errnos: $(hurd)/errnos.awk $(errno.texinfo) \
			      $(mach-errnos-deps)
	gawk -f $^ > $(hurd)/errnos.h-tmp
# Make it unwritable so noone will edit it by mistake.
	-chmod a-w $(hurd)/errnos.h-tmp
	./$(..)move-if-change $(hurd)/errnos.h-tmp $(hurd)/errnos.h
	test ! -d CVS || \
	  (cd $(hurd); cvs commit -m'Regenerated from $^' errnos.h)
	touch $@
generated += errnos.d stamp-errnos

# We install the real libc.a as libcrt.a and as libc.a we install a linker
# script which does -( -lcrt -lmachuser -lhurduser -).

libc-name = crt

ifeq (,$(subdir))
install-others += $(libdir)/libc.a
$(libdir)/libc.a: $(hurd)/libc-ldscript; $(do-install)
endif

# For the shared library, we don't need to do the linker script machination.
# Instead, we specify the required libraries when building the shared object.
$(common-objpfx)libc.so: $(firstword $(objdir) $(..)mach)/libmachuser.so \
			 $(firstword $(objdir) $(..)hurd)/libhurduser.so
ifndef objpfx
rpath-link += $(..)mach:$(..)hurd
endif


endif	# in-Makerules
