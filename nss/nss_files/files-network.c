/* Networks file parser in nss_files module.
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
#include <netdb.h>

#define ENTNAME		netent
#define DATABASE	"networks"

struct netent_data {};

#define TRAILING_LIST_MEMBER		n_aliases
#define TRAILING_LIST_SEPARATOR_P	isspace
#include "files-parse.c"
LINE_PARSER
("#",
 {
   char *addr;

   STRING_FIELD (result->n_name, isspace, 1);

   STRING_FIELD (addr, isspace, 1);
   result->n_net = inet_network (addr);

 })

#include "files-XXX.c"

DB_LOOKUP (netbyname, ,,
	   LOOKUP_NAME (n_name, n_aliases),
	   const char *name)

DB_LOOKUP (netbyaddr, ,,
	   {
	     if (result->n_addrtype == type && result->n_net == net)
	       /* Bingo!  */
	       break;
	   }, int type, unsigned long int net)
