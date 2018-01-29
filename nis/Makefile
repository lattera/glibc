# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

include ../Makeconfig

ifeq ($(build-obsolete-nsl),yes)
headers			:= $(wildcard rpcsvc/*.[hx])

# These are the databases available for the nis (and perhaps later nisplus)
# service.  This must be a superset of the services in nss.
databases		= proto service hosts network grp pwd rpc ethers \
			  spwd netgrp alias publickey

# Specify rules for the nss_* modules.
services		:= nis nisplus
endif

extra-libs		= libnsl
ifeq ($(build-obsolete-nsl),yes)
extra-libs += $(services:%=libnss_%)
endif
# These libraries will be built in the `others' pass rather than
# the `lib' pass, because they depend on libc.so being built already.
extra-libs-others	= $(extra-libs)

ifeq ($(build-obsolete-nsl),yes)
# The sources are found in the appropriate subdir.
subdir-dirs = $(services:%=nss_%)
vpath %.c $(subdir-dirs)
endif

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
		  nis_clone_res

ifeq ($(build-obsolete-nsl),yes)
libnsl-routines += nss-default
libnss_nis-routines	:= $(addprefix nis-,$(databases)) nis-initgroups \
			   nss-nis
libnss_nis-inhibit-o	= $(filter-out .os,$(object-suffixes))

libnss_nisplus-routines	:= $(addprefix nisplus-,$(databases)) nisplus-parser \
			   nss-nisplus nisplus-initgroups
libnss_nisplus-inhibit-o = $(filter-out .os,$(object-suffixes))

else # not $(build-obsolete-nsl)
# Pretend that libnsl.so is a linker script, so that the symbolic link
# is not installed.
install-lib-ldscripts = libnsl.so
$(inst_libdir)/libnsl.so:
# Build only shared libnsl.
libnsl-inhibit-o = $(filter-out .os,$(object-suffixes))

endif # not $(build-obsolete-nsl)

include ../Rules


ifeq ($(build-obsolete-nsl),yes)
$(objpfx)libnss_nis.so: $(objpfx)libnsl.so$(libnsl.so-version) \
			$(common-objpfx)nss/libnss_files.so
$(objpfx)libnss_nisplus.so: $(objpfx)libnsl.so$(libnsl.so-version)
endif

libnsl-libc = $(common-objpfx)linkobj/libc.so
# Target-specific variable setting to link objects using deprecated
# RPC interfaces with the version of libc.so that makes them available
# for new links:
$(services:%=$(objpfx)libnss_%.so) $(objpfx)libnsl.so: \
  libc-for-link = $(libnsl-libc)


ifeq ($(build-shared),yes)
$(others:%=$(objpfx)%): $(objpfx)libnsl.so$(libnsl.so-version)
else
ifeq ($(build-obsolete-nsl),yes)
$(others:%=$(objpfx)%): $(objpfx)libnsl.a
endif
endif
