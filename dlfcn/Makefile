# Copyright (C) 1995-2002, 2003, 2004 Free Software Foundation, Inc.
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
# License along with the GNU C Library; if not, write to the Free
# Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.

subdir		:= dlfcn
headers		:= bits/dlfcn.h dlfcn.h
extra-libs	:= libdl
libdl-routines	:= dlopen dlclose dlsym dlvsym dlerror dladdr dladdr1 dlinfo \
		   dlmopen
routines	:= $(patsubst %,s%,$(libdl-routines))
elide-routines.os := $(routines)
distribute	:= dlopenold.c glreflib1.c glreflib2.c failtestmod.c \
		   defaultmod1.c defaultmod2.c errmsg1mod.c modatexit.c \
		   modcxaatexit.c modstatic.c modstatic2.c \
		   bug-dlsym1-lib1.c bug-dlsym1-lib2.c

extra-libs-others := libdl

include ../Makeconfig

ifeq ($(versioning),yes)
libdl-routines	+= dlopenold
libdl-shared-only-routines := dlopenold
endif

ifeq (yes,$(build-shared))
tests = glrefmain failtest tst-dladdr default errmsg1 tstcxaatexit \
	bug-dlopen1 bug-dlsym1 tst-dlinfo
ifeq (yes,$(have-protected))
tests += tstatexit
endif
endif
modules-names = glreflib1 glreflib2 failtestmod defaultmod1 defaultmod2 \
		errmsg1mod modatexit modcxaatexit \
		bug-dlsym1-lib1 bug-dlsym1-lib2

failtestmod.so-no-z-defs = yes
glreflib2.so-no-z-defs = yes
errmsg1mod.so-no-z-defs = yes

ifeq (yesyesyes,$(build-static)$(build-shared)$(elf))
tests += tststatic tststatic2
tests-static += tststatic tststatic2
modules-names += modstatic modstatic2
tststatic-ENV = LD_LIBRARY_PATH=$(objpfx):$(common-objpfx):$(common-objpfx)elf
tststatic2-ENV = LD_LIBRARY_PATH=$(objpfx):$(common-objpfx):$(common-objpfx)elf
endif

extra-objs += $(modules-names:=.os)
generated := $(modules-names:=.so)

include ../Rules

LDFLAGS-dl.so = -Wl,-dynamic-linker,$(slibdir)/$(rtld-installed-name)

test-modules = $(addprefix $(objpfx),$(addsuffix .so,$(modules-names)))
$(test-modules): $(objpfx)%.so: $(objpfx)%.os $(common-objpfx)shlib.lds
	$(build-module)

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
$(objpfx)tst-dlinfo.out: $(objpfx)glreflib1.so

LDFLAGS-default = $(LDFLAGS-rdynamic)
$(objpfx)default: $(libdl) $(objpfx)defaultmod1.so $(objpfx)defaultmod2.so
$(objpfx)defaultmod1.so: $(libdl) $(common-objpfx)libc_nonshared.a
LDFLAGS-defaultmod2.so = $(LDFLAGS-Bsymbolic)
$(objpfx)defaultmod2.so: $(libdl) $(common-objpfx)libc_nonshared.a

$(objpfx)errmsg1: $(libdl)
$(objpfx)errmsg1.out: $(objpfx)errmsg1 $(objpfx)errmsg1mod.so

$(objpfx)tstatexit: $(libdl)
$(objpfx)tstatexit.out: $(objpfx)tstatexit $(objpfx)modatexit.so

$(objpfx)tstcxaatexit: $(libdl)
$(objpfx)tstcxaatexit.out: $(objpfx)tstcxaatexit $(objpfx)modcxaatexit.so

$(objpfx)modatexit.so: $(common-objpfx)libc.so $(common-objpfx)libc_nonshared.a

$(objpfx)tststatic: $(objpfx)libdl.a
$(objpfx)tststatic.out: $(objpfx)tststatic $(objpfx)modstatic.so

$(objpfx)tststatic2: $(objpfx)libdl.a
$(objpfx)tststatic2.out: $(objpfx)tststatic2 $(objpfx)modstatic.so \
			 $(objpfx)modstatic2.so

$(objpfx)modstatic2.so: $(libdl) $(common-objpfx)libc.so \
			$(common-objpfx)libc_nonshared.a

$(objpfx)bug-dlopen1: $(libdl)

$(objpfx)bug-dlsym1: $(libdl) $(objpfx)bug-dlsym1-lib2.so
$(objpfx)bug-dlsym1.out: $(objpfx)bug-dlsym1-lib1.so \
			 $(objpfx)bug-dlsym1-lib2.so
$(objpfx)bug-dlsym1-lib1.so: $(objpfx)bug-dlsym1-lib2.so \
			     $(common-objpfx)libc.so \
			     $(common-objpfx)libc_nonshared.a
$(objpfx)bug-dlsym1-lib2.so: $(common-objpfx)libc.so \
			     $(common-objpfx)libc_nonshared.a



# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libdl.so: $(common-objpfx)libc.so $(common-objpfx)libc_nonshared.a \
		   $(if $(filter yes,$(elf)), $(elfobjdir)/ld.so)
