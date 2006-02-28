# Copyright (C) 2003, 2004, 2005, 2006 Free Software Foundation, Inc.
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

# Makefile for libidn subdirectory of GNU C Library.

subdir	:= libidn

distribute := punycode.h stringprep.h idna.h iconvme.h

routines = idn-stub

extra-libs		= libcidn
extra-libs-others	= $(extra-libs)

libcidn-routines := punycode toutf8 nfkc stringprep rfc3454 profiles idna \
		    iconvme


include $(..)Makeconfig

libcidn-inhibit-o = $(filter-out .os,$(object-suffixes))

include $(..)Rules

$(objpfx)libcidn.so: $(common-objpfx)libc.so $(common-objpfx)libc_nonshared.a
