# Copyright (C) 1993, 94, 95, 96, 97, 98 Free Software Foundation, Inc.
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
# License along with the GNU C Library; see the file COPYING.LIB.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

ifdef in-Makerules

subdirs := $(filter-out sunrpc,$(subdirs)) # XXX skip broken dirs
# See hurd/Makefile for commands that install some crucial sunrpc headers.

# Look for header files in hurd/ under the top-level library source directory.
# Look for generated header files where they get created.
includes += -I$(..)hurd -I$(common-objpfx)hurd/

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

hurd-objpfx = $(common-objpfx)hurd/

# These are all the generated headers that <hurd.h> includes.
before-compile += $(patsubst %,$(hurd-objpfx)hurd/%.h,auth io fs process)
$(patsubst %,$(hurd-objpfx)hurd/%.%,auth io fs process):
	$(MAKE) -C $(..)hurd generated no_deps=t
endif

# Hurd profil.c includes this file, so give a rule to make it.
ifeq ($(subdir),gmon)
$(common-objpfx)hurd/../mach/RPC_task_get_sampled_pcs.c:
	$(MAKE) -C $(..)mach generated no_deps=t
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
	sed $(sed-remove-objpfx) -e 's,- *:,mach-errnos-deps :=,' \
	    -e 's,\.\./,$(..),g' > $@t
	mv -f $@t $@

$(hurd)/errnos.h: $(common-objpfx)stamp-errnos ;
$(common-objpfx)stamp-errnos: $(hurd)/errnos.awk $(errno.texinfo) \
			      $(mach-errnos-deps)
	gawk -f $^ > $(hurd)/errnos.h-tmp
# Make it unwritable so noone will edit it by mistake.
	-chmod a-w $(hurd)/errnos.h-tmp
	./$(..)move-if-change $(hurd)/errnos.h-tmp $(hurd)/errnos.h
ifeq ($(with-cvs),yes)
	test ! -d CVS || \
	  (cd $(hurd); cvs commit -m'Regenerated from $^' errnos.h)
endif
	touch $@

common-generated += errnos.d stamp-errnos

# We install the real libc.a as libcrt.a and as libc.a we install a linker
# script which does -( -lcrt -lmachuser -lhurduser -).

libc-name = crt

ifeq (,$(subdir))
install-others += $(inst_libdir)/libc.a $(inst_libdir)/libc_p.a
$(inst_libdir)/libc.a: $(hurd)/libc-ldscript $(+force); $(do-install)
$(inst_libdir)/libc_p.a: $(hurd)/libc_p-ldscript $(+force); $(do-install)
endif

# Make sure these are used to build the libc.so shared object too.
rpcuserlibs := $(common-objpfx)mach/libmachuser.so \
	       $(common-objpfx)hurd/libhurduser.so
$(common-objpfx)libc.so: $(rpcuserlibs)
rpath-link := $(rpath-link):$(common-objpfx)mach:$(common-objpfx)hurd

# And get them into the libc.so ldscript.
$(inst_libdir)/libc.so: $(rpcuserlibs)

# The RPC stubs from these libraries are needed in building the dynamic
# linker, too.  It must be self-contained, so we link the needed PIC
# objects directly into the shared object.
ifeq (elf,$(subdir))
$(objpfx)librtld.so: $(rpcuserlibs:.so=_pic.a)
endif


endif	# in-Makerules
