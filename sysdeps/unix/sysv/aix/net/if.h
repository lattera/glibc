/* Copyright (C) 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _NET_IF_H

#define _NET_IF_H	1
#include <features.h>

#include <sys/types.h>
#include <sys/socket.h>

/* Standard interface flags. */
enum
  {
    IFF_UP = 0x1,		/* Interface is up.  */
#define IFF_UP	IFF_UP
    IFF_BROADCAST = 0x2,	/* Broadcast address valid.  */
#define IFF_BROADCAST	IFF_BROADCAST
    IFF_DEBUG = 0x4,		/* Turn on debugging.  */
#define IFF_DEBUG	IFF_DEBUG
    IFF_LOOPBACK = 0x8,		/* Is a loopback net.  */
#define IFF_LOOPBACK	IFF_LOOPBACK
    IFF_POINTOPOINT = 0x10,	/* Interface is point-to-point link.  */
#define IFF_POINTOPOINT	IFF_POINTOPOINT
    IFF_NOTRAILERS = 0x20,	/* Avoid use of trailers.  */
#define IFF_NOTRAILERS	IFF_NOTRAILERS
    IFF_RUNNING = 0x40,		/* Resources allocated.  */
#define IFF_RUNNING	IFF_RUNNING
    IFF_NOARP = 0x80,		/* No address resolution protocol.  */
#define IFF_NOARP	IFF_NOARP
    IFF_PROMISC = 0x100,	/* Receive all packets.  */
#define IFF_PROMISC	IFF_PROMISC

    /* Not supported */
    IFF_ALLMULTI = 0x200,	/* Receive all multicast packets.  */
#define IFF_ALLMULTI	IFF_ALLMULTI

    IFF_OACTIVE = 0x400,	/* Transmission in progress.  */
#define IFF_OACTIVE	IFF_OACTIVE
    IFF_SIMPLEX = 0x800,	/* Cannot hear own transmissions.  */
#define IFF_SIMPLEX	IFF_SIMPLEX
    IFF_DO_HW_LOOPBACK = 0x10000, /* Force loopback through hardware.  */
#define IFF_DO_HW_LOOPBACK	IFF_DO_HW_LOOPBACK
    IFF_ALLCAST = 0x20000,	/* Global broadcast.  */
#define IFF_ALLCAST	IFF_ALLCAST
    IFF_BRIDGE = 0x40000,	/* Receive all bridge packets.  */
#define IFF_BRIDGE	IFF_BRIDGE
    IFF_NOECHO = IFF_SIMPLEX,	/* Reeives echo packets.  */
#define IFF_NOECHO	IFF_NOECHO
  };

/* The ifaddr structure contains information about one address of an
   interface.  They are maintained by the different address families,
   are allocated and attached when an address is set, and are linked
   together so all addresses for an interface can be located.  */

struct ifaddr
  {
    struct sockaddr ifa_addr;	/* Address of interface.  */
    union
      {
	struct sockaddr	ifu_broadaddr;
	struct sockaddr	ifu_dstaddr;
      } ifa_ifu;
    struct sockaddr *ifa_netmask; /* Used to determine subnet.  */
    struct iface *ifa_ifp;	/* Back-pointer to interface.  */
    struct ifaddr *ifa_next;	/* Next address for interface.  */
    void (*ifa_rtrequest) (void);
    struct rtentry *ifa_rt;
    unsigned short int ifa_flags;
    short int ifa_refcnt;
  };

#define	ifa_broadaddr	ifa_ifu.ifu_broadaddr	/* broadcast address	*/
#define	ifa_dstaddr	ifa_ifu.ifu_dstaddr	/* other end of link	*/

/* Interface request structure used for socket ioctl's.  All interface
   ioctl's must have parameter definitions which begin with ifr_name.
   The remainder may be interface specific.  */

struct ifreq
  {
#define IFHWADDRLEN	6
#define	IFNAMSIZ	16
    union
      {
	char ifrn_name[IFNAMSIZ];	/* Interface name, e.g. "en0".  */
      } ifr_ifrn;

    union
      {
	struct sockaddr ifru_addr;
	struct sockaddr ifru_dstaddr;
	struct sockaddr ifru_broadaddr;
	struct sockaddr ifru_netmask;
	struct sockaddr ifru_hwaddr;
	short int ifru_flags;
	int ifru_ivalue;
	unsigned int ifru_mtu;
	char ifru_slave[IFNAMSIZ];	/* Just fits the size */
	__caddr_t ifru_data;
	unsigned short int ifru_site6;
      } ifr_ifru;
  };

/* Old AIX 3.1 version.  */
struct oifreq
{
  char ifr_name[IFNAMSIZ];		/* if name, e.g. "en0" */
  union
  {
    struct  sockaddr ifru_addr;
    struct  sockaddr ifru_dstaddr;
    struct  sockaddr ifru_broadaddr;
    int ifru_flags;
    int ifru_metric;
    caddr_t ifru_data;
    unsigned int ifru_mtu;
  } ifr_ifru;
  unsigned char reserved[8];
};


#define ifr_name	ifr_ifrn.ifrn_name	/* interface name 	*/
#define ifr_hwaddr	ifr_ifru.ifru_hwaddr	/* MAC address 		*/
#define	ifr_addr	ifr_ifru.ifru_addr	/* address		*/
#define	ifr_dstaddr	ifr_ifru.ifru_dstaddr	/* other end of p-p lnk	*/
#define	ifr_broadaddr	ifr_ifru.ifru_broadaddr	/* broadcast address	*/
#define	ifr_netmask	ifr_ifru.ifru_netmask	/* interface net mask	*/
#define	ifr_flags	ifr_ifru.ifru_flags	/* flags		*/
#define	ifr_metric	ifr_ifru.ifru_ivalue	/* metric		*/
#define	ifr_mtu		ifr_ifru.ifru_mtu	/* mtu			*/
#define ifr_slave	ifr_ifru.ifru_slave	/* slave device		*/
#define	ifr_data	ifr_ifru.ifru_data	/* for use by interface	*/
#define ifr_ifindex	ifr_ifru.ifru_ivalue    /* interface index      */
#define ifr_bandwidth	ifr_ifru.ifru_ivalue	/* link bandwidth	*/
#define ifr_baudrate	ifr_ifru.ifru_ivalue	/* link bandwidth	*/
#define ifr_qlen	ifr_ifru.ifru_ivalue	/* queue length		*/
#define ifr_site6	ifr_ifru.ifru_site6	/* IPv6 site index      */


/* Structure used in SIOCGIFCONF request.  Used to retrieve interface
   configuration for machine (useful for programs which must know all
   networks accessible).  */

struct ifconf
  {
    int	ifc_len;			/* Size of buffer.  */
    union
      {
	__caddr_t ifcu_buf;
	struct ifreq *ifcu_req;
      } ifc_ifcu;
  };
#define	ifc_buf	ifc_ifcu.ifcu_buf	/* Buffer address.  */
#define	ifc_req	ifc_ifcu.ifcu_req	/* Array of structures.  */

__BEGIN_DECLS

/* Convert an interface name to an index, and vice versa.  */

extern unsigned int if_nametoindex (__const char *__ifname) __THROW;
extern char *if_indextoname (unsigned int __ifindex, char *__ifname) __THROW;

/* Return a list of all interfaces and their indices.  */

struct if_nameindex
  {
    unsigned int if_index;	/* 1, 2, ... */
    char *if_name;		/* null terminated name: "eth0", ... */
  };

extern struct if_nameindex *if_nameindex (void) __THROW;

/* Free the data returned from if_nameindex.  */

extern void if_freenameindex (struct if_nameindex *__ptr) __THROW;

__END_DECLS

#endif /* net/if.h */
