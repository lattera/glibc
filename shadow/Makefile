# Copyright (C) 1996, 2003, 2004 Free Software Foundation, Inc.
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
#	Makefile for shadow.
#
subdir	:= shadow

headers		= shadow.h
routines	= getspent getspnam sgetspent fgetspent putspent \
		  getspent_r getspnam_r sgetspent_r fgetspent_r \
		  lckpwdf

CFLAGS-getspent_r.c = -fexceptions
CFLAGS-getspent.c = -fexceptions
CFLAGS-fgetspent.c = -fexceptions
CFLAGS-fgetspent_r.c = -fexceptions -D_IO_MTSAFE_IO
CFLAGS-putspent.c = -fexceptions -D_IO_MTSAFE_IO
CFLAGS-getspnam.c = -fexceptions
CFLAGS-getspnam_r.c = -fexceptions

include ../Rules
