# Copyright (C) 1994,95,96,97,98,99,2000,2001 Free Software Foundation, Inc.
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
#	Sub-makefile for resolv portion of the library.
#
subdir	:= resolv

headers	:= resolv.h \
	   netdb.h bits/netdb.h \
	   arpa/nameser.h arpa/nameser_compat.h \
	   sys/bitypes.h
distribute := ../conf/portability.h mapv4v6addr.h mapv4v6hostent.h \
	      Banner res_hconf.h res_debug.h README gai_misc.h ga_test.c

routines := herror inet_addr inet_ntop inet_pton nsap_addr res_init \
	    res_hconf res_libc gai_sigqueue

tests = tst-aton

include ../Makeconfig

extra-libs := libresolv libanl libnss_dns
extra-libs-others = $(extra-libs)
libresolv-routines := gethnamaddr res_comp res_debug	\
		      res_data res_mkquery res_query res_send		\
		      inet_net_ntop inet_net_pton inet_neta base64	\
		      ns_parse ns_name ns_netint ns_ttl ns_print	\
		      ns_samedomain

libanl-routines := gai_cancel gai_error gai_misc gai_notify gai_suspend \
		   getaddrinfo_a

subdir-dirs = nss_dns
vpath %.c nss_dns

libnss_dns-routines	:= dns-host dns-network
ifneq ($(build-static-nss),yes)
libnss_dns-inhibit-o	= $(filter-out .os,$(object-suffixes))
endif

ifeq (yes,$(build-shared))
tests: $(objpfx)ga_test
endif

include ../Rules

CPPFLAGS += -Dgethostbyname=res_gethostbyname \
	    -Dgethostbyname2=res_gethostbyname2 \
	    -Dgethostbyaddr=res_gethostbyaddr \
	    -Dgetnetbyname=res_getnetbyname \
	    -Dgetnetbyaddr=res_getnetbyaddr

# The BIND code elicits some harmless warnings.
+cflags += -Wno-strict-prototypes -Wno-write-strings

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libresolv.so: $(common-objpfx)libc.so

# The DNS NSS modules needs the resolver.
$(objpfx)libnss_dns.so: $(objpfx)libresolv.so $(common-objpfx)libc.so

# The asynchronous name lookup code needs the thread library.
$(objpfx)libanl.so: $(common-objpfx)libc.so $(shared-thread-library)

$(objpfx)ga_test: $(objpfx)libanl.so $(shared-thread-library)
