# Copyright (C) 1995, 1996 Free Software Foundation, Inc.
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
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

#
#	Sub-makefile for gmon portion of the library.
#
subdir	:= gmon

headers	:= sys/gmon.h
distribute := machine-gmon.h
routines := gmon mcount profil

include ../Rules

# The mcount code won't work without a frame pointer.
CFLAGS-mcount.c := -fno-omit-frame-pointer

# We cannot compile mcount.c with -pg because that would
# create resursive calls.  Just copy the normal static object.
$(objpfx)mcount.po: $(objpfx)mcount.o
	rm -f $@
	ln $< $@
