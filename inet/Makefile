# Copyright (C) 1991-1999, 2000, 2001 Free Software Foundation, Inc.
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
#	Sub-makefile for inet portion of the library.
#
subdir	:= inet

headers	:= netinet/ether.h netinet/in.h netinet/in_systm.h \
	   netinet/if_ether.h \
	   netinet/tcp.h netinet/ip.h $(wildcard arpa/*.h protocols/*.h) \
	   aliases.h netinet/ip6.h netinet/icmp6.h bits/in.h

distribute := netgroup.h

routines := htonl htons		\
	    inet_lnaof inet_mkadr	\
	    inet_netof inet_ntoa inet_net herrno \
	    gethstbyad gethstbyad_r gethstbynm gethstbynm2 gethstbynm2_r \
	    gethstbynm_r gethstent gethstent_r \
	    getnetbyad getnetbyad_r getnetbynm getnetent getnetent_r \
	    getnetbynm_r \
	    getproto getproto_r getprtent getprtent_r getprtname getprtname_r \
	    getsrvbynm getsrvbynm_r getsrvbypt getsrvbypt_r getservent \
	    getservent_r	\
	    getrpcent getrpcbyname getrpcbynumber \
	    getrpcent_r getrpcbyname_r getrpcbynumber_r \
	    ether_aton ether_aton_r ether_hton ether_line \
	    ether_ntoa ether_ntoa_r ether_ntoh \
	    rcmd rexec ruserpass \
	    getnetgrent_r getnetgrent \
	    getaliasent_r getaliasent getaliasname getaliasname_r \
	    in6_addr getnameinfo if_index

tests := htontest test_ifindex tst-ntoa tst-ether_aton tst-network \
	 tst-gethnm

include ../Rules

ifeq ($(have-thread-library),yes)

CFLAGS-gethstbyad_r.c = -DUSE_NSCD=1
CFLAGS-gethstbynm_r.c = -DUSE_NSCD=1
CFLAGS-gethstbynm2_r.c = -DUSE_NSCD=1

endif

ifeq ($(build-static-nss),yes)
CFLAGS += -DSTATIC_NSS
endif

ifeq (yes,$(build-static-nss))
otherlibs += $(nssobjdir)/libnss_files.a $(resolvobjdir)/libnss_dns.a \
             $(resolvobjdir)/libresolv.a
endif
