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
# License along with the GNU C Library; see the file COPYING.LIB.  If not,
# write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.

#
#	Makefile for NIS part.
#
subdir	:= nis

headers			:= $(wildcard rpcsvc/*.[hx])

# These are the databases available for the nis (and perhaps later nisplus)
# service.  This must be a superset of the services in nss.
databases		= proto service hosts network grp pwd rpc ethers \
			  spwd netgrp alias

# Specify rules for the nss_* modules.  Later we may have nisplus as well.
services		:= nis compat

extra-libs		= libnsl $(services:%=libnss_%)
# These libraries will be built in the `others' pass rather than
# the `lib' pass, because they depend on libc.so being built already.
extra-libs-others	= $(extra-libs)

# The sources are found in the appropriate subdir.
subdir-dirs = $(services:%=nss_%)
vpath %.c $(subdir-dirs)

libnsl-routines = yp_xdr ypclnt ypupdate_xdr

libnss_compat-routines	:= $(addprefix compat-,grp pwd spwd)
libnss_compat-inhibit-o	= $(filter-out .so,$(object-suffixes))

libnss_nis-routines	:= $(addprefix nis-,$(databases))
libnss_nis-inhibit-o	= $(filter-out .so,$(object-suffixes))


# Sun's header files are not too clean.
CFLAGS-compat-pwd.c = -Wno-strict-prototypes
CFLAGS-compat-spwd.c = -Wno-strict-prototypes
CFLAGS-compat-grp.c = -Wno-strict-prototypes
CFLAGS-nis-alias.c = -Wno-strict-prototypes
CFLAGS-nis-ethers.c = -Wno-strict-prototypes
CFLAGS-nis-grp.c = -Wno-strict-prototypes
CFLAGS-nis-hosts.c = -Wno-strict-prototypes
CFLAGS-nis-netgrp.c = -Wno-strict-prototypes
CFLAGS-nis-network.c = -Wno-strict-prototypes
CFLAGS-nis-proto.c = -Wno-strict-prototypes
CFLAGS-nis-publickey.c = -Wno-strict-prototypes
CFLAGS-nis-pwd.c = -Wno-strict-prototypes
CFLAGS-nis-rpc.c = -Wno-strict-prototypes
CFLAGS-nis-service.c = -Wno-strict-prototypes
CFLAGS-nis-spwd.c = -Wno-strict-prototypes
CFLAGS-ypclnt.c = -Wno-strict-prototypes -Wno-write-strings -Irpcsvc
CFLAGS-yp_xdr.c = -Wno-strict-prototypes -Irpcsvc
CFLAGS-ypupdate_xdr.c = -Wno-strict-prototypes -Irpcsvc

include ../Rules


$(objpfx)libnss_compat.so: $(objpfx)libnsl.so$(libnsl.so-version) \
			   $(common-objpfx)nss/libnss_files.so
$(objpfx)libnss_nis.so: $(objpfx)libnsl.so$(libnsl.so-version) \
			$(common-objpfx)nss/libnss_files.so

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(services:%=$(objpfx)libnss_%.so): $(common-objpfx)libc.so


ifeq ($(build-shared),yes)
$(others:%=$(objpfx)%): $(objpfx)libnsl.so$(libnsl.so-version)
else
$(others:%=$(objpfx)%): $(objpfx)libnsl.a
endif
