# Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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
#	Makefile for name service switch.
#
subdir	:= nss

headers			:= nss.h
distribute		:= nsswitch.h XXX-lookup.c getXXbyYY.c getXXbyYY_r.c \
			   getXXent.c getXXent_r.c databases.def \
			   nsswitch.conf db-Makefile digits_dots.c \
			   function.def

# This is the trivial part which goes into libc itself.
routines		= nsswitch $(addsuffix -lookup,$(databases))

# These are the databases that go through nss dispatch.
# Caution: if you add a database here, you must add its real name
# in databases.def, too.
databases		= proto service hosts network grp pwd rpc ethers \
			  spwd netgrp key alias

tests			= test-netdb

include ../Makeconfig

# Specify rules for the nss_* modules.  We have some services.
services		:= files db $(LDAP)

extra-libs		= $(services:%=libnss_%)
# These libraries will be built in the `others' pass rather than
# the `lib' pass, because they depend on libc.so being built already.
extra-libs-others	= $(extra-libs)

# The sources are found in the appropriate subdir.
subdir-dirs = $(services:%=nss_%)
vpath %.c $(subdir-dirs)


libnss_files-routines	:= $(addprefix files-,$(filter-out key, $(databases)))
distribute		+= files-XXX.c files-parse.c

libnss_db-routines	:= $(addprefix db-,$(filter-out hosts network key,\
							$(databases)))
generated		+= $(filter-out db-alias.c db-netgrp.c, \
					$(addsuffix .c,$(libnss_db-routines)))
distribute		+= db-XXX.c

libnss_ldap-routines	:= $(addprefix ldap-,proto) util ldap-nss


# Build static module if requested
ifneq ($(build-static-nss),yes)
libnss_files-inhibit-o	= $(filter-out .os,$(object-suffixes))
endif
libnss_db-inhibit-o	= $(filter-out .os,$(object-suffixes))
libnss_ldap-inhibit-o	= $(filter-out .os,$(object-suffixes))

# If we compile the LDAP module we need the headers from the LDAP library.
ifneq ($(LDAP),)
CPPFLAGS += -I../ldap
endif

include ../Rules


$(objpfx)libnss_db.so: $(dbobjdir)/libdb.so $(objpfx)libnss_files.so

$(libnss_db-routines:%=$(objpfx)%.c): $(objpfx)db-%.c: nss_files/files-%.c
	@rm -f $@.new
	(echo '#define EXTERN_PARSER';\
	 echo '#define GENERIC "../nss_db/db-XXX.c"';\
	 echo '#include <$<>') > $@.new
	mv -f $@.new $@

# To complete the LDAP NSS module we need functions from the LDAP library.
$(objpfx)libnss_ldap.so: $(common-objpfx)ldap/libldap.so

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(services:%=$(objpfx)libnss_%.so): $(common-objpfx)libc.so
