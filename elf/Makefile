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
LDFLAGS-dl.so	:= -e 0 # work around ld bug

rtld-routines	:= rtld $(addprefix dl-,load lookup object reloc	\
				        runtime sysdep error init fini)
distribute	= $(rtld-routines:=.c) dynamic-link.h do-rel.h \
		  soinit.c sofini.c

include ../Makeconfig

ifeq (yes,$(build-shared))
extra-objs	= $(rtld-routines:=.so) soinit.so sofini.so
install-lib	= ld.so
endif

include ../Rules

$(objpfx)ld.so: $(rtld-routines:%=$(objpfx)%.so) \
		$(patsubst %,$(common-objpfx)lib%_pic.a,\
			   elf c $(LDLIBS-c.so:-l%=%))
	$(LINK.o) -nostdlib -nostartfiles -shared -o $@ \
		  '-Wl,-(' $^ -lgcc '-Wl,-)'

$(objpfx)libdl.so: $(objpfx)libdl_pic.a $(common-objpfx)libc.so $(objpfx)ld.so
	$(patsubst %/,cd %;,$(objpfx)) \
	$(LINK.o) -shared -o $(@:$(objpfx)%=%) \
		  $(LDFLAGS.so) $(LDFLAGS-dl.so) \
		  -Wl,--whole-archive $(^:$(objpfx)%=%)
