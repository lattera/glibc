# Copyright (C) 1996-2015 Free Software Foundation, Inc.
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
#	Makefile for name service switch.
#
subdir	:= nss

include ../Makeconfig

headers			:= nss.h

# This is the trivial part which goes into libc itself.
routines		= nsswitch getnssent getnssent_r digits_dots \
			  $(addsuffix -lookup,$(databases))

# These are the databases that go through nss dispatch.
# Caution: if you add a database here, you must add its real name
# in databases.def, too.
databases		= proto service hosts network grp pwd rpc ethers \
			  spwd netgrp key alias sgrp

others                  := getent makedb
install-bin             := getent makedb
makedb-modules = xmalloc hash-string
extra-objs		+= $(makedb-modules:=.o)

tests			= test-netdb tst-nss-test1 test-digits-dots
xtests			= bug-erange

# Specify rules for the nss_* modules.  We have some services.
services		:= files db

extra-libs		= $(services:%=libnss_%)
# These libraries will be built in the `others' pass rather than
# the `lib' pass, because they depend on libc.so being built already.
extra-libs-others	= $(extra-libs)

# The sources are found in the appropriate subdir.
subdir-dirs = $(services:%=nss_%)
vpath %.c $(subdir-dirs) ../locale/programs ../intl


libnss_files-routines	:= $(addprefix files-,$(databases)) \
			   files-initgroups files-have_o_cloexec files-init

libnss_db-dbs		:= $(addprefix db-,\
				       $(filter-out hosts network key alias,\
						    $(databases))) \
			   db-initgroups
libnss_db-routines	:= $(libnss_db-dbs) db-open db-init hash-string
generated		+= $(filter-out db-alias.c db-netgrp.c, \
					$(addsuffix .c,$(libnss_db-dbs)))

install-others		+= $(inst_vardbdir)/Makefile

# Build static module into libc if requested
libnss_files-inhibit-o	= $(filter-out .os,$(object-suffixes))
libnss_db-inhibit-o	= $(filter-out .os,$(object-suffixes))
ifeq ($(build-static-nss),yes)
routines                += $(libnss_files-routines)
static-only-routines    += $(libnss_files-routines)
tests-static		= tst-nss-static
tests			+= $(tests-static)
endif

include ../Rules

ifeq (yes,$(have-selinux))
LDLIBS-makedb		:= -lselinux
endif

libnss-libc = $(common-objpfx)linkobj/libc.so
# Target-specific variable setting to link objects using deprecated
# RPC interfaces with the version of libc.so that makes them available
# for new links:
$(services:%=$(objpfx)libnss_%.so): libc-for-link = $(libnss-libc)

$(objpfx)libnss_db.so: $(objpfx)libnss_files.so

$(libnss_db-dbs:%=$(objpfx)%.c): $(objpfx)db-%.c: nss_files/files-%.c
	@rm -f $@.new
	(echo '#define EXTERN_PARSER';\
	 echo '#define GENERIC "../nss_db/db-XXX.c"';\
	 echo '#include "$<"') > $@.new
	mv -f $@.new $@


$(objpfx)makedb: $(makedb-modules:%=$(objpfx)%.o)

$(inst_vardbdir)/Makefile: db-Makefile $(+force)
	$(do-install)

libof-nss_test1 = extramodules
$(objpfx)/libnss_test1.so: $(objpfx)nss_test1.os $(link-libc-deps)
	$(build-module)
ifdef libnss_test1.so-version
$(objpfx)/libnss_test1.so$(libnss_test1.so-version): $(objpfx)/libnss_test1.so
	$(make-link)
endif
$(objpfx)tst-nss-test1.out: $(objpfx)/libnss_test1.so$(libnss_test1.so-version)
