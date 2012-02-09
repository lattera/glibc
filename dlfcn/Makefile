# Copyright (C) 1995-2006, 2010, 2012 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

subdir		:= dlfcn
headers		:= bits/dlfcn.h dlfcn.h
extra-libs	:= libdl
libdl-routines	:= dlopen dlclose dlsym dlvsym dlerror dladdr dladdr1 dlinfo \
		   dlmopen dlfcn
routines	:= $(patsubst %,s%,$(filter-out dlfcn,$(libdl-routines)))
elide-routines.os := $(routines)
distribute	:= dlopenold.c glreflib1.c glreflib2.c glreflib3.c \
		   failtestmod.c \
		   defaultmod1.c defaultmod2.c errmsg1mod.c modatexit.c \
		   modcxaatexit.c modstatic.c modstatic2.c \
		   bug-dlsym1-lib1.c bug-dlsym1-lib2.c bug-atexit1-lib.c \
		   bug-atexit2-lib.c

extra-libs-others := libdl

include ../Makeconfig

ifeq ($(versioning),yes)
libdl-routines	+= dlopenold
libdl-shared-only-routines := dlopenold dlfcn
endif

ifeq (yes,$(build-shared))
tests = glrefmain failtest tst-dladdr default errmsg1 tstcxaatexit \
	bug-dlopen1 bug-dlsym1 tst-dlinfo bug-atexit1 bug-atexit2 \
	bug-atexit3 tstatexit
endif
modules-names = glreflib1 glreflib2 glreflib3 failtestmod defaultmod1 \
		defaultmod2 errmsg1mod modatexit modcxaatexit \
		bug-dlsym1-lib1 bug-dlsym1-lib2 bug-atexit1-lib \
		bug-atexit2-lib bug-atexit3-lib

failtestmod.so-no-z-defs = yes
glreflib2.so-no-z-defs = yes
errmsg1mod.so-no-z-defs = yes

ifeq (yes,$(build-shared))
tests += tststatic tststatic2
tests-static += tststatic tststatic2
modules-names += modstatic modstatic2
tststatic-ENV = LD_LIBRARY_PATH=$(objpfx):$(common-objpfx):$(common-objpfx)elf
tststatic2-ENV = LD_LIBRARY_PATH=$(objpfx):$(common-objpfx):$(common-objpfx)elf
endif

extra-test-objs += $(modules-names:=.os)
generated := $(modules-names:=.so)

include ../Rules

test-modules = $(addprefix $(objpfx),$(addsuffix .so,$(modules-names)))

ifeq ($(build-shared),yes)
# Build all the modules even when not actually running test programs.
tests: $(test-modules)
endif

$(objpfx)glrefmain: $(libdl)
$(objpfx)glrefmain.out: $(objpfx)glrefmain \
			$(objpfx)glreflib1.so $(objpfx)glreflib2.so

$(objpfx)failtest: $(libdl)
$(objpfx)failtest.out: $(objpfx)failtestmod.so

$(objpfx)tst-dladdr: $(libdl)
$(objpfx)tst-dladdr.out: $(objpfx)glreflib1.so

$(objpfx)tst-dlinfo: $(libdl)
$(objpfx)tst-dlinfo.out: $(objpfx)glreflib3.so
LDFLAGS-glreflib3.so = -Wl,-rpath,:

LDFLAGS-default = $(LDFLAGS-rdynamic)
$(objpfx)default: $(libdl) $(objpfx)defaultmod1.so $(objpfx)defaultmod2.so
$(objpfx)defaultmod1.so: $(libdl)
LDFLAGS-defaultmod2.so = $(LDFLAGS-Bsymbolic)
$(objpfx)defaultmod2.so: $(libdl)

$(objpfx)errmsg1: $(libdl)
$(objpfx)errmsg1.out: $(objpfx)errmsg1 $(objpfx)errmsg1mod.so

$(objpfx)tstatexit: $(libdl)
$(objpfx)tstatexit.out: $(objpfx)tstatexit $(objpfx)modatexit.so

$(objpfx)tstcxaatexit: $(libdl)
$(objpfx)tstcxaatexit.out: $(objpfx)tstcxaatexit $(objpfx)modcxaatexit.so

$(objpfx)tststatic: $(objpfx)libdl.a
$(objpfx)tststatic.out: $(objpfx)tststatic $(objpfx)modstatic.so

$(objpfx)tststatic2: $(objpfx)libdl.a
$(objpfx)tststatic2.out: $(objpfx)tststatic2 $(objpfx)modstatic.so \
			 $(objpfx)modstatic2.so

$(objpfx)modstatic2.so: $(libdl)

$(objpfx)bug-dlopen1: $(libdl)

$(objpfx)bug-dlsym1: $(libdl) $(objpfx)bug-dlsym1-lib2.so
$(objpfx)bug-dlsym1.out: $(objpfx)bug-dlsym1-lib1.so \
			 $(objpfx)bug-dlsym1-lib2.so
$(objpfx)bug-dlsym1-lib1.so: $(objpfx)bug-dlsym1-lib2.so

$(objpfx)bug-atexit1: $(libdl)
$(objpfx)bug-atexit1.out: $(objpfx)bug-atexit1-lib.so

$(objpfx)bug-atexit2: $(libdl)
$(objpfx)bug-atexit2.out: $(objpfx)bug-atexit2-lib.so

LDLIBS-bug-atexit3-lib.so = -lstdc++ -lgcc_eh $(elfobjdir)/ld.so \
			    $(common-objpfx)libc_nonshared.a
$(objpfx)bug-atexit3: $(libdl)
$(objpfx)bug-atexit3.out: $(objpfx)bug-atexit3-lib.so


# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libdl.so: $(common-objpfx)libc.so $(common-objpfx)libc_nonshared.a \
		   $(elfobjdir)/ld.so
