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

#ifndef	_NETINET_IN_H

#define	_NETINET_IN_H	1
#include <features.h>

#include <sys/socket.h>
#include <sys/types.h>


__BEGIN_DECLS

/* Standard well-defined IP protocols.  */
enum
  {
    IPPROTO_IP = 0,	 /* Dummy protocol for TCP.  */
    IPPROTO_ICMP = 1,	 /* Internet Control Message Protocol.  */
    IPPROTO_IGMP = 2,	 /* Internet Group Management Protocol. */
    IPPROTO_IPIP = 4,	 /* IPIP tunnels (older KA9Q tunnels use 94).  */
    IPPROTO_TCP = 6,	 /* Transmission Control Protocol.  */
    IPPROTO_EGP = 8,	 /* Exterior Gateway Protocol.  */
    IPPROTO_PUP = 12,	 /* PUP protocol.  */
    IPPROTO_UDP = 17,	 /* User Datagram Protocol.  */
    IPPROTO_IDP = 22,	 /* XNS IDP protocol.  */
    IPPROTO_IPV6 = 41,   /* IPv6-in-IPv4 tunnelling.  */
    IPPROTO_ICMPV6 = 58, /* ICMPv6.  */

    IPPROTO_RAW = 255,	 /* Raw IP packets.  */
    IPPROTO_MAX
  };

/* Standard well-known ports.  */
enum
  {
    IPPORT_ECHO = 7,		/* Echo service.  */
    IPPORT_DISCARD = 9,		/* Discard transmissions service.  */
    IPPORT_SYSTAT = 11,		/* System status service.  */
    IPPORT_DAYTIME = 13,	/* Time of day service.  */
    IPPORT_NETSTAT = 15,	/* Network status service.  */
    IPPORT_FTP = 21,		/* File Transfer Protocol.  */
    IPPORT_TELNET = 23,		/* Telnet protocol.  */
    IPPORT_SMTP = 25,		/* Simple Mail Transfer Protocol.  */
    IPPORT_TIMESERVER = 37,	/* Timeserver service.  */
    IPPORT_NAMESERVER = 42,	/* Domain Name Service.  */
    IPPORT_WHOIS = 43,		/* Internet Whois service.  */
    IPPORT_MTP = 57,

    IPPORT_TFTP = 69,		/* Trivial File Transfer Protocol.  */
    IPPORT_RJE = 77,
    IPPORT_FINGER = 79,		/* Finger service.  */
    IPPORT_TTYLINK = 87,
    IPPORT_SUPDUP = 95,		/* SUPDUP protocol.  */


    IPPORT_EXECSERVER = 512,	/* execd service.  */
    IPPORT_LOGINSERVER = 513,	/* rlogind service.  */
    IPPORT_CMDSERVER = 514,
    IPPORT_EFSSERVER = 520,

    /* UDP ports.  */
    IPPORT_BIFFUDP = 512,
    IPPORT_WHOSERVER = 513,
    IPPORT_ROUTESERVER = 520,

    /* Ports less than this value are reserved for privileged processes.  */
    IPPORT_RESERVED = 1024,

    /* Ports greater this value are reserved for (non-privileged) servers.  */
    IPPORT_USERRESERVED = 5000
  };


/* Internet address.  */
struct in_addr
  {
    unsigned int s_addr;
  };


/* Definitions of the bits in an Internet address integer.

   On subnets, host and network parts are found according to
   the subnet mask, not these masks.  */

#define	IN_CLASSA(a)		((((unsigned) (a)) & 0x80000000) == 0)
#define	IN_CLASSA_NET		0xff000000
#define	IN_CLASSA_NSHIFT	24
#define	IN_CLASSA_HOST		(0xffffffff & ~IN_CLASSA_NET)
#define	IN_CLASSA_MAX		128

#define	IN_CLASSB(a)		((((unsigned) (a)) & 0xc0000000) == 0x80000000)
#define	IN_CLASSB_NET		0xffff0000
#define	IN_CLASSB_NSHIFT	16
#define	IN_CLASSB_HOST		(0xffffffff & ~IN_CLASSB_NET)
#define	IN_CLASSB_MAX		65536

#define	IN_CLASSC(a)		((((unsigned) (a)) & 0xc0000000) == 0xc0000000)
#define	IN_CLASSC_NET		0xffffff00
#define	IN_CLASSC_NSHIFT	8
#define	IN_CLASSC_HOST		(0xffffffff & ~IN_CLASSC_NET)

#define	IN_CLASSD(a)		((((unsigned) (a)) & 0xf0000000) == 0xe0000000)
#define	IN_MULTICAST(a)		IN_CLASSD(a)

#define	IN_EXPERIMENTAL(a)	((((unsigned) (a)) & 0xe0000000) == 0xe0000000)
#define	IN_BADCLASS(a)		((((unsigned) (a)) & 0xf0000000) == 0xf0000000)

/* Address to accept any incoming messages.  */
#define	INADDR_ANY		((unsigned) 0x00000000)
/* Address to send to all hosts.  */
#define	INADDR_BROADCAST	((unsigned) 0xffffffff)
/* Address indicating an error return.  */
#define	INADDR_NONE		((unsigned) 0xffffffff)

/* Network number for local host loopback.  */
#define	IN_LOOPBACKNET	127
/* Address to loopback in software to local host.  */
#ifndef INADDR_LOOPBACK
#define	INADDR_LOOPBACK	((unsigned) 0x7f000001)	/* Inet address 127.0.0.1.  */
#endif


/* IPv6 address */
struct in6_addr
  {
    union
      {
	u_int8_t	u6_addr8[16];
	u_int16_t	u6_addr16[8];
	u_int32_t	u6_addr32[4];
#if (~0UL) > 0xffffffff
	u_int64_t	u6_addr64[2];
#endif
      } in6_u;
#define s6_addr			in6_u.u6_addr8
#define s6_addr16		in6_u.u6_addr16
#define s6_addr32		in6_u.u6_addr32
#define s6_addr64		in6_u.u6_addr64
  };

extern const struct in6_addr in6addr_any;        /* :: */
extern const struct in6_addr in6addr_loopback;   /* ::1 */
#define IN6ADDR_ANY_INIT { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }
#define IN6ADDR_LOOPBACK_INIT { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1 }

#define INET_ADDRSTRLEN 16
#define INET6_ADDRSTRLEN 46

/* Get the definition of the macro to define the common sockaddr members.  */
#include <sockaddrcom.h>


/* Structure describing an Internet socket address.  */
struct sockaddr_in
  {
    __SOCKADDR_COMMON (sin_);
    unsigned short int sin_port;	/* Port number.  */
    struct in_addr sin_addr;		/* Internet address.  */

    /* Pad to size of `struct sockaddr'.  */
    unsigned char sin_zero[sizeof(struct sockaddr) -
			   __SOCKADDR_COMMON_SIZE -
			   sizeof(unsigned short int) -
			   sizeof(struct in_addr)];
  };

/* Ditto, for IPv6.  */
struct sockaddr_in6
  {
    __SOCKADDR_COMMON (sin6_);
    u_int16_t		sin6_port;      /* Transport layer port # */
    u_int32_t		sin6_flowinfo;  /* IPv6 flow information */
    struct in6_addr	sin6_addr;      /* IPv6 address */
  };

/* IPv6 multicast request.  */
struct ipv6_mreq
  {
    /* IPv6 multicast address of group */
    struct in6_addr ipv6mr_multiaddr;

    /* local IPv6 address of interface */
    int		ipv6mr_ifindex;
  };


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

/* Functions to convert between host and network byte order.

   Please note that these functions normally take `unsigned long int' or
   `unsigned short int' values as arguments and also return them.  But
   this was a short-sighted decision since on different systems the types
   may have different representations but the values are always the same.  */

extern u_int32_t ntohl __P ((u_int32_t __netlong));
extern u_int16_t ntohs __P ((u_int16_t __netshort));
extern u_int32_t htonl __P ((u_int32_t __hostlong));
extern u_int16_t htons __P ((u_int16_t __hostshort));

#include <endian.h>

#if __BYTE_ORDER == __BIG_ENDIAN
/* The host byte order is the same as network byte order,
   so these functions are all just identity.  */
#define	ntohl(x)	(x)
#define	ntohs(x)	(x)
#define	htonl(x)	(x)
#define	htons(x)	(x)
#endif


/* IPV6 socket options.  */
#define IPV6_ADDRFORM		1
#define IPV6_RXINFO		2
#define IPV6_RXHOPOPTS		3
#define IPV6_RXDSTOPTS		4
#define IPV6_RXSRCRT		5
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
#define IPV6_ADD_MEMBERSHIP	20
#define IPV6_DROP_MEMBERSHIP	21

#define IN6_IS_ADDR_UNSPECIFIED(a) \
        ((((u_int32_t *)(a))[0] == 0) && ((u_int32_t *)(a))[1] == 0) && \
         (((u_int32_t *)(a))[2] == 0) && ((u_int32_t *)(a))[3] == 0))

#define IN6_IS_ADDR_LOOPBACK(a) \
        ((((u_int32_t *)(a))[0] == 0) && ((u_int32_t *)(a))[1] == 0) && \
         (((u_int32_t *)(a))[2] == 0) && ((u_int32_t *)(a))[3] == htonl(1)))

#define IN6_IS_ADDR_MULTICAST(a) (((u_int8_t *)(a))[0] == 0xff)

#define IN6_IS_ADDR_LINKLOCAL(a) \
        ((((u_int32_t *)(a))[0] & htonl(0xffc00000)) == htonl(0xfe800000))

#define IN6_IS_ADDR_SITELOCAL(a) \
        ((((u_int32_t *)(a))[0] & htonl(0xffc00000)) == htonl(0xfec00000))

#define IN6_IS_ADDR_V4MAPPED(a) \
        ((((u_int32_t *)(a))[0] == 0) && (((u_int32_t *)(a))[1] == 0) && \
         (((u_int32_t *)(a))[2] == htonl(0xffff)))

#define IN6_IS_ADDR_V4COMPAT(a) \
        ((((u_int32_t *)(a))[0] == 0) && (((u_int32_t *)(a))[1] == 0) && \
         (((u_int32_t *)(a))[2] == 0) && (ntohl(((u_int32_t *)(a))[3]) > 1))


/* Bind socket to a priviledged IP port.  */
extern int bindresvport __P ((int __sockfd, struct sockaddr_in *__sin));

__END_DECLS

#endif	/* netinet/in.h */
