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

routines := nscd_getpw_r nscd_getgr_r

# We can later add the names of other thread packages here.
ifeq (,$(findstring linuxthreads,$(add-ons)))

others := nscd
install-sbin := nscd

endif

nscd-routines := nscd connections pwdcache getpwnam_r getpwuid_r grpcache \
		 getgrnam_r getgrgid_r dbg_log nscd_conf nscd_stat
extra-objs := $(nscd-routines:=.o)

distribute := nscd.h dbg_log.h $(nscd-routines:=.c) TODO nscd.conf nscd.init \
	      nscd_proto.h

include ../Rules

ifeq ($(build-shared),yes)
$(objpfx)nscd: $(nscd-routines:%=$(objpfx)%.o) $(objpfx)../linuxthreads/libpthread.so$(libpthread.so-version) $(objpfx)../nis/libnsl.so$(libnsl.so-version)
else
$(objpfx)nscd: $(nscd-routines:%=$(objpfx)%.o) $(objpfx)../linuxthreads/libpthread.a $(objpfx)../nis/libnsl.a
endif
