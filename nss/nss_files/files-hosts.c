/* Hosts file parser in nss_files module.
   Copyright (C) 1996, 1997, 1998, 1999, 2000 Free Software Foundation, Inc.
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

#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <netdb.h>
#include <resolv.h>


/* Get implementation for some internal functions.  */
#include "../resolv/mapv4v6addr.h"


#define ENTNAME		hostent
#define DATABASE	"hosts"
#define NEED_H_ERRNO

#define EXTRA_ARGS	 , af, flags
#define EXTRA_ARGS_DECL	 , int af, int flags

#define ENTDATA hostent_data
struct hostent_data
  {
    unsigned char host_addr[16]; /* IPv4 or IPv6 address.  */
    char *h_addr_ptrs[2];	/* Points to that and null terminator.  */
  };

#define TRAILING_LIST_MEMBER		h_aliases
#define TRAILING_LIST_SEPARATOR_P	isspace
#include "files-parse.c"
LINE_PARSER
("#",
 {
   char *addr;

   STRING_FIELD (addr, isspace, 1);

   /* Parse address.  */
   if (af == AF_INET && inet_pton (AF_INET, addr, entdata->host_addr) > 0)
     {
       if (flags & AI_V4MAPPED)
	 {
	   map_v4v6_address ((char *) entdata->host_addr,
			     (char *) entdata->host_addr);
	   result->h_addrtype = AF_INET6;
	   result->h_length = IN6ADDRSZ;
	 }
       else
	 {
	   result->h_addrtype = AF_INET;
	   result->h_length = INADDRSZ;
	 }
     }
   else if (af == AF_INET6
	    && inet_pton (AF_INET6, addr, entdata->host_addr) > 0)
     {
       result->h_addrtype = AF_INET6;
       result->h_length = IN6ADDRSZ;
     }
   else
     /* Illegal address: ignore line.  */
     return 0;

   /* Store a pointer to the address in the expected form.  */
   entdata->h_addr_ptrs[0] = entdata->host_addr;
   entdata->h_addr_ptrs[1] = NULL;
   result->h_addr_list = entdata->h_addr_ptrs;

   STRING_FIELD (result->h_name, isspace, 1);
 })

#define EXTRA_ARGS_VALUE \
  , ((_res.options & RES_USE_INET6) ? AF_INET6 : AF_INET),		      \
  ((_res.options & RES_USE_INET6) ? AI_V4MAPPED : 0)
#include "files-XXX.c"

DB_LOOKUP (hostbyname, ,,
	   {
	     LOOKUP_NAME_CASE (h_name, h_aliases)
	   }, const char *name)

#undef EXTRA_ARGS_VALUE
#define EXTRA_ARGS_VALUE \
  , af, ((_res.options & RES_USE_INET6) ? AI_V4MAPPED : 0)
DB_LOOKUP (hostbyname2, ,,
	   {
	     LOOKUP_NAME_CASE (h_name, h_aliases)
	   }, const char *name, int af)

DB_LOOKUP (hostbyaddr, ,,
	   {
	     if (result->h_length == len
		 && ! memcmp (addr, result->h_addr_list[0], len))
	       break;
	   }, const void *addr, socklen_t len, int af)

#undef EXTRA_ARGS_VALUE
#define EXTRA_ARGS_VALUE \
  , af, flags
DB_LOOKUP (ipnodebyname, ,,
	   {
	     LOOKUP_NAME_CASE (h_name, h_aliases)
	   }, const char *name, int af, int flags)
