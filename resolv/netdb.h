/* Copyright (C) 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it
   and/or modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be
   useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, write to the Free Software Foundation, Inc., 59 Temple Place -
   Suite 330, Boston, MA 02111-1307, USA.  */

/* All data returned by the network data base library are supplied in
   host order and returned in network order (suitable for use in
   system calls).  */

#ifndef	_NETDB_H

#define	_NETDB_H	1
#include <features.h>

/* This is necessary to make this include file properly replace the
   Sun version.  */
#include <rpc/netdb.h>

/* Absolute file name for network data base files.  */
#define	_PATH_HEQUIV		"/etc/hosts.equiv"
#define	_PATH_HOSTS		"/etc/hosts"
#define	_PATH_NETWORKS		"/etc/networks"
#define	_PATH_NSSWITCH_CONF	"/etc/nsswitch.conf"
#define	_PATH_PROTOCOLS		"/etc/protocols"
#define	_PATH_SERVICES		"/etc/services"


__BEGIN_DECLS

/* Error status for non-reentrant lookup functions.  */
extern int h_errno;

#if defined __USE_REENTRANT && (!defined _LIBC || defined _LIBC_REENTRANT)
/* Function to access thread specific `h_errno' variable.  */
extern int *__h_errno_location __P ((void)) __attribute__ ((__const__));

/* An alias name for above variable.  */
extern int __h_errno;

/* Use a macro to access always the thread specific `h_errno' variable.  */
#define h_errno (*__h_errno_location ())
#endif

/* Possible values left in `h_errno'.  */
#define	NETDB_INTERNAL	-1	/* See errno.  */
#define	NETDB_SUCCESS	0	/* No problem.  */
#define	HOST_NOT_FOUND	1	/* Authoritative Answer Host not found.  */
#define	TRY_AGAIN	2	/* Non-Authoritive Host not found,
				   or SERVERFAIL.  */
#define	NO_RECOVERY	3	/* Non recoverable errors, FORMERR, REFUSED,
				   NOTIMP.  */
#define	NO_DATA		4	/* Valid name, no data record of requested
				   type.  */
#define	NO_ADDRESS	NO_DATA	/* No address, look for MX record.  */

/* Print error indicated by `h_errno' variable on standard error.  STR
   if non-null is printed before the error string.  */
extern void herror __P ((__const char *__str));

/* Return string associated with error ERR_NUM.  */
extern __const char *hstrerror __P ((int __err_num));



/* Description of data base entry for a single host.  */
struct hostent
{
  char *h_name;			/* Official name of host.  */
  char **h_aliases;		/* Alias list.  */
  int h_addrtype;		/* Host address type.  */
  int h_length;			/* Length of address.  */
  char **h_addr_list;		/* List of addresses from name server.  */
#define	h_addr	h_addr_list[0]	/* Address, for backward compatiblity.  */
};

/* Open host data base files and mark them as staying open even after
   a later search if STAY_OPEN is non-zero.  */
extern void sethostent __P ((int __stay_open));

/* Close host data base files and clear `stay open' flag.  */
extern void endhostent __P ((void));

/* Get next entry from host data base file.  Open data base if
   necessary.  */
extern struct hostent *gethostent __P ((void));

/* Return entry from host data base which address match ADDR with
   length LEN and type TYPE.  */
extern struct hostent *gethostbyaddr __P ((__const char *__addr, int __len,
					   int __type));

/* Return entry from host data base for host with NAME.  */
extern struct hostent *gethostbyname __P ((__const char *__name));

/* Return entry from host data base for host with NAME.  AF must be
   set to the address type which as `AF_INET' for IPv4 or `AF_INET6'
   for IPv6.  */
extern struct hostent *gethostbyname2 __P ((__const char *__name, int __af));

#ifdef	__USE_REENTRANT
/* Reentrant versions of the functions above.  The additional
   arguments specify a buffer of BUFLEN starting at BUF.  The last
   argument is a pointer to a variable which gets the value which
   would be stored in the global variable `herrno' by the
   non-reentrant functions.  */
extern struct hostent *__gethostent_r __P ((struct hostent *__result_buf,
					    char *__buf, int __buf_len,
					    int *__h_errnop));
extern struct hostent *gethostent_r __P ((struct hostent *__result_buf,
					  char *__buf, int __buf_len,
					  int *__h_errnop));

extern struct hostent *__gethostbyaddr_r __P ((__const char *__addr, int __len,
					       int __type,
					       struct hostent *__result_buf,
					       char *__buf, int __buflen,
					       int *__h_errnop));
extern struct hostent *gethostbyaddr_r __P ((__const char *__addr, int __len,
					     int __type,
					     struct hostent *__result_buf,
					     char *__buf, int __buflen,
					     int *__h_errnop));

extern struct hostent *__gethostbyname_r __P ((__const char *__name,
					       struct hostent *__result_buf,
					       char *__buf, int __buflen,
					       int *__h_errnop));
extern struct hostent *gethostbyname_r __P ((__const char *__name,
					     struct hostent *__result_buf,
					     char *__buf, int __buflen,
					     int *__h_errnop));

extern struct hostent *__gethostbyname2_r __P ((__const char *__name, int __af,
						struct hostent *__result_buf,
						char *__buf, int __buflen,
						int *__h_errnop));
extern struct hostent *gethostbyname2_r __P ((__const char *__name, int __af,
					      struct hostent *__result_buf,
					      char *__buf, int __buflen,
					      int *__h_errnop));
#endif	/* reentrant */


/* Description of data base entry for a single network.  NOTE: here a
   poor assumption is made.  The network number is expected to fit
   into an unsigned long int variable.  */
struct netent
{
  char *n_name;			/* Official name of network.  */
  char **n_aliases;		/* Alias list.  */
  int n_addrtype;		/* Net address type.  */
  unsigned long	int n_net;	/* Network number.  */
};

/* Open network data base files and mark them as staying open even
   after a later search if STAY_OPEN is non-zero.  */
extern void setnetent __P ((int __stay_open));

/* Close network data base files and clear `stay open' flag.  */
extern void endnetent __P ((void));

/* Get next entry from network data base file.  Open data base if
   necessary.  */
extern struct netent *getnetent __P ((void));

/* Return entry from network data base which address match NET and
   type TYPE.  */
extern struct netent *getnetbyaddr __P ((unsigned long int __net,
					 int __type));

/* Return entry from network data base for network with NAME.  */
extern struct netent *getnetbyname __P ((__const char *__name));

#ifdef	__USE_REENTRANT
/* Reentrant versions of the functions above.  The additional
   arguments specify a buffer of BUFLEN starting at BUF.  The last
   argument is a pointer to a variable which gets the value which
   would be stored in the global variable `herrno' by the
   non-reentrant functions.  */
extern struct netent *__getnetent_r __P ((struct netent *__result_buf,
					  char *__buf, int __buf_len,
					  int *__h_errnop));
extern struct netent *getnetent_r __P ((struct netent *__result_buf,
					char *__buf, int __buf_len,
					int *__h_errnop));

extern struct netent *__getnetbyaddr_r __P ((unsigned long int __net,
					     int __type,
					     struct netent *__result_buf,
					     char *__buf, int __buflen,
					     int *__h_errnop));
extern struct netent *getnetbyaddr_r __P ((unsigned long int __net,
					   int __type,
					   struct netent *__result_buf,
					   char *__buf, int __buflen,
					   int *__h_errnop));

extern struct netent *__getnetbyname_r __P ((__const char *__name,
					     struct netent *__result_buf,
					     char *__buf, int __buflen,
					     int *__h_errnop));
extern struct netent *getnetbyname_r __P ((__const char *__name,
					   struct netent *__result_buf,
					   char *__buf, int __buflen,
					   int *__h_errnop));
#endif	/* reentrant */


/* Description of data base entry for a single service.  */
struct servent
{
  char *s_name;			/* Official service name.  */
  char **s_aliases;		/* Alias list.  */
  int s_port;			/* Port number.  */
  char *s_proto;		/* Protocol to use.  */
};

/* Open service data base files and mark them as staying open even
   after a later search if STAY_OPEN is non-zero.  */
extern void setservent __P ((int __stay_open));

/* Close service data base files and clear `stay open' flag.  */
extern void endservent __P ((void));

/* Get next entry from service data base file.  Open data base if
   necessary.  */
extern struct servent *getservent __P ((void));

/* Return entry from network data base for network with NAME and
   protocol PROTO.  */
extern struct servent *getservbyname __P ((__const char *__name,
					   __const char *__proto));

/* Return entry from service data base which matches port PORT and
   protocol PROTO.  */
extern struct servent *getservbyport __P ((int __port, __const char *__proto));


#ifdef	__USE_REENTRANT
/* Reentrant versions of the functions above.  The additional
   arguments specify a buffer of BUFLEN starting at BUF.  */
extern struct servent *__getservent_r __P ((struct servent *__result_buf,
					    char *__buf, int __buf_len));
extern struct servent *getservent_r __P ((struct servent *__result_buf,
					  char *__buf, int __buf_len));

extern struct servent *__getservbyname_r __P ((__const char *__name,
					       __const char *__proto,
					       struct servent *__result_buf,
					       char *__buf, int __buflen));
extern struct servent *getservbyname_r __P ((__const char *__name,
					     __const char *__proto,
					     struct servent *__result_buf,
					     char *__buf, int __buflen));

extern struct servent *__getservbyport_r __P ((int __port,
					       __const char *__proto,
					       struct servent *__result_buf,
					       char *__buf, int __buflen));
extern struct servent *getservbyport_r __P ((int __port, __const char *__proto,
					     struct servent *__result_buf,
					     char *__buf, int __buflen));
#endif	/* reentrant */


/* Description of data base entry for a single service.  */
struct protoent
{
  char *p_name;			/* Official protocol name.  */
  char **p_aliases;		/* Alias list.  */
  int p_proto;			/* Protocol number.  */
};

/* Open protocol data base files and mark them as staying open even
   after a later search if STAY_OPEN is non-zero.  */
extern void setprotoent __P ((int __stay_open));

/* Close protocol data base files and clear `stay open' flag.  */
extern void endprotoent __P ((void));

/* Get next entry from protocol data base file.  Open data base if
   necessary.  */
extern struct protoent *getprotoent __P ((void));

/* Return entry from protocol data base for network with NAME.  */
extern struct protoent *getprotobyname __P ((__const char *__name));

/* Return entry from protocol data base which number is PROTO.  */
extern struct protoent *getprotobynumber __P ((int __proto));


#ifdef	__USE_REENTRANT
/* Reentrant versions of the functions above.  The additional
   arguments specify a buffer of BUFLEN starting at BUF.  */
extern struct protoent *__getprotoent_r __P ((struct protoent *__result_buf,
					      char *__buf, int __buf_len));
extern struct protoent *getprotoent_r __P ((struct protoent *__result_buf,
					    char *__buf, int __buf_len));

extern struct protoent *__getprotobyname_r __P ((__const char *__name,
						 struct protoent *__result_buf,
						 char *__buf, int __buflen));
extern struct protoent *getprotobyname_r __P ((__const char *__name,
					       struct protoent *__result_buf,
					       char *__buf, int __buflen));

extern struct protoent *__getprotobynumber_r __P ((int __proto,
						   struct protoent *__res_buf,
						   char *__buf, int __buflen));
extern struct protoent *getprotobynumber_r __P ((int __proto,
						 struct protoent *__result_buf,
						 char *__buf, int __buflen));
#endif	/* reentrant */


__END_DECLS

#endif	/* netdb.h */
