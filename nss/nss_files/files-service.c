/* Services file parser in nss_files module.
   Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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

#include <netinet/in.h>
#include <netdb.h>


#define ENTNAME		servent
#define DATABASE	"services"

struct servent_data {};

#define TRAILING_LIST_MEMBER		s_aliases
#define TRAILING_LIST_SEPARATOR_P	isspace
#include "files-parse.c"
#define ISSLASH(c) ((c) == '/')
LINE_PARSER
("#",
 STRING_FIELD (result->s_name, isspace, 1);
 INT_FIELD (result->s_port, ISSLASH, 10, 0, htons);
 STRING_FIELD (result->s_proto, isspace, 1);
 )

#include GENERIC

DB_LOOKUP (servbyname, 2 + strlen (name) + (proto ? strlen (proto) : 0),
	   (".%s/%s", name, proto ?: ""),
	   {
	     /* Must match both protocol (if specified) and name.  */
	     if (proto != NULL && strcmp (result->s_proto, proto))
	       continue;
	     LOOKUP_NAME (s_name, s_aliases)
	   },
	   const char *name, const char *proto)

DB_LOOKUP (servbyport, 21 + (proto ? strlen (proto) : 0),
	   ("=%d/%s", ntohs (port), proto ?: ""),
	   {
	     /* Must match both port and protocol.  */
	     if (result->s_port == port
		 && (proto == NULL
		     || strcmp (result->s_proto, proto) == 0))
	       break;
	   }, int port, const char *proto)
