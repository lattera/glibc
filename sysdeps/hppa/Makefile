# Copyright (C) 2000, 2001 Free Software Foundation, Inc.
# This file is part of the GNU C Library.
# Contributed by David Huggins-Daines (dhd@debian.org)

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

# We used to need this since the build process uses ld -r.  Now we use
# ld -r --unique=.text* which does more or less the same thing, but better.
# CFLAGS-.os += -ffunction-sections
LDFLAGS-c_pic.os += -Wl,--unique=.text*

ifeq ($(subdir),malloc)
CFLAGS-malloc.c += -DMALLOC_ALIGNMENT=16
endif

ifeq ($(subdir),elf)
dl-routines += dl-symaddr dl-fptr
rtld-routines += dl-symaddr dl-fptr
endif
