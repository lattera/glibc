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
|* Here we assume several symbols to be defined:		   *|
|* 								   *|
|* LOOKUP_TYPE   - the return type of the function		   *|
|* 								   *|
|* FUNCTION_NAME - name of the non-reentrant function		   *|
|* 								   *|
|* DATABASE_NAME - name of the database the function accesses	   *|
|*		   (e.g., host, services, ...)			   *|
|* 								   *|
|* ADD_PARAMS    - additional parameter, can vary in number	   *|
|* 								   *|
|* ADD_VARIABLES - names of additional parameter		   *|
|* 								   *|
|* Optionally the following vars can be defined:		   *|
|* 								   *|
|* NEED_H_ERRNO  - an extra parameter will be passed to point to   *|
|*		   the global `h_errno' variable.		   *|
|* 								   *|
|* NEED__RES     - the global _res variable might be used so we	   *|
|* 		   will have to initialize it if necessary	   *|
|* 								   *|
\*******************************************************************/

/* To make the real sources a bit prettier.  */
#define REENTRANT_NAME APPEND_R (FUNCTION_NAME)
#define APPEND_R(name) APPEND_R1 (name)
#define APPEND_R1(name) name##_r
#define INTERNAL(name) INTERNAL1 (name)
#define INTERNAL1(name) __##name

#define FUNCTION_NAME_STRING STRINGIZE (FUNCTION_NAME)
#define REENTRANT_NAME_STRING STRINGIZE (REENTRANT_NAME)
#define DATABASE_NAME_STRING STRINGIZE (DATABASE_NAME)
#define STRINGIZE(name) STRINGIZE1 (name)
#define STRINGIZE1(name) #name

#define DB_LOOKUP_FCT CONCAT3_1 (__nss_, DATABASE_NAME, _lookup)
#define CONCAT3_1(Pre, Name, Post) CONCAT3_2 (Pre, Name, Post)
#define CONCAT3_2(Pre, Name, Post) Pre##Name##Post

/* Sometimes we need to store error codes in the `h_errno' variable.  */
#ifdef NEED_H_ERRNO
# define H_ERRNO_PARM , int *h_errnop
# define H_ERRNO_VAR , h_errnop
#else
# define H_ERRNO_PARM
# define H_ERRNO_VAR
#endif


/* Type of the lookup function we need here.  */
typedef int (*lookup_function) (ADD_PARAMS, LOOKUP_TYPE *, char *, int
				H_ERRNO_PARM);

/* Some usages of this file might use this variable.  */
extern struct __res_state _res;

/* The lookup function for the first entry of this service.  */
extern int DB_LOOKUP_FCT (service_user **nip, const char *name, void **fctp);



int
INTERNAL (REENTRANT_NAME) (ADD_PARAMS, LOOKUP_TYPE *resbuf, char *buffer,
			   size_t buflen, LOOKUP_TYPE **result H_ERRNO_PARM)
{
  static service_user *startp = NULL;
  static lookup_function start_fct;
  service_user *nip;
  lookup_function fct;
  int no_more;
  enum nss_status status = NSS_STATUS_UNAVAIL;

  if (startp == NULL)
    {
      no_more = DB_LOOKUP_FCT (&nip, REENTRANT_NAME_STRING, (void **) &fct);
      if (no_more)
	startp = (service_user *) -1;
      else
	{
	  startp = nip;
	  start_fct = fct;

#ifdef NEED__RES
	  /* The resolver code will really be used so we have to
	     initialize it.  */
	  if ((_res.options & RES_INIT) == 0 && res_init () == -1)
	    {
	      *h_errnop = NETDB_INTERNAL;
	      return NULL;
	    }
#endif /* need _res */
	}
    }
  else
    {
      fct = start_fct;
      no_more = (nip = startp) == (service_user *) -1;
    }

  while (no_more == 0)
    {
      status = (*fct) (ADD_VARIABLES, resbuf, buffer, buflen H_ERRNO_VAR);

      no_more = __nss_next (&nip, REENTRANT_NAME_STRING,
			    (void **) &fct, status, 0);
    }

  *result = status == NSS_STATUS_SUCCESS ? resbuf : NULL;
  return status == NSS_STATUS_SUCCESS ? 0 : -1;
}

#define do_weak_alias(n1, n2) weak_alias ((n1), (n2))
do_weak_alias (INTERNAL (REENTRANT_NAME), REENTRANT_NAME)
