# Copyright (C) 1996 Free Software Foundation, Inc.
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
# not, write to the Free Software Foundation, Inc.,
# 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#
#	Makefile for name service switch.
#
subdir	:= nss

distribute		:= nsswitch.h XXX-lookup.c getXXbyYY.c getXXbyYY_r.c \
			   getXXent.c getXXent_r.c

# This is the trivial part which goes into libc itself.
routines		= nsswitch $(addsuffix -lookup,$(databases))

# These are the databases that go through nss dispatch.
databases		= proto service host network grp pwd rpc ethers

# Specify rules for the nss_* modules.  We have some services.
services		:= files dns

extra-libs		= $(services:%=libnss_%)

# The sources are found in the appropriate subdir.
subdir-dirs = $(services:%=nss_%)
vpath %.c $(subdir-dirs)


libnss_files-routines	:= $(addprefix files-,$(databases))
distribute		+= files-XXX.c files-parse.c

libnss_dns-routines	:= dns-host dns-network

libnss_files-inhibit-o	= $(filter-out .so,$(object-suffixes))
libnss_dns-inhibit-o	= $(filter-out .so,$(object-suffixes))


include ../Rules


resobjdir := $(firstword $(objdir) $(patsubst ../$(subdir),.,$(..)resolv))
LDLIBS-nss_dns.so = -L$(resobjdir) -Wl,-rpath-link=$(resobjdir) -lresolv
