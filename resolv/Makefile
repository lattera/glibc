# Copyright (C) 1994-2001,2003,2004,2007,2008,2011,2012
#	Free Software Foundation, Inc.
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
#	Sub-makefile for resolv portion of the library.
#
subdir	:= resolv

headers	:= resolv.h \
	   netdb.h bits/netdb.h \
	   arpa/nameser.h arpa/nameser_compat.h \
	   sys/bitypes.h

routines := herror inet_addr inet_ntop inet_pton nsap_addr res_init \
	    res_hconf res_libc res-state

tests = tst-aton tst-leaks tst-inet_ntop
xtests = tst-leaks2

generate := mtrace-tst-leaks tst-leaks.mtrace tst-leaks2.mtrace

include ../Makeconfig

extra-libs := libresolv libnss_dns
ifeq ($(have-thread-library),yes)
extra-libs += libanl
routines += gai_sigqueue
endif
extra-libs-others = $(extra-libs)
libresolv-routines := gethnamaddr res_comp res_debug	\
		      res_data res_mkquery res_query res_send		\
		      inet_net_ntop inet_net_pton inet_neta base64	\
		      ns_parse ns_name ns_netint ns_ttl ns_print	\
		      ns_samedomain ns_date

libanl-routines := gai_cancel gai_error gai_misc gai_notify gai_suspend \
		   getaddrinfo_a

subdir-dirs = nss_dns
vpath %.c nss_dns

libnss_dns-routines	:= dns-host dns-network dns-canon
libnss_dns-inhibit-o	= $(filter-out .os,$(object-suffixes))
ifeq ($(build-static-nss),yes)
routines                += $(libnss_dns-routines) $(libresolv-routines)
static-only-routines    += $(libnss_dns-routines) $(libresolv-routines)
endif

ifeq (yesyes,$(build-shared)$(have-thread-library))
tests: $(objpfx)ga_test
endif

generated := mtrace-tst-leaks tst-leaks.mtrace \
	     mtrace-tst-leaks2 tst-leaks2.mtrace

include ../Rules

CPPFLAGS += -Dgethostbyname=res_gethostbyname \
	    -Dgethostbyname2=res_gethostbyname2 \
	    -Dgethostbyaddr=res_gethostbyaddr \
	    -Dgetnetbyname=res_getnetbyname \
	    -Dgetnetbyaddr=res_getnetbyaddr

ifeq (yes,$(have-ssp))
CFLAGS-libresolv += -fstack-protector
endif
CFLAGS-res_hconf.c = -fexceptions

# The BIND code elicits some harmless warnings.
+cflags += -Wno-strict-prototypes -Wno-write-strings

# Depend on libc.so so a DT_NEEDED is generated in the shared objects.
# This ensures they will load libc.so for needed symbols if loaded by
# a statically-linked program that hasn't already loaded it.
$(objpfx)libresolv.so: $(common-objpfx)libc.so $(common-objpfx)libc_nonshared.a
# Some hosts need '__stack_chk_guard', so pull in the definition from
# ld.so if required.
ifeq (yes,$(have-ssp))
LDLIBS-resolv.so += $(as-needed) $(elfobjdir)/ld.so $(no-as-needed)
endif

# The DNS NSS modules needs the resolver.
$(objpfx)libnss_dns.so: $(objpfx)libresolv.so $(common-objpfx)libc.so \
			$(common-objpfx)libc_nonshared.a

# The asynchronous name lookup code needs the thread library.
$(objpfx)libanl.so: $(common-objpfx)libc.so $(common-objpfx)libc_nonshared.a \
		    $(shared-thread-library)

$(objpfx)ga_test: $(objpfx)libanl.so $(shared-thread-library)

$(objpfx)tst-leaks: $(objpfx)libresolv.so
tst-leaks-ENV = MALLOC_TRACE=$(objpfx)tst-leaks.mtrace
$(objpfx)mtrace-tst-leaks: $(objpfx)tst-leaks.out
	$(common-objpfx)malloc/mtrace $(objpfx)tst-leaks.mtrace > $@
ifeq ($(run-built-tests),yes)
ifneq (no,$(PERL))
tests: $(objpfx)mtrace-tst-leaks
endif
endif

tst-leaks2-ENV = MALLOC_TRACE=$(objpfx)tst-leaks2.mtrace
$(objpfx)mtrace-tst-leaks2: $(objpfx)tst-leaks2.out
	$(common-objpfx)malloc/mtrace $(objpfx)tst-leaks2.mtrace > $@
ifeq ($(run-built-tests),yes)
ifneq (no,$(PERL))
xtests: $(objpfx)mtrace-tst-leaks2
endif
endif
