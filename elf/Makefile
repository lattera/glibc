# Makefile for elf subdirectory of GNU C Library.

# Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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
# License along with the GNU C Library; see the file COPYING.LIB.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

subdir		:= elf

headers		= elf.h elfclass.h link.h dlfcn.h
routines	= $(dl-routines) dl-open dl-close dl-symbol dl-support \
		  dl-version enbl-secure

# The core dynamic linking functions are in libc for the static and
# profiled libraries.
dl-routines	= $(addprefix dl-,load cache lookup object reloc deps \
			          runtime error init fini debug misc)
# But they are absent from the shared libc, because that code is in ld.so.
elide-routines.so = $(dl-routines) dl-support enbl-secure

# ld.so uses those routines, plus some special stuff for being the program
# interpreter and operating independent of libc.
rtld-routines	:= rtld $(dl-routines) dl-sysdep dl-minimal
distribute	= $(rtld-routines:=.c) dynamic-link.h do-rel.h dl-machine.h \
		  soinit.c sofini.c ldd.sh.in ldd.bash.in eval.c

extra-libs	 = libdl
extra-libs-others = $(extra-libs)
libdl-routines	:= dlopen dlclose dlsym dlvsym dlerror dladdr
libdl-map	:= libdl.map

before-compile = $(objpfx)trusted-dirs.h


all: # Make this the default target; it will be defined in Rules.

include ../Makeconfig

ifeq (yes,$(build-shared))
extra-objs	= $(rtld-routines:=.so) soinit.so sofini.so eval.so
generated	= librtld.so dl-allobjs.so
install-others	= $(inst_slibdir)/$(rtld-installed-name)
install-bin	= ldd
endif

ifeq (yes,$(has-ldconfig))
extra-objs	+= ldconfig.o
install-rootsbin = ldconfig

$(objpfx)ldconfig: $(objpfx)ldconfig.o
	$(CC) -nostdlib -nostartfiles -o $@ $(sysdep-LDFLAGS) -static	      \
	      $(addprefix $(csu-objpfx),start.o) $(+preinit)		      \
	      $(filter-out $(addprefix $(csu-objpfx),start.o) $(+preinit)     \
		$(link-extra-libs) $(common-objpfx)libc% $(+postinit),$^)     \
	      $(link-extra-libs) $(common-objpfx)libc.a $(gnulib) $(+postinit)
endif

include ../Rules


ifeq (yes,$(build-shared))
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
$(objpfx)librtld.so: $(objpfx)dl-allobjs.so $(common-objpfx)libc_pic.a
	$(reloc-link) '-Wl,-(' $^ -lgcc '-Wl,-)'

$(objpfx)ld.so: $(objpfx)librtld.so
	$(rtld-link) -Wl,-soname=$(rtld-installed-name)

define rtld-link
$(LINK.o) -nostdlib -nostartfiles -shared -o $@ \
	  -Wl,-rpath=$(default-rpath) $^
endef

# The dl code in the static libc needs a default library path.
CFLAGS-dl-support.c = -D'DEFAULT_RPATH="$(default-rpath)"'

ifneq (ld.so,$(rtld-installed-name))
# Make sure ld.so.1 exists in the build directory so we can link
# against it.
$(objpfx)$(rtld-installed-name): $(objpfx)ld.so
	rm -f $@
	ln -s $(<F) $@
endif

# Build a file mentioning all trustworthy directories to look for shared
# libraries when using LD_LIBRARY_PATH in a setuid program.  The user can
# add directories to the list by defining $(user-defined-trusted-dirs)
# before starting make.
$(objpfx)trusted-dirs.h: Makefile
	$(make-target-directory)
	(for dir in `echo "$(default-rpath) $(user-defined-trusted-dirs)" |   \
		     sed 's/:/ /g'`; do					      \
	   echo "  \"$$dir\",";						      \
	 done;) > $@T
	mv -f $@T $@
CPPFLAGS-dl-load.c = -I$(objdir)/$(subdir)
CFLAGS-dl-load.c += -Wno-uninitialized

# Specify the dependencies of libdl.so; its commands come from the generic
# rule to build a shared library.
$(objpfx)libdl.so: $(objpfx)libdl_pic.a $(common-objpfx)libc.so $(objpfx)ld.so

$(inst_slibdir)/$(rtld-version-installed-name): $(objpfx)ld.so
	$(do-install-program)

$(inst_slibdir)/$(rtld-installed-name): \
  $(inst_slibdir)/$(rtld-version-installed-name)
	$(make-shlib-link)

ifneq ($(have-bash2),yes)
$(objpfx)ldd: ldd.sh.in $(common-objpfx)version.mk \
	      $(common-objpfx)soversions.mk $(common-objpfx)config.make
	sed -e 's%@RTLD@%$(slibdir)/$(rtld-installed-name)%g' \
	    -e 's%@VERSION@%$(version)%g' < $< > $@.new
	chmod 555 $@.new
	mv -f $@.new $@
else
$(objpfx)ldd: ldd.bash.in $(common-objpfx)version.mk \
	      $(common-objpfx)soversions.mk $(common-objpfx)config.make
	sed -e 's%@BASH@%$(BASH)%g' \
	    -e 's%@RTLD@%$(slibdir)/$(rtld-installed-name)%g' \
	    -e 's%@VERSION@%$(version)%g' \
	    -e 's%@TEXTDOMAINDIR@%$(localedir)%g' < $< > $@.new
	chmod 555 $@.new
	mv -f $@.new $@
endif

# muwahaha

LDFLAGS-dl.so = -Wl,-dynamic-linker,$(slibdir)/$(rtld-installed-name)
$(objpfx)libdl.so: $(objpfx)eval.so
