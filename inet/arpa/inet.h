/* Copyright (C) 1997 Free Software Foundation, Inc.
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

#ifndef _ARPA_INET_H
#define	_ARPA_INET_H	1

#include <features.h>

#include <sys/types.h>
#include <netinet/in.h>		/* To define `struct in_addr'.  */

__BEGIN_DECLS

/* Convert Internet host address from numbers-and-dots notation in CP
   into binary data in network byte order.  */
extern u_int32_t inet_addr __P ((__const char *__cp));

/* Convert Internet host address from numbers-and-dots notation in CP
   into binary data and store the result in the structure INP.  */
extern int inet_aton __P ((__const char *__cp, struct in_addr *__inp));

/* Return the local host address part of the Internet address in IN.  */
extern u_int32_t inet_lnaof __P ((struct in_addr __in));

/* Make Internet host address in network byte order by combining the
   network number NET with the local address HOST.  */
extern struct in_addr inet_makeaddr __P ((u_int32_t __net, u_int32_t __host));

/* Format a network number NET into presentation format and place result
   in buffer starting at BUF with length of LEN bytes.  */
extern char *inet_neta __P ((u_int32_t __net, char *__buf, size_t __len));

/* Return network number part of the Internet address IN.  */
extern u_int32_t inet_netof __P ((struct in_addr __in));

/* Extract the network number in network byte order from the address
   in numbers-and-dots natation starting at CP.  */
extern u_int32_t inet_network __P ((__const char *__cp));

/* Convert network number for interface type AF in buffer starting at
   CP to presentation format.  The result will specifiy BITS bits of
   the number.  */
extern char *inet_net_ntop __P ((int __af, __const void *__cp, int __bits,
				 char *__buf, size_t __len));

/* Convert network number for interface type AF from presentation in
   buffer starting at CP to network format and store result int
   buffer starting at BUF of size LEN.  */
extern int inet_net_pton __P ((int __af, __const char *__cp,
			       void *__buf, size_t __len));

/* Convert Internet number in IN to ASCII representation.  The return value
   is a pointer to an internal array containing the string.  */
extern char *inet_ntoa __P ((struct in_addr __in));

/* Convert from presentation format of an Internet number in buffer
   starting at CP to the binary network format and store result for
   interface type AF in buffer starting at BUF.  */
extern int inet_pton __P ((int __af, __const char *__cp, void *__buf));

/* Convert a Internet address in binary network format for interface
   type AF in buffer starting at CP to presentation form and place
   result in buffer of length LEN astarting at BUF.  */
extern __const char *inet_ntop __P ((int __af, __const void *__cp,
				     char *__buf, size_t __len));

/* Convert ASCII representation in hexadecimal form of the Internet
   address to binary form and place result in buffer of length LEN
   starting at BUF.  */
extern unsigned int inet_nsap_addr __P ((__const char *__cp,
					 unsigned char *__buf, int __len));

/* Convert internet address in binary form in LEN bytes starting at CP
   a presentation form and place result in BUF.  */
extern char *inet_nsap_ntoa __P ((int __len, __const unsigned char *__cp,
				  char *__buf));

__END_DECLS

#endif /* arpa/inet.h */
