# Copyright (C) 1993-2015 Free Software Foundation, Inc.
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
# License along with the GNU C Library.  If not, see
# <http://www.gnu.org/licenses/>.

# The mpn functions need this.  All existing 68k ports use MIT syntax.  If
# a new port wants to use Motorola or Sony syntax, it can redefine this
# variable.
ifndef m68k-syntax-flag
m68k-syntax-flag = -DMIT_SYNTAX
endif

asm-CPPFLAGS += $(m68k-syntax-flag)

pic-ccflag = -fpic

ifeq ($(subdir),setjmp)
# Make sure setjmp.c is compiled with a frame pointer
CFLAGS-setjmp.c := -fno-omit-frame-pointer
endif

ifeq ($(subdir),debug)
CFLAGS-backtrace.c += -funwind-tables
endif

ifeq ($(subdir),elf)
CFLAGS-rtld.c += -Wno-uninitialized -Wno-unused
endif
