# Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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

#
#	Sub-makefile for MD5 crypt portion of the library.
#
subdir	:= md5-crypt

headers := crypt.h

distribute := md5.h

tests	:= md5test md5c-test

extra-libs := libcrypt
extra-libs-others := $(extra-libs)

md5-routines := crypt-entry md5-crypt md5
libcrypt-routines := $(md5-routines)

onlymd5-routines := onlymd5-entry md5-crypt md5
distribute += onlymd5-entry.c
extra-objs = $(patsubst %,onlymd5-entry%,$(object-suffixes))

include ../Makeconfig

rpath-dirs += md5-crypt

ifeq ($(crypt-in-libc),yes)
routines += $(libcrypt-routines)
endif

$(objpfx)md5test: $(objpfx)md5.o

include ../Rules

ifeq ($(build-shared),yes)
libdepend = $(common-objpfx)md5-crypt/libcrypt.so$(libcrypt.so-version)
else
libdepend = $(common-objpfx)md5-crypt/libcrypt.a
endif

$(objpfx)md5test $(objpfx)md5c-test: $(libdepend)

.PHONY: libmd5crypt
libmd5crypt: $(foreach o,$(object-suffixes), \
		    $(addprefix $(objpfx),$(patsubst %,$(libtype$o),md5crypt)))
ifeq ($(build-shared),yes)
libmd5crypt: $(objpfx)libmd5crypt.so
# Use the same soname as the real libcrypt, so that it can be used as a
# drop in replacement.
LDFLAGS-md5crypt.so = -Wl,-soname=lib$(libprefix)crypt.so$(libcrypt.so-version)
# We also use the same version script.
libmd5crypt-map := libcrypt.map
endif

define o-iterator-doit
$(objpfx)$(patsubst %,$(libtype$o),md5crypt): \
  $(onlymd5-routines:%=$(objpfx)%$o); $$(build-extra-lib)
endef
object-suffixes-left = $(object-suffixes)
include $(patsubst %,$(..)o-iterator.mk,$(object-suffixes))

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libcrypt.so: $(common-objpfx)libc.so
$(objpfx)libmd5crypt.so: $(common-objpfx)libc.so
