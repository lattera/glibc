/* Functions for storing Ethernet addresses in ASCII and mapping to hostnames.
Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef __NETINET_ETHER_H

#define __NETINET_ETHER_H	1
#include <features.h>

/* Get definition of `struct ether_addr'.  */
#include <netinet/if_ether.h>


/* Convert 48 bit Ethernet ADDRess to ASCII.  */
char *ether_ntoa __P ((const struct ether_addr *__addr));
char *ether_ntoa_r __P ((const struct ether_addr *__addr, char *__buf));

/* Convert ASCII string S to 48 bit Ethernet address.  */
struct ether_addr *ether_aton __P ((const char *__asc));
struct ether_addr *ether_aton_r __P ((const char *__asc,
				      struct ether_addr *__addr));

/* Map 48 bit Ethernet number ADDR to HOSTNAME.  */
int ether_ntohost __P ((char *__hostname, const struct ether_addr *__addr));

/* Map HOSTNAME to 48 bit Ethernet address.  */
int ether_hostton __P ((const char *__hostname, struct ether_addr *__addr));

/* Scan LINE and set ADDR and HOSTNAME.  */
int ether_line __P ((const char *__line, struct ether_addr *__addr,
		     char *__hostname));

#endif /* netinet/ether.h */
