# Copyright (C) 1995-1999, 2000, 2001 Free Software Foundation, Inc.
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

subdir		:= dlfcn
headers		:= bits/dlfcn.h dlfcn.h
extra-libs	:= libdl
libdl-routines	:= dlopen dlclose dlsym dlvsym dlerror dladdr eval
distribute	:= dlopenold.c glreflib1.c glreflib2.c failtestmod.c eval.c \
		   defaultmod1.c defaultmod2.c errmsg1mod.c modatexit.c \
		   modcxaatexit.c

extra-libs-others := libdl

include ../Makeconfig

ifeq ($(versioning),yes)
libdl-routines	+= dlopenold
libdl-shared-only-routines := dlopenold
endif
libdl-shared-only-routines += eval

ifeq (yes,$(build-shared))
tests = glrefmain failtest tst-dladdr default errmsg1 tstatexit tstcxaatexit
endif
modules-names = glreflib1 glreflib2 failtestmod defaultmod1 defaultmod2 \
		errmsg1mod modatexit modcxaatexit
extra-objs += $(modules-names:=.os) eval.os
generated := $(modules-names:=.so)

include ../Rules

LDFLAGS-dl.so = -Wl,-dynamic-linker,$(slibdir)/$(rtld-installed-name)

test-modules = $(addprefix $(objpfx),$(addsuffix .so,$(modules-names)))
$(test-modules): $(objpfx)%.so: $(objpfx)%.os
	$(build-module)

$(objpfx)glrefmain: $(libdl)
$(objpfx)glrefmain.out: $(objpfx)glrefmain \
			$(objpfx)glreflib1.so $(objpfx)glreflib2.so

$(objpfx)failtest: $(libdl)
$(objpfx)failtest.out: $(objpfx)failtestmod.so

$(objpfx)tst-dladdr: $(libdl)
$(objpfx)tst-dladdr.out: $(objpfx)glreflib1.so

LDFLAGS-default = -rdynamic
$(objpfx)default: $(libdl) $(objpfx)defaultmod1.so $(objpfx)defaultmod2.so
$(objpfx)defaultmod1.so: $(libdl)
LDFLAGS-defaultmod2.so = -Bsymbolic
$(objpfx)defaultmod2.so: $(libdl)

$(objpfx)errmsg1: $(libdl)
$(objpfx)errmsg1.out: $(objpfx)errmsg1 $(objpfx)errmsg1mod.so

$(objpfx)tstatexit: $(libdl)
$(objpfx)tstatexit.out: $(objpfx)tstatexit $(objpfx)modatexit.so

$(objpfx)tstcxaatexit: $(libdl)
$(objpfx)tstcxaatexit.out: $(objpfx)tstcxaatexit $(objpfx)modcxaatexit.so

$(objpfx)modatexit.so: $(common-objpfx)libc.so $(common-objpfx)libc_nonshared.a
