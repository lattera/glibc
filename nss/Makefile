# Copyright (C) 1996,1997,1998,2000,2001,2002 Free Software Foundation, Inc.
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
#	Makefile for name service switch.
#
subdir	:= nss

headers			:= nss.h
distribute		:= nsswitch.h XXX-lookup.c getXXbyYY.c getXXbyYY_r.c \
			   getXXent.c getXXent_r.c databases.def \
			   nsswitch.conf digits_dots.c function.def

# This is the trivial part which goes into libc itself.
routines		= nsswitch getnssent getnssent_r digits_dots \
			  $(addsuffix -lookup,$(databases))

# These are the databases that go through nss dispatch.
# Caution: if you add a database here, you must add its real name
# in databases.def, too.
databases		= proto service hosts network grp pwd rpc ethers \
			  spwd netgrp key alias

others                  := getent
install-bin             := getent

tests			= test-netdb
xtests			= bug-erange

include ../Makeconfig

ifeq (yes,$(build-static-nss))
otherlibs += $(nssobjdir)/libnss_files.a $(resolvobjdir)/libnss_dns.a \
	     $(resolvobjdir)/libresolv.a
endif

# Specify rules for the nss_* modules.  We have some services.
services		:= files

extra-libs		= $(services:%=libnss_%)
# These libraries will be built in the `others' pass rather than
# the `lib' pass, because they depend on libc.so being built already.
extra-libs-others	= $(extra-libs)

# The sources are found in the appropriate subdir.
subdir-dirs = $(services:%=nss_%)
vpath %.c $(subdir-dirs)


libnss_files-routines	:= $(addprefix files-,$(databases))
distribute		+= files-XXX.c files-parse.c


# Build static module if requested
ifneq ($(build-static-nss),yes)
libnss_files-inhibit-o	= $(filter-out .os,$(object-suffixes))
endif

include ../Rules


ifeq (yes,$(build-static-nss))
$(objpfx)getent: $(objpfx)libnss_files.a
endif

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(services:%=$(objpfx)libnss_%.so): $(common-objpfx)libc.so \
				    $(common-objpfx)libc_nonshared.a
