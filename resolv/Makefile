# Copyright (C) 1994 Free Software Foundation, Inc.
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
#	Sub-makefile for resolv portion of the library.
#
subdir	:= resolv

headers	:= resolv.h arpa/nameser.h sys/bitypes.h
distribute := ../conf/portability.h

routines := gethnamaddr getnetbyaddr getnetbyname getnetent getnetnamadr \
	    herror nsap_addr res_comp res_debug res_init res_mkquery \
	    res_query res_send sethostent

include ../Rules
