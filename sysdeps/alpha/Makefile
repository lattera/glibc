# Copyright (C) 1993, 1994, 1995, 1996 Free Software Foundation, Inc.
# Contributed by Brendan Kehoe (brendan@zen.org).

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

ifeq ($(subdir),gmon)
sysdep_routines += _mcount
endif

ifeq ($(subdir),setjmp)
sysdep_routines := $(sysdep_routines) setjmp_aux
endif

ifeq ($(subdir),gnulib)
routines = $(divrem)
endif	# gnulib

ifeq ($(subdir),elf)
# The ld.so code cannot use literals until it self-relocates.
CFLAGS-rtld.c = -mbuild-constants
# The rest of ld.so shouldn't use FP regs for block moves so
# that the lazy link trampoline doesn't have to save them.
sysdep-CFLAGS += -mno-fp-regs
endif

divrem := divl divlu divq divqu reml remlu remq remqu
