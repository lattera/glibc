# Copyright (C) 1995, 1996, 1997, 2001, 2002 Free Software Foundation, Inc.
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

#
#	Sub-makefile for gmon portion of the library.
#
subdir	:= gmon

headers	:= sys/gmon.h sys/gmon_out.h sys/profil.h
distribute := machine-gmon.h profil-counter.h
routines := gmon mcount profil sprofil bb_init_func bb_exit_func prof-freq

elide-routines.os = bb_init_func bb_exit_func

tests	:= tst-sprofil

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
