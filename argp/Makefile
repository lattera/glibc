# Copyright (C) 1997, 2002, 2003 Free Software Foundation, Inc.
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
#	Makefile for argp.
#
subdir	:= argp

headers		= argp.h
distribute	= argp-fmtstream.h argp-namefrob.h
routines	= $(addprefix argp-, ba fmtstream fs-xinl help parse pv \
				     pvh xinl eexst)

tests		= argp-test tst-argp1

CFLAGS-argp-help.c = $(uses-callbacks) -fexceptions
CFLAGS-argp-parse.c = $(uses-callbacks)
CFLAGS-argp-fmtstream.c = -fexceptions

include ../Rules
