# Makefile for elf subdirectory of GNU C Library.

# Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

subdir		:= elf

headers		= elf.h elfclass.h link.h dlfcn.h
routines	= $(dl-routines) dl-open dl-close dl-symbol dl-support

# The core dynamic linking functions are in libc for the static and
# profiled libraries.
dl-routines	= $(addprefix dl-,load lookup object reloc deps \
			          runtime error init fini debug)
# But they are absent from the shared libc, because that code is in ld.so.
elide-routines.so = $(dl-routines) dl-support

# ld.so uses those routines, plus some special stuff for being the program
# interpreter and operating independent of libc.
rtld-routines	:= rtld $(dl-routines) dl-sysdep dl-minimal
distribute	= $(rtld-routines:=.c) dynamic-link.h do-rel.h dl-machine.h \
		  soinit.c sofini.c ldd.sh.in linux-compat.c

extra-libs	= libdl
libdl-routines	:= dlopen dlclose dlsym dlerror dladdr


all: # Make this the default target; it will be defined in Rules.

include ../Makeconfig

ifeq (yes,$(build-shared))
extra-objs	= $(rtld-routines:=.so) soinit.so sofini.so
generated	= librtld.so dl-allobjs.so
install-others	= $(slibdir)/$(rtld-installed-name)
install-bin	= ldd

ifneq (,$(filter linux% linux,$(config-os)))
extra-objs	+= linux-compat.so
install-others	+= $(slibdir)/ld-linux.so.1
lib-noranlib: $(objpfx)ld-linux.so.1
endif
endif

include ../Rules


ifeq (yes, $(build-shared))
# Make sure these things are built in the `make lib' pass so they can be used
# to run programs during the `make others' pass.
lib-noranlib: $(objpfx)$(rtld-installed-name) \
	      $(addprefix $(objpfx),$(extra-objs))
endif

# Command to link into a larger single relocatable object.
reloc-link = $(LINK.o) -nostdlib -nostartfiles -r -o $@

$(objpfx)dl-allobjs.so: $(rtld-routines:%=$(objpfx)%.so)
	$(reloc-link) $^

# Link together the dynamic linker into a single relocatable object.
# We use this to produce both the ABI-compliant and Linux-compatible
# dynamic linker shared objects below.
$(objpfx)librtld.so: $(objpfx)dl-allobjs.so \
		     $(patsubst %,$(common-objpfx)lib%_pic.a,\
				c $(LDLIBS-c.so:-l%=%))
	$(reloc-link) '-Wl,-(' $^ -lgcc '-Wl,-)'

$(objpfx)ld.so: $(objpfx)librtld.so
	$(rtld-link) -Wl,-soname=$(rtld-installed-name)
$(objpfx)ld-linux.so.1: $(objpfx)librtld.so
	$(rtld-link) -Wl,-soname=ld-linux.so.1

define rtld-link
$(LINK.o) -nostdlib -nostartfiles -shared -o $@ \
	  -Wl,-rpath=$(default-rpath) $^
endef

# The dl code in the static libc needs a default library path.
CFLAGS-dl-support.c = -D'DEFAULT_RPATH="$(default-rpath)"'

ifneq (ld.so, $(rtld-installed-name))
# Make sure ld-gnu.so.1 exists in the build directory so we can link
# against it.
$(objpfx)$(rtld-installed-name): $(objpfx)ld.so
	rm -f $@
	ln $< $@
endif

# The Linux-compatible dynamic linker shared object is just the same
# with one object file of compatibility initialization code added.
$(objpfx)ld-linux.so.1: $(objpfx)linux-compat.so


# Specify the dependencies of libdl.so; its commands come from the generic
# rule to build a shared library.
$(objpfx)libdl.so: $(objpfx)libdl_pic.a $(common-objpfx)libc.so $(objpfx)ld.so

$(slibdir)/$(rtld-installed-name): $(objpfx)ld.so; $(do-install-program)
$(slibdir)/ld-linux.so.1: $(objpfx)ld-linux.so.1; $(do-install-program)

$(objpfx)ldd: ldd.sh.in Makefile
	sed 's%@RTLD@%$(slibdir)/$(rtld-installed-name)%g' < $< > $@.new
	chmod 555 $@.new
	mv -f $@.new $@
