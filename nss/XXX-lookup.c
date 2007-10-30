/* Copyright (C) 1996, 1997, 1999, 2000, 2002, 2007 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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

#include "nsswitch.h"

/*******************************************************************\
|* Here we assume one symbol to be defined:			   *|
|* 								   *|
|* DATABASE_NAME - name of the database the function accesses	   *|
|*		   (e.g., hosts, services, ...)			   *|
|* 								   *|
|* One additional symbol may optionally be defined:		   *|
|* 								   *|
|* ALTERNATE_NAME - name of another service which is examined in   *|
|*                  case DATABASE_NAME is not found                *|
|* 								   *|
|* DEFAULT_CONFIG - string for default conf (e.g. "dns files")	   *|
|* 								   *|
\*******************************************************************/

#define DB_LOOKUP_FCT CONCAT3_1 (__nss_, DATABASE_NAME, _lookup2)
#define DB_COMPAT_FCT CONCAT3_1 (__nss_, DATABASE_NAME, _lookup)
#define CONCAT3_1(Pre, Name, Post) CONCAT3_2 (Pre, Name, Post)
#define CONCAT3_2(Pre, Name, Post) Pre##Name##Post

#define DATABASE_NAME_SYMBOL CONCAT3_1 (__nss_, DATABASE_NAME, _database)
#define DATABASE_NAME_STRING STRINGIFY1 (DATABASE_NAME)
#define STRINGIFY1(Name) STRINGIFY2 (Name)
#define STRINGIFY2(Name) #Name

#ifdef ALTERNATE_NAME
#define ALTERNATE_NAME_STRING STRINGIFY1 (ALTERNATE_NAME)
#else
#define ALTERNATE_NAME_STRING NULL
#endif

#ifndef DEFAULT_CONFIG
#define DEFAULT_CONFIG NULL
#endif

service_user *DATABASE_NAME_SYMBOL attribute_hidden;

extern int DB_LOOKUP_FCT (service_user **ni, const char *fct_name,
			  const char *fct2_name, void **fctp)
  internal_function;
libc_hidden_proto (DB_LOOKUP_FCT)

int
internal_function
DB_LOOKUP_FCT (service_user **ni, const char *fct_name, const char *fct2_name,
	       void **fctp)
{
  if (DATABASE_NAME_SYMBOL == NULL
      && __nss_database_lookup (DATABASE_NAME_STRING, ALTERNATE_NAME_STRING,
				DEFAULT_CONFIG, &DATABASE_NAME_SYMBOL) < 0)
    return -1;

  *ni = DATABASE_NAME_SYMBOL;

  return __nss_lookup (ni, fct_name, fct2_name, fctp);
}
libc_hidden_def (DB_LOOKUP_FCT)


#ifndef NO_COMPAT
int
internal_function attribute_compat_text_section
DB_COMPAT_FCT (service_user **ni, const char *fct_name, void **fctp)
{
  return DB_LOOKUP_FCT (ni, fct_name, NULL, fctp);
}
#endif
