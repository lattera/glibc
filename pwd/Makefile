# Copyright (C) 1991,1996-1999,2003,2004 Free Software Foundation, Inc.
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
#	Sub-makefile for pwd portion of the library.
#
subdir	:= pwd
headers := pwd.h

routines := fgetpwent getpw putpwent \
	    getpwent getpwnam getpwuid \
	    getpwent_r getpwnam_r getpwuid_r fgetpwent_r

tests := tst-getpw

include ../Rules

ifeq ($(have-thread-library),yes)

CFLAGS-getpwuid_r.c = -DUSE_NSCD=1
CFLAGS-getpwnam_r.c = -DUSE_NSCD=1
CFLAGS-getpwent_r.c = -fexceptions
CFLAGS-getpwent.c = -fexceptions
CFLAGS-getpw.c = -fexceptions
CFLAGS-fgetpwent_r.c = -D_IO_MTSAFE_IO

endif

ifeq (yes,$(build-static-nss))
otherlibs += $(nssobjdir)/libnss_files.a $(resolvobjdir)/libnss_dns.a \
             $(resolvobjdir)/libresolv.a
endif
