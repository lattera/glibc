# Copyright (C) 1991-2000, 2002, 2003 Free Software Foundation, Inc.
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
#	Sub-makefile for dirent portion of the library.
#
subdir		:= dirent

headers		:= dirent.h bits/dirent.h
routines	:= opendir closedir readdir readdir_r rewinddir \
		   seekdir telldir scandir alphasort versionsort \
		   getdents getdents64 dirfd readdir64 readdir64_r scandir64 \
		   alphasort64 versionsort64
distribute := dirstream.h

tests	   := list tst-seekdir opendir-tst1 bug-readdir1

CFLAGS-scandir.c = $(uses-callbacks)
CFLAGS-scandir64.c = $(uses-callbacks)

include ../Rules

opendir-tst1-ARGS = --test-dir=${common-objpfx}dirent
