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
databases		= proto service hosts network grp pwd rpc ethers \
			  spwd

# Specify rules for the nss_* modules.  We have some services.
services		:= files dns db

extra-libs		= $(services:%=libnss_%)
# These libraries will be built in the `others' pass rather than
# the `lib' pass, because they depend on libc.so being built already.
extra-libs-others	= $(extra-libs)

# The sources are found in the appropriate subdir.
subdir-dirs = $(services:%=nss_%)
vpath %.c $(subdir-dirs)


libnss_files-routines	:= $(addprefix files-,$(databases))
distribute		+= files-XXX.c files-parse.c

libnss_dns-routines	:= dns-host dns-network

libnss_db-routines	:= $(addprefix db-,$(filter-out hosts network,\
							$(databases)))
generated		+= $(addsuffix .c,$(libnss_db-routines))
distribute		+= db-XXX.c

libnss_files-inhibit-o	= $(filter-out .so,$(object-suffixes))
libnss_dns-inhibit-o	= $(filter-out .so,$(object-suffixes))
libnss_db-inhibit-o	= $(filter-out .so,$(object-suffixes))


include ../Rules


$(objpfx)libnss_dns.so: $(firstword $(objdir) $(..)resolv)/libresolv.so

$(objpfx)libnss_db.so: $(firstword $(objdir) $(..)db)/libdb.so \
		       $(objpfx)libnss_files.so

$(libnss_db-routines:%=$(objpfx)%.c): $(objpfx)db-%.c: nss_files/files-%.c
	@rm -f $@.new
	(echo '#define EXTERN_PARSER';\
	 echo '#define GENERIC "../nss_db/db-XXX.c"';\
	 echo '#include <$<>') > $@.new
	mv -f $@.new $@

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(services:%=$(objpfx)libnss_%.so): $(common-objpfx)libc.so
