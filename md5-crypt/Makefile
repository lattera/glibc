# Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

libcrypt-routines := crypt-entry md5-crypt md5

include ../Makeconfig

rpath-link := $(common-objpfx)md5-crypt:$(rpath-link)

ifeq ($(crypt-in-libc),yes)
routines += $(libcrypt-routines)
endif

include ../Rules

ifeq ($(build-shared),yes)
libdepend = $(common-objpfx)md5-crypt/libcrypt.so$(libcrypt.so-version)
else
libdepend = $(common-objpfx)md5-crypt/libcrypt.a
endif

$(objpfx)md5test $(objpfx)md5c-test: $(libdepend)
