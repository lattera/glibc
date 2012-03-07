# Copyright (C) 1996-1998,2001,2004,2006,2011,2012
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
#	Makefile for NIS/NIS+ part.
#
subdir	:= nis

aux			:= nis_hash

# These are the databases available for the nis (and perhaps later nisplus)
# service.  This must be a superset of the services in nss.
databases		= proto service hosts network grp pwd rpc ethers \
			  spwd netgrp alias publickey

# Specify rules for the nss_* modules.
services		:= nis nisplus compat

extra-libs		= libnsl $(services:%=libnss_%)
# These libraries will be built in the `others' pass rather than
# the `lib' pass, because they depend on libc.so being built already.
extra-libs-others	= $(extra-libs)

# The sources are found in the appropriate subdir.
subdir-dirs = $(services:%=nss_%)
vpath %.c $(subdir-dirs)

libnsl-routines = yp_xdr ypclnt ypupdate_xdr \
		  nis_subr nis_local_names nis_free nis_file \
		  nis_print nis_error nis_call nis_lookup\
		  nis_table nis_xdr nis_server nis_ping \
		  nis_checkpoint nis_mkdir nis_rmdir nis_getservlist\
		  nis_verifygroup nis_ismember nis_addmember nis_util\
		  nis_removemember nis_creategroup nis_destroygroup\
		  nis_print_group_entry nis_domain_of nis_domain_of_r\
		  nis_modify nis_remove nis_add nis_defaults\
		  nis_findserv nis_callback nis_clone_dir nis_clone_obj\
		  nis_clone_res nss-default

libnss_compat-routines	:= $(addprefix compat-,grp pwd spwd initgroups)
libnss_compat-inhibit-o	= $(filter-out .os,$(object-suffixes))

libnss_nis-routines	:= $(addprefix nis-,$(databases)) nis-initgroups \
			   nss-nis
libnss_nis-inhibit-o	= $(filter-out .os,$(object-suffixes))

libnss_nisplus-routines	:= $(addprefix nisplus-,$(databases)) nisplus-parser \
			   nss-nisplus nisplus-initgroups
libnss_nisplus-inhibit-o = $(filter-out .os,$(object-suffixes))

include ../Rules


$(objpfx)libnss_compat.so: $(objpfx)libnsl.so$(libnsl.so-version)
$(objpfx)libnss_nis.so: $(objpfx)libnsl.so$(libnsl.so-version) \
			$(common-objpfx)nss/libnss_files.so
$(objpfx)libnss_nisplus.so: $(objpfx)libnsl.so$(libnsl.so-version)

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
libnsl-libc = $(common-objpfx)linkobj/libc.so
$(services:%=$(objpfx)libnss_%.so) $(objpfx)libnsl.so: $(libnsl-libc) \
					       $(common-objpfx)libc_nonshared.a


ifeq ($(build-shared),yes)
$(others:%=$(objpfx)%): $(objpfx)libnsl.so$(libnsl.so-version)
else
$(others:%=$(objpfx)%): $(objpfx)libnsl.a
endif
