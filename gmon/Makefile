# Copyright (C) 1995-1997,2001,2002,2005,2012 Free Software Foundation, Inc.
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
#	Sub-makefile for gmon portion of the library.
#
subdir	:= gmon

headers	:= sys/gmon.h sys/gmon_out.h sys/profil.h
routines := gmon mcount profil sprofil bb_init_func bb_exit_func prof-freq

elide-routines.os = bb_init_func bb_exit_func

include ../Makeconfig

tests	= tst-sprofil
ifeq ($(build-profile),yes)
tests	+= tst-profile-static
tests-static	+= tst-profile-static

LDFLAGS-tst-profile-static = -profile
endif

# The mcount code won't work without a frame pointer.
CFLAGS-mcount.c := -fno-omit-frame-pointer

include ../Rules

# We cannot compile mcount.c with -pg because that would
# create recursive calls.  Just copy the normal static object.
# On systems where `profil' is not a system call, the same
# problem exists for the internal functions in profil.c.

noprof := mcount
ifeq (,$(filter profil,$(unix-syscalls)))
noprof += profil sprofil
endif

$(noprof:%=$(objpfx)%.op): %.op: %.o
	rm -f $@
	ln $< $@
