# Copyright (C) 1998 Free Software Foundation, Inc.
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
#	Sub-makefile for nscd portion of the library.
#
subdir	:= nscd

routines := nscd_getpw_r nscd_getgr_r nscd_gethst_r

include ../Makeconfig

nscd-modules := nscd connections pwdcache getpwnam_r getpwuid_r grpcache \
		getgrnam_r getgrgid_r hstcache gethstbyad_r gethstbynm2_r \
		dbg_log nscd_conf nscd_stat cache

ifeq ($(have-thread-library),yes)

others := nscd
install-sbin := nscd

extra-objs := $(nscd-modules:=.o)

endif

distribute := nscd.h nscd-client.h dbg_log.h \
	      $(nscd-modules:=.c) TODO nscd.conf nscd.init \
	      nscd_proto.h

include ../Rules

$(objpfx)nscd: $(nscd-modules:%=$(objpfx)%.o)

ifeq ($(build-shared),yes)
$(objpfx)nscd: $(shared-thread-library) $(common-objpfx)nis/libnsl.so
else
$(objpfx)nscd: $(static-thread-library) $(common-objpfx)nis/libnsl.a
endif
