/* Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include "nsswitch.h"

/*******************************************************************\
|* Here we assume one symbol to be defined:			   *|
|* 								   *|
|* DATABASE_NAME - name of the database the function accesses	   *|
|*		   (e.g., hosts, servicess, ...)		   *|
|* 								   *|
|* One additional symbol may optionally be defined:		   *|
|* 								   *|
|* DEFAULT_CONFIG - string for default conf (e.g. "dns files")	   *|
|* 								   *|
\*******************************************************************/

#define DB_LOOKUP_FCT CONCAT3_1 (__nss_, DATABASE_NAME, _lookup)
#define CONCAT3_1(Pre, Name, Post) CONCAT3_2 (Pre, Name, Post)
#define CONCAT3_2(Pre, Name, Post) Pre##Name##Post

#define DATABASE_NAME_STRING STRINGIFY1 (DATABASE_NAME)
#define STRINGIFY1(Name) STRINGIFY2 (Name)
#define STRINGIFY2(Name) #Name

#ifndef DEFAULT_CONFIG
#define DEFAULT_CONFIG 0
#endif

static service_user *database = NULL;

int
DB_LOOKUP_FCT (service_user **ni, const char *fct_name, void **fctp)
{
  if (database == NULL
      && __nss_database_lookup (DATABASE_NAME_STRING, DEFAULT_CONFIG,
				&database) < 0)
    return -1;

  *ni = database;

  return __nss_lookup (ni, fct_name, fctp);
}
