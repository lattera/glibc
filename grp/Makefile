# Copyright (C) 1991,1992,1996-2000,2003,2004,2012
# Free Software Foundation, Inc.
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
# License along with the GNU C Library; if not, see
# <http://www.gnu.org/licenses/>.

#
#	Sub-makefile for grp portion of the library.
#
subdir	:= grp
headers := grp.h

routines := fgetgrent initgroups setgroups \
	    getgrent getgrgid getgrnam putgrent \
	    getgrent_r getgrgid_r getgrnam_r fgetgrent_r

include ../Makeconfig

tests := testgrp

ifeq (yes,$(build-shared))
test-srcs :=  tst_fgetgrent
endif


include ../Rules

ifeq (yes,$(build-static-nss))
otherlibs += $(nssobjdir)/libnss_files.a $(resolvobjdir)/libnss_dns.a \
	     $(resolvobjdir)/libresolv.a
endif


ifeq ($(have-thread-library),yes)

CFLAGS-getgrgid_r.c = -DUSE_NSCD=1 -fexceptions
CFLAGS-getgrnam_r.c = -DUSE_NSCD=1 -fexceptions
CFLAGS-getgrent_r.c = -fexceptions
CFLAGS-getgrent.c = -fexceptions
CFLAGS-fgetgrent.c = -fexceptions
CFLAGS-fgetgrent_r.c = -fexceptions -D_IO_MTSAFE_IO
CFLAGS-putgrent.c = -fexceptions -D_IO_MTSAFE_IO
CFLAGS-initgroups.c = -DUSE_NSCD=1 -fexceptions
CFLAGS-getgrgid.c = -fexceptions

endif

ifeq (no,$(cross-compiling))
# tst_fgetgrent currently only works with shared libraries
ifeq (yes,$(build-shared))
tests: $(objpfx)tst_fgetgrent.out
$(objpfx)tst_fgetgrent.out: tst_fgetgrent.sh $(objpfx)tst_fgetgrent
	$(SHELL) -e tst_fgetgrent.sh $(common-objpfx) $(elf-objpfx) \
		 $(rtld-installed-name)

endif
endif
