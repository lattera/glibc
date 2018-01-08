# Copyright (C) 1994-2018 Free Software Foundation, Inc.
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

include ../Makeconfig

headers	:= resolv.h bits/types/res_state.h \
	   netdb.h bits/netdb.h \
	   arpa/nameser.h arpa/nameser_compat.h \
	   sys/bitypes.h

routines := herror inet_addr inet_ntop inet_pton nsap_addr res_init \
	    res_hconf res_libc res-state res_randomid res-close \
	    resolv_context resolv_conf

tests = tst-aton tst-leaks tst-inet_ntop
xtests = tst-leaks2

generate := mtrace-tst-leaks.out tst-leaks.mtrace tst-leaks2.mtrace

extra-libs := libresolv libnss_dns
ifeq ($(have-thread-library),yes)
extra-libs += libanl
routines += gai_sigqueue

tests += \
  tst-bug18665 \
  tst-bug18665-tcp \
  tst-ns_name \
  tst-ns_name_compress \
  tst-ns_name_pton \
  tst-res_hconf_reorder \
  tst-res_hnok \
  tst-res_use_inet6 \
  tst-resolv-basic \
  tst-resolv-binary \
  tst-resolv-edns \
  tst-resolv-network \
  tst-resolv-res_init-multi \
  tst-resolv-search \

# These tests need libdl.
ifeq (yes,$(build-shared))
tests += \
  tst-resolv-canonname \

# uses DEPRECATED_RES_USE_INET6 from <resolv-internal.h>.
tests-internal += \
  tst-resolv-res_init \
  tst-resolv-res_init-thread \

# Needs resolv_context.
tests-internal += \
  tst-resolv-res_ninit \
  tst-resolv-threads \

endif

# This test accesses __inet_ntop_length, an internal libc function.
tests-internal += tst-inet_pton

# This test accesses the __p_secstodate compat symbol.
tests-internal += tst-p_secstodate

# This test sends millions of packets and is rather slow.
xtests += tst-resolv-qtypes

# This test has dropped packet tests and runs for a long time.
xtests += tst-resolv-rotate
endif
extra-libs-others = $(extra-libs)
libresolv-routines := res_comp res_debug \
		      res_data res_mkquery res_query res_send		\
		      inet_net_ntop inet_net_pton inet_neta base64	\
		      ns_parse ns_name ns_netint ns_ttl ns_print	\
		      ns_samedomain ns_date \
		      compat-hooks compat-gethnamaddr

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

ifeq ($(run-built-tests),yes)
ifneq (no,$(PERL))
tests-special += $(objpfx)mtrace-tst-leaks.out
xtests-special += $(objpfx)mtrace-tst-leaks2.out
tests-special += $(objpfx)mtrace-tst-resolv-res_ninit.out
endif
endif

ifeq (,$(filter sunrpc,$(subdirs)))
# The netdb.h we install does '#include <rpc/netdb.h>', so one must exist.
# If sunrpc/ is built in this configuration, it installs a real <rpc/netdb.h>.
# If that's not going to happen, install our dummy file.
headers += rpc/netdb.h
endif

generated += mtrace-tst-leaks.out tst-leaks.mtrace \
	     mtrace-tst-leaks2.out tst-leaks2.mtrace \
	     mtrace-tst-resolv-res_ninit.out tst-resolv-res_ninit.mtrace \

include ../Rules

CFLAGS-res_hconf.c += -fexceptions

# The DNS NSS modules needs the resolver.
$(objpfx)libnss_dns.so: $(objpfx)libresolv.so

# The asynchronous name lookup code needs the thread library.
$(objpfx)libanl.so: $(shared-thread-library)

$(objpfx)tst-res_hconf_reorder: $(libdl) $(shared-thread-library)
tst-res_hconf_reorder-ENV = RESOLV_REORDER=on

$(objpfx)tst-leaks: $(objpfx)libresolv.so
tst-leaks-ENV = MALLOC_TRACE=$(objpfx)tst-leaks.mtrace
$(objpfx)mtrace-tst-leaks.out: $(objpfx)tst-leaks.out
	$(common-objpfx)malloc/mtrace $(objpfx)tst-leaks.mtrace > $@; \
	$(evaluate-test)

tst-leaks2-ENV = MALLOC_TRACE=$(objpfx)tst-leaks2.mtrace
$(objpfx)mtrace-tst-leaks2.out: $(objpfx)tst-leaks2.out
	$(common-objpfx)malloc/mtrace $(objpfx)tst-leaks2.mtrace > $@; \
	$(evaluate-test)

tst-resolv-res_ninit-ENV = MALLOC_TRACE=$(objpfx)tst-resolv-res_ninit.mtrace
$(objpfx)mtrace-tst-resolv-res_ninit.out: $(objpfx)tst-resolv-res_ninit.out
	$(common-objpfx)malloc/mtrace \
	  $(objpfx)tst-resolv-res_ninit.mtrace > $@; \
	  $(evaluate-test)

$(objpfx)tst-bug18665-tcp: $(objpfx)libresolv.so $(shared-thread-library)
$(objpfx)tst-bug18665: $(objpfx)libresolv.so $(shared-thread-library)
$(objpfx)tst-res_use_inet6: $(objpfx)libresolv.so $(shared-thread-library)
$(objpfx)tst-resolv-basic: $(objpfx)libresolv.so $(shared-thread-library)
$(objpfx)tst-resolv-binary: $(objpfx)libresolv.so $(shared-thread-library)
$(objpfx)tst-resolv-edns: $(objpfx)libresolv.so $(shared-thread-library)
$(objpfx)tst-resolv-network: $(objpfx)libresolv.so $(shared-thread-library)
$(objpfx)tst-resolv-res_init: $(libdl) $(objpfx)libresolv.so
$(objpfx)tst-resolv-res_init-multi: $(objpfx)libresolv.so \
  $(shared-thread-library)
$(objpfx)tst-resolv-res_init-thread: $(libdl) $(objpfx)libresolv.so \
  $(shared-thread-library)
$(objpfx)tst-resolv-qtypes: $(objpfx)libresolv.so $(shared-thread-library)
$(objpfx)tst-resolv-rotate: $(objpfx)libresolv.so $(shared-thread-library)
$(objpfx)tst-resolv-search: $(objpfx)libresolv.so $(shared-thread-library)
$(objpfx)tst-resolv-threads: \
  $(libdl) $(objpfx)libresolv.so $(shared-thread-library)
$(objpfx)tst-resolv-canonname: \
  $(libdl) $(objpfx)libresolv.so $(shared-thread-library)

$(objpfx)tst-ns_name: $(objpfx)libresolv.so
$(objpfx)tst-ns_name.out: tst-ns_name.data
$(objpfx)tst-ns_name_compress: $(objpfx)libresolv.so
$(objpfx)tst-ns_name_pton: $(objpfx)libresolv.so
$(objpfx)tst-res_hnok: $(objpfx)libresolv.so
$(objpfx)tst-p_secstodate: $(objpfx)libresolv.so


# This test case uses the deprecated RES_USE_INET6 resolver option.
CFLAGS-tst-res_use_inet6.c += -Wno-error
