/* Copyright (C) 1991,92,93,94,95,96,97,98 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* Linux version.  */

#ifndef _NETINET_IN_H
# error "Never use <bits/in.h> directly; include <netinet/in.h> instead."
#endif

/* Options for use with `getsockopt' and `setsockopt' at the IP level.
   The first word in the comment at the right is the data type used;
   "bool" means a boolean value stored in an `int'.  */
#define	IP_TOS		   1	/* int; IP type of service and precedence.  */
#define	IP_TTL		   2	/* int; IP time to live.  */
#define	IP_HDRINCL	   3	/* int; Header is included with data.  */
#define	IP_OPTIONS	   4	/* ip_opts; IP per-packet options.  */
#define IP_MULTICAST_IF    32	/* in_addr; set/get IP multicast i/f */
#define IP_MULTICAST_TTL   33	/* u_char; set/get IP multicast ttl */
#define IP_MULTICAST_LOOP  34	/* i_char; set/get IP multicast loopback */
#define IP_ADD_MEMBERSHIP  35	/* ip_mreq; add an IP group membership */
#define IP_DROP_MEMBERSHIP 36	/* ip_mreq; drop an IP group membership */

/* To select the IP level.  */
#define SOL_IP	0

/* Structure used to describe IP options for IP_OPTIONS. The `ip_dst'
   field is used for the first-hop gateway when using a source route
   (this gets put into the header proper).  */
struct ip_opts
  {
    struct in_addr ip_dst;	/* First hop; zero without source route.  */
    char ip_opts[40];		/* Actually variable in size.  */
  };

/* Structure used for IP_ADD_MEMBERSHIP and IP_DROP_MEMBERSHIP. */
struct ip_mreq
  {
    struct in_addr imr_multiaddr;	/* IP multicast address of group */
    struct in_addr imr_interface;	/* local IP address of interface */
  };

/* IPV6 socket options.  */
#define IPV6_ADDRFORM		1
#define IPV6_PKTINFO		2
#define IPV6_RXHOPOPTS		3 /* obsolete name */
#define IPV6_RXDSTOPTS		4 /* obsolete name */
#define IPV6_HOPOPTS		IPV6_RXHOPOPTS  /* new name */
#define IPV6_DSTOPTS		IPV6_RXDSTOPTS  /* new name */
#define IPV6_RXSRCRT		5
#define IPV6_PKTOPTIONS		6
#define IPV6_CHECKSUM		7
#define IPV6_HOPLIMIT		8

#define SCM_SRCRT		IPV6_RXSRCRT

#define IPV6_UNICAST_HOPS	16
#define IPV6_MULTICAST_IF	17
#define IPV6_MULTICAST_HOPS	18
#define IPV6_MULTICAST_LOOP	19
#define IPV6_ADD_MEMBERSHIP	20
#define IPV6_DROP_MEMBERSHIP	21
#define IPV6_ROUTER_ALERT	22
