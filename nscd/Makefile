# Copyright (C) 1998,2000,2002-2009,2011 Free Software Foundation, Inc.
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
#	Sub-makefile for nscd portion of the library.
#
subdir	:= nscd

routines := nscd_getpw_r nscd_getgr_r nscd_gethst_r nscd_getai \
	    nscd_initgroups nscd_getserv_r nscd_netgroup
aux	:= nscd_helper

include ../Makeconfig

# To find xmalloc.c
vpath %.c ../locale/programs

nscd-modules := nscd connections pwdcache getpwnam_r getpwuid_r grpcache \
		getgrnam_r getgrgid_r hstcache gethstbyad_r gethstbynm3_r \
		getsrvbynm_r getsrvbypt_r servicescache \
		dbg_log nscd_conf nscd_stat cache mem nscd_setup_thread \
		xmalloc xstrdup aicache initgrcache gai res_hconf \
		netgroupcache

ifeq ($(have-thread-library),yes)

others += nscd
others-pie += nscd
install-sbin := nscd

extra-objs := $(nscd-modules:=.o)

endif

ifeq (yes,$(build-static-nss))
otherlibs += $(nssobjdir)/libnss_files.a $(resolvobjdir)/libnss_dns.a \
	     $(resolvobjdir)/libresolv.a
endif

all-nscd-modules := $(nscd-modules) selinux
ifeq (yes,$(have-selinux))
ifeq (yes,$(have-libaudit))
libaudit = -laudit
ifeq (yes,$(have-libcap))
libcap = -lcap
endif
endif

nscd-modules += selinux
selinux-LIBS := -lselinux $(libaudit) $(libcap)

# The configure.in check for libselinux and its headers did not use
# $SYSINCLUDES.  The directory specified by --with-headers usually
# contains only the basic kernel interface headers, not something like
# libselinux.  So the simplest thing is to presume that the standard
# system headers will be ok for this file.
$(objpfx)nscd_stat.o: sysincludes = # nothing
$(objpfx)selinux.o: sysincludes = # nothing
endif

LDLIBS-nscd = $(selinux-LIBS)

distribute := nscd.h nscd-client.h dbg_log.h \
	      $(addsuffix .c, $(filter-out xmalloc,$(all-nscd-modules))) \
	      nscd.conf nscd.init nscd_proto.h nscd-types.h

include ../Rules

CFLAGS-nscd_getpw_r.c = -fexceptions
CFLAGS-nscd_getgr_r.c = -fexceptions
CFLAGS-nscd_gethst_r.c = -fexceptions
CFLAGS-nscd_getai.c = -fexceptions
CFLAGS-nscd_initgroups.c = -fexceptions

nscd-cflags = -DIS_IN_nscd=1 -D_FORTIFY_SOURCE=2
ifeq (yesyes,$(have-fpie)$(build-shared))
nscd-cflags += $(pie-ccflag)
endif
ifeq (yes,$(have-ssp))
nscd-cflags += -fstack-protector
endif

CFLAGS-nscd.c += $(nscd-cflags)
CFLAGS-connections.c += $(nscd-cflags)
CFLAGS-pwdcache.c += $(nscd-cflags)
CFLAGS-getpwnam_r.c += $(nscd-cflags)
CFLAGS-getpwuid_r.c += $(nscd-cflags)
CFLAGS-grpcache.c += $(nscd-cflags)
CFLAGS-getgrnam_r.c += $(nscd-cflags)
CFLAGS-getgrgid_r.c += $(nscd-cflags)
CFLAGS-hstcache.c += $(nscd-cflags)
CFLAGS-gethstbyad_r.c += $(nscd-cflags)
CFLAGS-gethstbynm3_r.c += $(nscd-cflags)
CFLAGS-dbg_log.c += $(nscd-cflags)
CFLAGS-nscd_conf.c += $(nscd-cflags)
CFLAGS-nscd_stat.c += $(nscd-cflags)
CFLAGS-cache.c += $(nscd-cflags)
CFLAGS-xmalloc.c += $(nscd-cflags)
CFLAGS-xstrdup.c += $(nscd-cflags)
CFLAGS-mem.c += $(nscd-cflags)
CFLAGS-nscd_setup_thread.c += $(nscd-cflags)
CFLAGS-aicache.c += $(nscd-cflags)
CFLAGS-selinux.c += $(nscd-cflags)
CFLAGS-initgrcache.c += $(nscd-cflags)
CFLAGS-gai.c += $(nscd-cflags)
CFLAGS-servicescache.c += $(nscd-cflags)
CFLAGS-getsrvbynm_r.c += $(nscd-cflags)
CFLAGS-getsrvbypt_r.c += $(nscd-cflags)
CFLAGS-res_hconf.c += $(nscd-cflags)
CFLAGS-netgroupcache.c += $(nscd-cflags)

ifeq (yesyes,$(have-fpie)$(build-shared))
LDFLAGS-nscd = -Wl,-z,now
endif

# This makes sure -DNOT_IN_libc is passed for all these modules.
cpp-srcs-left := $(nscd-modules:=.c)
lib := nonlib
include $(patsubst %,$(..)cppflags-iterator.mk,$(cpp-srcs-left))

$(objpfx)nscd: $(nscd-modules:%=$(objpfx)%.o)

ifeq ($(build-shared),yes)
$(objpfx)nscd: $(common-objpfx)rt/librt.so $(shared-thread-library) \
	       $(common-objpfx)nis/libnsl.so
else
$(objpfx)nscd: $(common-objpfx)rt/librt.a $(static-thread-library) \
	       $(common-objpfx)nis/libnsl.a
endif
