/* Copyright (C) 1997, 2000, 2004, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* Generic version.  */

#ifndef _NETINET_IN_H
# error "Never use <bits/in.h> directly; include <netinet/in.h> instead."
#endif

/* Options for use with `getsockopt' and `setsockopt' at the IP level.
   The first word in the comment at the right is the data type used;
   "bool" means a boolean value stored in an `int'.  */
#define	IP_OPTIONS	1	/* ip_opts; IP per-packet options.  */
#define	IP_HDRINCL	2	/* int; Header is included with data.  */
#define	IP_TOS		3	/* int; IP type of service and precedence.  */
#define	IP_TTL		4	/* int; IP time to live.  */
#define	IP_RECVOPTS	5	/* bool; Receive all IP options w/datagram.  */
#define	IP_RECVRETOPTS	6	/* bool; Receive IP options for response.  */
#define	IP_RECVDSTADDR	7	/* bool; Receive IP dst addr w/datagram.  */
#define	IP_RETOPTS	8	/* ip_opts; Set/get IP per-packet options.  */
#define IP_MULTICAST_IF 9	/* in_addr; set/get IP multicast i/f */
#define IP_MULTICAST_TTL 10	/* u_char; set/get IP multicast ttl */
#define IP_MULTICAST_LOOP 11	/* i_char; set/get IP multicast loopback */
#define IP_ADD_MEMBERSHIP 12	/* ip_mreq; add an IP group membership */
#define IP_DROP_MEMBERSHIP 13	/* ip_mreq; drop an IP group membership */

/* Structure used to describe IP options for IP_OPTIONS and IP_RETOPTS.
   The `ip_dst' field is used for the first-hop gateway when using a
   source route (this gets put into the header proper).  */
struct ip_opts
  {
    struct in_addr ip_dst;	/* First hop; zero without source route.  */
    char ip_opts[40];		/* Actually variable in size.  */
  };

/* IPV6 socket options.  */
#define IPV6_ADDRFORM		1
#define IPV6_RXINFO		2
#define IPV6_HOPOPTS		3
#define IPV6_DSTOPTS		4
#define IPV6_RTHDR		5
#define IPV6_PKTOPTIONS		6
#define IPV6_CHECKSUM		7
#define IPV6_HOPLIMIT		8

#define IPV6_TXINFO		IPV6_RXINFO
#define SCM_SRCINFO		IPV6_TXINFO
#define SCM_SRCRT		IPV6_RXSRCRT

#define IPV6_UNICAST_HOPS	16
#define IPV6_MULTICAST_IF	17
#define IPV6_MULTICAST_HOPS	18
#define IPV6_MULTICAST_LOOP	19
#define IPV6_JOIN_GROUP		20
#define IPV6_LEAVE_GROUP	21
#define IPV6_ROUTER_ALERT      22
#define IPV6_MTU_DISCOVER      23
#define IPV6_MTU               24
#define IPV6_RECVERR           25
#define IPV6_V6ONLY            26
#define IPV6_JOIN_ANYCAST      27
#define IPV6_LEAVE_ANYCAST     28

/* Obsolete synonyms for the above.  */
#define IPV6_ADD_MEMBERSHIP	IPV6_JOIN_GROUP
#define IPV6_DROP_MEMBERSHIP	IPV6_LEAVE_GROUP
#define IPV6_RXHOPOPTS		IPV6_HOPOPTS
#define IPV6_RXDSTOPTS		IPV6_DSTOPTS

/* Routing header options for IPv6.  */
#define IPV6_RTHDR_LOOSE	0	/* Hop doesn't need to be neighbour. */
#define IPV6_RTHDR_STRICT	1	/* Hop must be a neighbour.  */

#define IPV6_RTHDR_TYPE_0	0	/* IPv6 Routing header type 0.  */
