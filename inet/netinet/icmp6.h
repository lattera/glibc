/* Copyright (C) 1991, 92, 93, 94, 95, 96, 97 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _NETINET_ICMP6_H
#define _NETINET_ICMP6_H 1

#include <sys/types.h>
#include <netinet/in.h>

#define ICMPV6_FILTER 1

#define ICMPV6_FILTER_BLOCK		1
#define ICMPV6_FILTER_PASS		2
#define ICMPV6_FILTER_BLOCKOTHERS	3
#define ICMPV6_FILTER_PASSONLY		4

struct icmpv6_filter
  {
    u_int32_t data[8];
  };

struct icmpv6hdr
  {
    u_int8_t icmpv6_type;   /* type field */
    u_int8_t icmpv6_code;   /* code field */
    u_int16_t icmpv6_cksum;  /* checksum field */
    union
      {
	u_int32_t un_data32[1]; /* type-specific field */
	u_int16_t un_data16[2]; /* type-specific field */
	u_int8_t un_data8[4];  /* type-specific field */
      } icmpv6_dataun;
  };

#define icmpv6_data32    icmpv6_dataun.un_data32
#define icmpv6_data16    icmpv6_dataun.un_data16
#define icmpv6_data8     icmpv6_dataun.un_data8
#define icmpv6_pptr      icmpv6_data32[0]  /* parameter prob */
#define icmpv6_mtu       icmpv6_data32[0]  /* packet too big */
#define icmpv6_id        icmpv6_data16[0]  /* echo request/reply */
#define icmpv6_seq       icmpv6_data16[1]  /* echo request/reply */
#define icmpv6_maxdelay  icmpv6_data16[0]  /* mcast group membership */

#define ICMPV6_DEST_UNREACH             1
#define ICMPV6_PACKET_TOOBIG            2
#define ICMPV6_TIME_EXCEEDED            3
#define ICMPV6_PARAMETER_PROBLEM        4
#define ICMPV6_INFOMSG_MASK             128 /* message is info if bit set */
#define ICMPV6_ECHOREQUEST              128
#define ICMPV6_ECHOREPLY                129
#define ICMPV6_MGM_QUERY                130
#define ICMPV6_MGM_REPORT               131
#define ICMPV6_MGM_REDUCTION            132

#define ICMPV6_DEST_UNREACH_NOROUTE	  0
#define ICMPV6_DEST_UNREACH_ADMIN	  1 /* administratively prohibited */
#define ICMPV6_DEST_UNREACH_NOTNEIGHBOR   2 /* not a neighbor (and must be) */
#define ICMPV6_DEST_UNREACH_ADDR          3
#define ICMPV6_DEST_UNREACH_NOPORT        4
#define ICMPV6_TIME_EXCEED_HOPS           0 /* Hop Limit == 0 in transit */
#define ICMPV6_TIME_EXCEED_REASSEMBLY     1 /* Reassembly time out */
#define ICMPV6_PARAMPROB_HEADER           0 /* erroneous header field */
#define ICMPV6_PARAMPROB_NEXTHEADER       1 /* unrecognized Next Header */
#define ICMPV6_PARAMPROB_OPTION           2 /* unrecognized option */

#define ICMPV6_FILTER_WILLPASS(type, filterp) \
	((((filterp)->data[(type) >> 5]) & (1 << ((type) & 31))) == 0)

#define ICMPV6_FILTER_WILLBLOCK(type, filterp) \
	((((filterp)->data[(type) >> 5]) & (1 << ((type) & 31))) != 0)

#define ICMPV6_FILTER_SETPASS(type, filterp) \
	((((filterp)->data[(type) >> 5]) &= ~(1 << ((type) & 31))))

#define ICMPV6_FILTER_SETBLOCK(type, filterp) \
	((((filterp)->data[(type) >> 5]) |=  (1 << ((type) & 31))))

#define ICMPV6_FILTER_SETPASSALL(filterp) \
	memset (filterp, 0, sizeof (struct icmpv6_filter));

#define ICMPV6_FILTER_SETBLOCKALL(filterp) \
	memset (filterp, 0xFF, sizeof (struct icmpv6_filter));

#define ND6_ROUTER_SOLICITATION		133
#define ND6_ROUTER_ADVERTISEMENT	134
#define ND6_NEIGHBOR_SOLICITATION	135
#define ND6_NEIGHBOR_ADVERTISEMENT	136
#define ND6_REDIRECT			137

enum nd6_option
  {
    ND6_OPT_SOURCE_LINKADDR=1,
    ND6_OPT_TARGET_LINKADDR=2,
    ND6_OPT_PREFIX_INFORMATION=3,
    ND6_OPT_REDIRECTED_HEADER=4,
    ND6_OPT_MTU=5,
    ND6_OPT_ENDOFLIST=256
  };

struct nd6_router_solicit      /* router solicitation */
  {
    struct icmpv6hdr		rsol_hdr;
  };

#define rsol_type		rsol_hdr.icmpv6_type
#define rsol_code		rsol_hdr.icmpv6_code
#define rsol_cksum		rsol_hdr.icmpv6_cksum
#define rsol_reserved		rsol_hdr.icmpv6_data32[0]

struct nd6_router_advert
  {
    struct icmpv6hdr	radv_hdr;
    u_int32_t		radv_reachable;	 /* reachable time	*/
    u_int32_t		radv_retransmit; /* reachable retransmit time */
  };

#define radv_type		radv_hdr.icmpv6_type
#define radv_code		radv_hdr.icmpv6_code
#define radv_cksum		radv_hdr.icmpv6_cksum
#define radv_maxhoplimit	radv_hdr.icmpv6_data8[0]
#define radv_m_o_res		radv_hdr.icmpv6_data8[1]
#define ND6_RADV_M_BIT		0x80
#define ND6_RADV_O_BIT		0x40
#define radv_router_lifetime	radv_hdr.icmpv6_data16[1]

struct nd6_nsolicitation 	/* neighbor solicitation */
  {
    struct icmpv6hdr	nsol6_hdr;
    struct in6_addr	nsol6_target;
  };

struct nd6_nadvertisement 	/* neighbor advertisement */
  {
    struct icmpv6hdr	nadv6_hdr;
    struct in6_addr	nadv6_target;
  };

#define nadv6_flags			nadv6_hdr.icmpv6_data32[0]
#define ND6_NADVERFLAG_ISROUTER		0x80
#define ND6_NADVERFLAG_SOLICITED	0x40
#define ND6_NADVERFLAG_OVERRIDE		0x20

struct nd6_redirect            /* redirect */
  {
    struct icmpv6hdr	redirect_hdr;
    struct in6_addr	redirect_target;
    struct in6_addr	redirect_destination;
  };

struct nd6_opt_prefix_info     /* prefix information */
  {
    u_int8_t		opt_type;
    u_int8_t		opt_length;
    u_int8_t		opt_prefix_length;
    u_int8_t		opt_l_a_res;
    u_int32_t		opt_valid_life;
    u_int32_t		opt_preferred_life;
    u_int32_t		opt_reserved2;
    struct in6_addr	opt_prefix;
  };

#define ND6_OPT_PI_L_BIT	0x80
#define ND6_OPT_PI_A_BIT	0x40

struct nd6_opt_mtu 		/* MTU option */
  {
    u_int8_t		opt_type;
    u_int8_t		opt_length;
    u_int16_t		opt_reserved;
    u_int32_t		opt_mtu;
  };

#endif /* _NETINET6_ICMPV6_H */
