/* Copyright (C) 1991, 92, 93, 95, 96, 97 Free Software Foundation, Inc.
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

/* This header file was taken from linux (2.1.26) sources and modified
 * to work under GNU LIBC 2.0.
 */

/*
 * Copyright (c) 1993 Daniel Boulet
 * Copyright (c) 1994 Ugen J.S.Antsilevich
 *
 * Redistribution and use in source forms, with and without modification,
 * are permitted provided that this entire comment appears intact.
 *
 * Redistribution in binary form may occur without any restrictions.
 * Obviously, it would be nice if you gave credit where credit is due
 * but requiring it would be too onerous.
 *
 * This software is provided ``AS IS'' without any warranties of any kind.
 */

/*
 * 	Format of an IP firewall descriptor
 *
 * 	src, dst, src_mask, dst_mask are always stored in network byte order.
 * 	flags and num_*_ports are stored in host byte order (of course).
 * 	Port numbers are stored in HOST byte order.
 */
 
#ifndef _NETINET_FW_H
#define _NETINET_FW_H

#include <sys/cdefs.h>
#include <sys/types.h>

#include <netinet/icmp.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

__BEGIN_DECLS

struct ip_fw {
  struct ip_fw  *fw_next;		/* Next firewall on chain */
  struct in_addr fw_src, fw_dst;	/* Source and destination IP addr */
  struct in_addr fw_smsk, fw_dmsk;	/* Mask for src and dest IP addr */
  struct in_addr fw_via;		/* IP address of interface "via" */
  void *fw_viadev;	                /* device of interface "via" */
  u_int16_t fw_flg;		        /* Flags word */
  u_int16_t fw_nsp, fw_ndp;             /* N'of src ports and # of dst ports */
					/* in ports array (dst ports follow */
    					/* src ports; max of 10 ports in all;*/
    					/* count of 0 means match all ports) */
#define IP_FW_MAX_PORTS	10      	/* A reasonable maximum */
  u_int16_t fw_pts[IP_FW_MAX_PORTS];    /* Array of port numbers to match */
  u_int32_t fw_pcnt, fw_bcnt;		/* Packet and byte counters */
  u_int8_t fw_tosand, fw_tosxor;	/* Revised packet priority */
  char fw_vianame[IFNAMSIZ];	        /* name of interface "via" */
};

/*
 *	Values for "flags" field .
 */

#define IP_FW_F_ALL	0x0000	/* This is a universal packet firewall*/
#define IP_FW_F_TCP	0x0001	/* This is a TCP packet firewall      */
#define IP_FW_F_UDP	0x0002	/* This is a UDP packet firewall      */
#define IP_FW_F_ICMP	0x0003	/* This is a ICMP packet firewall     */
#define IP_FW_F_KIND	0x0003	/* Mask to isolate firewall kind      */
#define IP_FW_F_ACCEPT	0x0004	/* This is an accept firewall (as     *
				 *         opposed to a deny firewall)*
				 *                                    */
#define IP_FW_F_SRNG	0x0008	/* The first two src ports are a min  *
				 * and max range (stored in host byte *
				 * order).                            *
				 *                                    */
#define IP_FW_F_DRNG	0x0010	/* The first two dst ports are a min  *
				 * and max range (stored in host byte *
				 * order).                            *
				 * (ports[0] <= port <= ports[1])     *
				 *                                    */
#define IP_FW_F_PRN	0x0020	/* In verbose mode print this firewall*/
#define IP_FW_F_BIDIR	0x0040	/* For bidirectional firewalls        */
#define IP_FW_F_TCPSYN	0x0080	/* For tcp packets-check SYN only     */
#define IP_FW_F_ICMPRPL 0x0100	/* Send back icmp unreachable packet  */
#define IP_FW_F_MASQ	0x0200	/* Masquerading			      */
#define IP_FW_F_TCPACK	0x0400	/* For tcp-packets match if ACK is set*/
#define IP_FW_F_REDIR	0x0800	/* Redirect to local port fw_pts[n]   */
#define IP_FW_F_ACCTIN  0x1000	/* Account incoming packets only.     */
#define IP_FW_F_ACCTOUT 0x2000	/* Account outgoing packets only.     */

#define IP_FW_F_MASK	0x3FFF	/* All possible flag bits mask        */

/*    
 *	New IP firewall options for [gs]etsockopt at the RAW IP level.
 *	Unlike BSD Linux inherits IP options so you don't have to use
 *	a raw socket for this. Instead we check rights in the calls.
 */     

#define IP_FW_BASE_CTL  	64	/* base for firewall socket options */

#define IP_FW_COMMAND		0x00FF	/* mask for command without chain */
#define IP_FW_TYPE		0x0300	/* mask for type (chain) */
#define IP_FW_SHIFT		8	/* shift count for type (chain) */

#define IP_FW_FWD		0
#define IP_FW_IN		1
#define IP_FW_OUT		2
#define IP_FW_ACCT		3
#define IP_FW_CHAINS		4	/* total number of ip_fw chains */

#define IP_FW_INSERT		(IP_FW_BASE_CTL)
#define IP_FW_APPEND		(IP_FW_BASE_CTL+1)
#define IP_FW_DELETE		(IP_FW_BASE_CTL+2)
#define IP_FW_FLUSH		(IP_FW_BASE_CTL+3)
#define IP_FW_ZERO		(IP_FW_BASE_CTL+4)
#define IP_FW_POLICY		(IP_FW_BASE_CTL+5)
#define IP_FW_CHECK		(IP_FW_BASE_CTL+6)
#define IP_FW_MASQ_TIMEOUTS	(IP_FW_BASE_CTL+7)

#define IP_FW_INSERT_FWD	(IP_FW_INSERT | (IP_FW_FWD << IP_FW_SHIFT))
#define IP_FW_APPEND_FWD	(IP_FW_APPEND | (IP_FW_FWD << IP_FW_SHIFT))
#define IP_FW_DELETE_FWD	(IP_FW_DELETE | (IP_FW_FWD << IP_FW_SHIFT))
#define IP_FW_FLUSH_FWD		(IP_FW_FLUSH  | (IP_FW_FWD << IP_FW_SHIFT))
#define IP_FW_ZERO_FWD		(IP_FW_ZERO   | (IP_FW_FWD << IP_FW_SHIFT))
#define IP_FW_POLICY_FWD	(IP_FW_POLICY | (IP_FW_FWD << IP_FW_SHIFT))
#define IP_FW_CHECK_FWD		(IP_FW_CHECK  | (IP_FW_FWD << IP_FW_SHIFT))

#define IP_FW_INSERT_IN		(IP_FW_INSERT | (IP_FW_IN << IP_FW_SHIFT))
#define IP_FW_APPEND_IN		(IP_FW_APPEND | (IP_FW_IN << IP_FW_SHIFT))
#define IP_FW_DELETE_IN		(IP_FW_DELETE | (IP_FW_IN << IP_FW_SHIFT))
#define IP_FW_FLUSH_IN		(IP_FW_FLUSH  | (IP_FW_IN << IP_FW_SHIFT))
#define IP_FW_ZERO_IN		(IP_FW_ZERO   | (IP_FW_IN << IP_FW_SHIFT))
#define IP_FW_POLICY_IN		(IP_FW_POLICY | (IP_FW_IN << IP_FW_SHIFT))
#define IP_FW_CHECK_IN		(IP_FW_CHECK  | (IP_FW_IN << IP_FW_SHIFT))

#define IP_FW_INSERT_OUT	(IP_FW_INSERT | (IP_FW_OUT << IP_FW_SHIFT))
#define IP_FW_APPEND_OUT	(IP_FW_APPEND | (IP_FW_OUT << IP_FW_SHIFT))
#define IP_FW_DELETE_OUT	(IP_FW_DELETE | (IP_FW_OUT << IP_FW_SHIFT))
#define IP_FW_FLUSH_OUT		(IP_FW_FLUSH  | (IP_FW_OUT << IP_FW_SHIFT))
#define IP_FW_ZERO_OUT		(IP_FW_ZERO   | (IP_FW_OUT << IP_FW_SHIFT))
#define IP_FW_POLICY_OUT	(IP_FW_POLICY | (IP_FW_OUT << IP_FW_SHIFT))
#define IP_FW_CHECK_OUT		(IP_FW_CHECK  | (IP_FW_OUT << IP_FW_SHIFT))

#define IP_ACCT_INSERT		(IP_FW_INSERT | (IP_FW_ACCT << IP_FW_SHIFT))
#define IP_ACCT_APPEND		(IP_FW_APPEND | (IP_FW_ACCT << IP_FW_SHIFT))
#define IP_ACCT_DELETE		(IP_FW_DELETE | (IP_FW_ACCT << IP_FW_SHIFT))
#define IP_ACCT_FLUSH		(IP_FW_FLUSH  | (IP_FW_ACCT << IP_FW_SHIFT))
#define IP_ACCT_ZERO		(IP_FW_ZERO   | (IP_FW_ACCT << IP_FW_SHIFT))

struct ip_fwpkt
{
  struct iphdr fwp_iph;			/* IP header */
  union {
    struct tcphdr fwp_tcph;		/* TCP header or */
    struct udphdr fwp_udph;		/* UDP header */
    struct icmphdr fwp_icmph;	        /* ICMP header */
  } fwp_protoh;
  struct in_addr fwp_via;	        /* interface address */
  char fwp_vianame[IFNAMSIZ];	        /* interface name */
};

/*
 * timeouts for ip masquerading
 */

struct ip_fw_masq;
  
__END_DECLS

#endif /* _NETINET_IP_FW_H */
