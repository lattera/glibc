# Makefile for elf subdirectory of GNU C Library.

# Copyright (C) 1995 Free Software Foundation, Inc.
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

headers		:= elf.h libelf.h link.h dlfcn.h
routines	:= init-first

extra-libs	= libelf libdl
libelf-routines	:= elf_hash
libdl-routines	:= dlopen dlclose dlsym dlerror
libdl-inhibit-o	= $(filter-out .so,$(object-suffixes)) # Build only shared.

rtld-routines	:= rtld $(addprefix dl-,load lookup object reloc	\
				        runtime sysdep error init fini)
distribute	= $(rtld-routines:=.c) dynamic-link.h do-rel.h \
		  soinit.c sofini.c ldd.sh.in linux-compat.c

include ../Makeconfig

ifeq (yes,$(build-shared))
extra-objs	= $(rtld-routines:=.so) soinit.so sofini.so
generated	= librtld.so
install-others	= $(libdir)/$(rtld-installed-name)
install-bin	= ldd

ifneq (,$(filter linux%,$(config-os)))
extra-objs	+= linux-compat.so
install-lib	+= ld-linux.so.1
endif
endif

include ../Rules


# Link together the dynamic linker into a single relocatable object.
# We use this to produce both the ABI-compliant and Linux-compatible
# dynamic linker shared objects below.
$(objpfx)librtld.so: $(rtld-routines:%=$(objpfx)%.so) \
		     $(patsubst %,$(common-objpfx)lib%_pic.a,\
				elf c $(LDLIBS-c.so:-l%=%))
	$(LINK.o) -nostdlib -nostartfiles -r -o $@ \
		  '-Wl,-(' $^ -lgcc '-Wl,-)'

$(objpfx)ld.so $(objpfx)ld-linux.so.1: $(objpfx)librtld.so
	$(LINK.o) -nostdlib -nostartfiles -shared -o $@ $^

# The Linux-compatible dynamic linker shared object is just the same
# with one object file of compatibility initialization code added.
$(objpfx)ld-linux.so.1: $(objpfx)linux-compat.so


$(objpfx)libdl.so: $(objpfx)libdl_pic.a $(common-objpfx)libc.so $(objpfx)ld.so
	$(patsubst %/,cd %;,$(objpfx)) \
	$(LINK.o) -shared -o $(@:$(objpfx)%=%) \
		  $(LDFLAGS.so) $(LDFLAGS-dl.so) \
		  -Wl,--whole-archive $(^:$(objpfx)%=%)

$(libdir)$(rtld-installed-name): $(objpfx)ld.so; $(do-install-program)

$(objpfx)ldd: ldd.sh.in
	sed 's%@RTLD@%$(libdir)/$(rtld-installed-name)%g' < $< > $@.new
	mv -f $@.new $@
