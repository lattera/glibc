# Copyright (C) 1992,93,94,95,96,97,99,2002,2005
#	Free Software Foundation, Inc.
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

ifeq ($(subdir),string)
CFLAGS-wordcopy.c += -Wno-uninitialized
endif

ifeq ($(subdir),elf)
ifeq (yes:yes,$(build-shared):$(unwind-find-fde))
# This is needed to support g++ v2 and v3.
sysdep_routines += framestate unwind-pe
shared-only-routines += framestate unwind-pe
endif
endif
