# Copyright (C) 1995, 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
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

# Makefile for elf subdirectory of GNU C Library.

subdir		:= elf

headers		= elf.h bits/elfclass.h bits/dlfcn.h link.h dlfcn.h
routines	= $(dl-routines) dl-open dl-close dl-symbol dl-support \
		  dl-addr enbl-secure dl-profstub

# The core dynamic linking functions are in libc for the static and
# profiled libraries.
dl-routines	= $(addprefix dl-,load cache lookup object reloc deps \
			          runtime error init fini debug misc \
				  version profile)
# But they are absent from the shared libc, because that code is in ld.so.
elide-routines.os = $(dl-routines) dl-support enbl-secure

# ld.so uses those routines, plus some special stuff for being the program
# interpreter and operating independent of libc.
rtld-routines	:= rtld $(dl-routines) dl-sysdep dl-environ dl-minimal
distribute	= $(rtld-routines:=.c) dynamic-link.h do-rel.h dl-machine.h \
		  dl-hash.h soinit.c sofini.c ldd.bash.in eval.c \
		  genrtldtbl.awk atomicity.h dl-procinfo.h ldsodefs.h \
		  dl-librecon.h interp.c sln.c dl-origin.h hp-timing.h \
		  testobj1.c testobj2.c testobj3.c testobj4.c testobj5.c \
		  testobj6.c testobj1_1.c failobj.c

include ../Makeconfig

extra-libs	 = libdl
extra-libs-others = $(extra-libs)
libdl-routines	:= dlopen dlclose dlsym dlvsym dlerror dladdr
ifeq ($(versioning),yes)
libdl-routines	+= dlopenold
libdl-shared-only-routines := dlopenold
endif

before-compile = $(objpfx)trusted-dirs.h

ifeq ($(versioning),yes)
ld-map		= $(common-objpfx)libc.map
endif

ifeq (yes,$(build-shared))
extra-objs	= $(rtld-routines:=.os) soinit.os sofini.os eval.os interp.os
generated	= librtld.os dl-allobjs.os ld.so trusted-dirs.h trusted-dirs.st
install-others	= $(inst_slibdir)/$(rtld-installed-name)
install-bin	= ldd
generated      += ldd
endif

others		= sprof sln
install-bin	+= sprof
others-static   = sln
install-rootsbin = sln

ifeq (yes,$(has-ldconfig))
others-static	+= ldconfig
others		+= ldconfig
install-rootsbin += ldconfig
endif

tests = loadtest restest1 preloadtest loadfail

include ../Rules


ifeq (yes,$(build-shared))
# Make sure these things are built in the `make lib' pass so they can be used
# to run programs during the `make others' pass.
lib-noranlib: $(objpfx)$(rtld-installed-name) \
	      $(addprefix $(objpfx),$(extra-objs))
endif

# Command to link into a larger single relocatable object.
reloc-link = $(LINK.o) -nostdlib -nostartfiles -r -o $@

$(objpfx)dl-allobjs.os: $(rtld-routines:%=$(objpfx)%.os)
	$(reloc-link) $^

# Link together the dynamic linker into a single relocatable object.
$(objpfx)librtld.os: $(objpfx)dl-allobjs.os $(common-objpfx)libc_pic.a
	$(reloc-link) '-Wl,-(' $^ -lgcc '-Wl,-)'

# Do we need a linker script?
rtld-ldscript-in := $(firstword $(wildcard $(+sysdep_dirs:%=%/rtld-ldscript.in)))

ifneq (,$(rtld-ldscript-in))
rtld-ldscript = $(objpfx)rtld-ldscript
generated += rtld-ldscript

LDFLAGS-rtld = -T $(rtld-ldscript)
before-compile += $(rtld-ldscript)

rtld-parms = $(wildcard $(+sysdep_dirs:%=%/rtld-parms))
include $(rtld-parms)

$(rtld-ldscript): $(rtld-ldscript-in) $(rtld-parms)
	sed -e 's#@@rtld-oformat@@#$(rtld-oformat)#' \
	    -e 's#@@rtld-arch@@#$(rtld-arch)#' \
	    -e 's#@@rtld-entry@@#$(rtld-entry)#' \
	    -e 's#@@rtld-base@@#$(rtld-base)#' $< >$@
endif

$(objpfx)ld.so: $(objpfx)librtld.os $(addprefix $(objpfx),$(rtld-ldscript)) \
		$(ld-map)
	$(LINK.o) -nostdlib -nostartfiles -shared -o $@ $(LDFLAGS-rtld) \
		  $(filter-out $(rtld-ldscript) $(map-file),$^)		\
		  $(load-map-file) -Wl,-soname=$(rtld-installed-name)

# interp.c exists just to get this string into the libraries.
CFLAGS-interp.c = -D'RUNTIME_LINKER="$(slibdir)/$(rtld-installed-name)"'

ifneq (ld.so,$(rtld-installed-name))
# Make sure ld.so.1 exists in the build directory so we can link
# against it.
$(objpfx)$(rtld-installed-name): $(objpfx)ld.so
	rm -f $@
	ln -s $(<F) $@
generated += $(rtld-installed-name)
endif

# Build a file mentioning all trustworthy directories to look for shared
# libraries when using LD_LIBRARY_PATH in a setuid program.  The user can
# add directories to the list by defining $(user-defined-trusted-dirs)
# before starting make.
$(objpfx)trusted-dirs.h: $(objpfx)trusted-dirs.st; @:
$(objpfx)trusted-dirs.st: Makefile $(..)Makeconfig
	$(make-target-directory)
	dirs="$(subst :, ,$(default-rpath) $(user-defined-trusted-dirs))";   \
	for dir in $$dirs; do						     \
	  echo "  \"$$dir/\",";						     \
	done > ${@:st=T}
	$(move-if-change) ${@:st=T} ${@:st=h}
	touch $@
CPPFLAGS-dl-load.c = -I$(objpfx).
CFLAGS-dl-load.c += -Wno-uninitialized

# Specify the dependencies of libdl.so; its commands come from the generic
# rule to build a shared library.
$(objpfx)libdl.so: $(objpfx)libdl_pic.a $(common-objpfx)libc.so $(objpfx)ld.so

$(inst_slibdir)/$(rtld-version-installed-name): $(objpfx)ld.so $(+force)
	$(do-install-program)

$(inst_slibdir)/$(rtld-installed-name): \
  $(inst_slibdir)/$(rtld-version-installed-name)
	$(make-shlib-link)

# Special target called by parent to install just the dynamic linker.
.PHONY: ldso_install
ldso_install: $(inst_slibdir)/$(rtld-installed-name)


common-ldd-rewrite = -e 's%@RTLD@%$(slibdir)/$(rtld-installed-name)%g' \
		     -e 's%@VERSION@%$(version)%g'
sh-ldd-rewrite = $(common-ldd-rewrite) -e 's%@BASH@%/bin/sh%g;s/\$$"/"/g'
bash-ldd-rewrite = $(common-ldd-rewrite) -e 's%@BASH@%$(BASH)%g' \
		   -e 's%@TEXTDOMAINDIR@%$(localedir)%g'

ifneq ($(have-bash2),yes)
ldd-shell = sh
else
ldd-shell = bash
endif

ifeq ($(ldd-rewrite-script),no)
define gen-ldd
sed $($(ldd-shell)-ldd-rewrite) < $< > $@.new
endef
else
define gen-ldd
sed $($(ldd-shell)-ldd-rewrite) < $< | sed -f $(ldd-rewrite-script) > $@.new
endef
endif

$(objpfx)ldd: ldd.bash.in $(common-objpfx)soversions.mk \
	      $(common-objpfx)config.make
	$(gen-ldd)
	chmod 555 $@.new
	mv -f $@.new $@


ifeq ($(build-shared),yes)
$(objpfx)sprof: $(objpfx)libdl.so$(libdl.so-version)
else
$(objpfx)sprof: $(objpfx)libdl.a
endif

modules-names = testobj1 testobj2 testobj3 testobj4 testobj5 testobj6 \
		testobj1_1 failobj
test-modules = $(addprefix $(objpfx),$(addsuffix .so,$(modules-names)))
generated += $(test-modules)

LDLIBS-testobj1.so = -ldl
$(objpfx)testobj1_1.so: $(objpfx)testobj1.so
LDLIBS-testobj1_1.so = $(objpfx)testobj1.so
$(objpfx)testobj2.so: $(objpfx)testobj1.so
LDLIBS-testobj2.so = $(objpfx)testobj1.so -ldl
LDLIBS-testobj3.so = -ldl
LDLIBS-testobj4.so = -ldl
LDLIBS-testobj5.so = -ldl
LDLIBS-testobj6.so = -ldl
LDLIBS-failobj.so = $(objpfx)testobj6.so

$(test-modules): $(objpfx)%.so: $(objpfx)%.os
	$(build-module)

$(objpfx)loadtest: $(objpfx)libdl.so
LDFLAGS-loadtest = -rdynamic

$(objpfx)loadtest.out: $(test-modules)

$(objpfx)restest1: $(objpfx)libdl.so
LDFLAGS-restest1 = -rdynamic $(objpfx)testobj1.so $(objpfx)testobj1_1.so

$(objpfx)restest1.out: $(test-modules)

$(objpfx)preloadtest.out: $(test-modules)
LDFLAGS-preloadtest = -rdynamic $(objpfx)testobj6.so
preloadtest-ENV = LD_PRELOAD=testobj1.so:testobj2.so:testobj3.so:testobj4.so:testobj5.so

$(objpfx)loadfail: $(objpfx)libdl.so
LDFLAGS-loadfail = -rdynamic

$(objpfx)loadfile.out: $(objpfx)failobj.so

# muwahaha

LDFLAGS-dl.so = -Wl,-dynamic-linker,$(slibdir)/$(rtld-installed-name)
$(objpfx)libdl.so: $(objpfx)eval.os
