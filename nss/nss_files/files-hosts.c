/* Hosts file parser in nss_files module.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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
#include "../resolv/mapv4v6hostent.h"


#define ENTNAME		hostent
#define DATABASE	"hosts"
#define NEED_H_ERRNO

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
   if ((_res.options & RES_USE_INET6)
       && inet_pton (AF_INET6, addr, entdata->host_addr) > 0)
     {
       result->h_addrtype = AF_INET6;
       result->h_length = IN6ADDRSZ;
     }
   else if (inet_pton (AF_INET, addr, entdata->host_addr) > 0)
     {
       if (_res.options & RES_USE_INET6)
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
   else
     /* Illegal address: ignore line.  */
     return 0;

   /* Store a pointer to the address in the expected form.  */
   entdata->h_addr_ptrs[0] = entdata->host_addr;
   entdata->h_addr_ptrs[1] = NULL;
   result->h_addr_list = entdata->h_addr_ptrs;

   /* If we need the host entry in IPv6 form change it now.  */
   if (_res.options & RES_USE_INET6)
     {
       char *bufptr = data->linebuffer;
       int buflen = (char *) data + datalen - bufptr;
       map_v4v6_hostent (result, &bufptr, &buflen);
     }

   STRING_FIELD (result->h_name, isspace, 1);
 })

#include "files-XXX.c"

DB_LOOKUP (hostbyname, ,,
	   LOOKUP_NAME (h_name, h_aliases),
	   const char *name)

DB_LOOKUP (hostbyaddr, ,,
	   {
	     if (result->h_addrtype == type && result->h_length == len &&
		 ! memcmp (addr, result->h_addr_list[0], len))
	       break;
	   }, const char *addr, int len, int type)
