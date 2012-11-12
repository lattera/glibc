# Copyright (C) 1996,2000,2001,2007,2009,2012 Free Software Foundation, Inc.
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

#
#	Sub-makefile for crypt() portion of the library.
#
subdir	:= crypt

headers := crypt.h

extra-libs := libcrypt
extra-libs-others := $(extra-libs)

libcrypt-routines := crypt-entry md5-crypt sha256-crypt sha512-crypt crypt \
		     crypt_util

tests := cert md5c-test sha256c-test sha512c-test badsalttest

include ../Makeconfig

ifeq ($(crypt-in-libc),yes)
routines += $(libcrypt-routines)
endif

ifeq ($(nss-crypt),yes)
CPPFLAGS-sha256-crypt.c = -DUSE_NSS -I$(shell nss-config --includedir)
CPPFLAGS-sha512-crypt.c = -DUSE_NSS -I$(shell nss-config --includedir)
CPPFLAGS-md5-crypt.c = -DUSE_NSS -I$(shell nss-config --includedir)
LDLIBS-crypt.so = -lfreebl3
else
libcrypt-routines += md5 sha256 sha512

tests += md5test sha256test sha512test

# The test md5test-giant uses up to 400 MB of RSS and runs on a fast
# machine over a minute.
xtests = md5test-giant
endif

include ../Rules

ifneq ($(nss-crypt),yes)
md5-routines := md5 $(filter md5%,$(libcrypt-sysdep_routines))
sha256-routines := sha256 $(filter sha256%,$(libcrypt-sysdep_routines))
sha512-routines := sha512 $(filter sha512%,$(libcrypt-sysdep_routines))

$(objpfx)md5test: $(patsubst %, $(objpfx)%.o,$(md5-routines))
$(objpfx)md5test-giant: $(patsubst %, $(objpfx)%.o,$(md5-routines))
$(objpfx)sha256test: $(patsubst %, $(objpfx)%.o,$(sha256-routines))
$(objpfx)sha512test: $(patsubst %, $(objpfx)%.o,$(sha512-routines))
endif

ifeq (yes,$(build-shared))
$(addprefix $(objpfx),$(tests)): $(objpfx)libcrypt.so
else
$(addprefix $(objpfx),$(tests)): $(objpfx)libcrypt.a
endif
ifeq (yes,$(build-bounded))
$(tests:%=$(objpfx)%-bp): $(objpfx)libcrypt_b.a
endif

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libcrypt.so: $(common-objpfx)libc.so $(common-objpfx)libc_nonshared.a
