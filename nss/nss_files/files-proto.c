/* Protocols file parser in nss_files module.
Copyright (C) 1996 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <netdb.h>


#define ENTNAME		protoent
#define DATAFILE	_PATH_PROTOCOLS

struct protoent_data {};

#define TRAILING_LIST_MEMBER		p_aliases
#define TRAILING_LIST_SEPARATOR_P	isspace
#include "files-parse.c"
LINE_PARSER
(
 MIDLINE_COMMENTS;
 STRING_FIELD (result->p_name, isspace, 1);
 INT_FIELD (result->p_proto, isspace, 1, 10,);
 )

#include "files-XXX.c"

DB_LOOKUP (protobyname,
	   LOOKUP_NAME (p_name, p_aliases),
	   const char *name)

DB_LOOKUP (protobynumber,
	   {
	     if (result->p_proto == proto)
	       break;
	   }, int proto)
