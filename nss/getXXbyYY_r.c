/* Copyright (C) 1996,97,98,99,2000,2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <assert.h>
#include <errno.h>
#include "nsswitch.h"
#ifdef USE_NSCD
# include <nscd/nscd_proto.h>
#endif
#ifdef NEED__RES_HCONF
# include <resolv/res_hconf.h>
#endif
#ifdef NEED__RES
# include <resolv.h>
#endif
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
|* PREPROCESS    - code run before anything else		   *|
|* 								   *|
|* POSTPROCESS   - code run after the lookup			   *|
|* 								   *|
\*******************************************************************/

/* To make the real sources a bit prettier.  */
#define REENTRANT_NAME APPEND_R (FUNCTION_NAME)
#define APPEND_R(name) APPEND_R1 (name)
#define APPEND_R1(name) name##_r
#define INTERNAL(name) INTERNAL1 (name)
#define INTERNAL1(name) __##name

#ifdef USE_NSCD
# define NSCD_NAME ADD_NSCD (REENTRANT_NAME)
# define ADD_NSCD(name) ADD_NSCD1 (name)
# define ADD_NSCD1(name) __nscd_##name
# define NOT_USENSCD_NAME ADD_NOT_NSCDUSE (DATABASE_NAME)
# define ADD_NOT_NSCDUSE(name) ADD_NOT_NSCDUSE1 (name)
# define ADD_NOT_NSCDUSE1(name) __nss_not_use_nscd_##name
#endif

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
# define H_ERRNO_VAR_P h_errnop
#else
# define H_ERRNO_PARM
# define H_ERRNO_VAR
# define H_ERRNO_VAR_P NULL
#endif

#ifndef HAVE_TYPE
# define TYPE_VAR_P NULL
# define FLAGS_VAR 0
#endif

#ifdef HAVE_AF
# define AF_VAR_P &af
#else
# define AF_VAR_P NULL
#endif

/* Type of the lookup function we need here.  */
typedef enum nss_status (*lookup_function) (ADD_PARAMS, LOOKUP_TYPE *, char *,
					    size_t, int * H_ERRNO_PARM);

/* The lookup function for the first entry of this service.  */
extern int DB_LOOKUP_FCT (service_user **nip, const char *name, void **fctp);

/* Interval in which we transfer retry to contact the NSCD.  */
#define NSS_NSCD_RETRY	100


int
INTERNAL (REENTRANT_NAME) (ADD_PARAMS, LOOKUP_TYPE *resbuf, char *buffer,
			   size_t buflen, LOOKUP_TYPE **result H_ERRNO_PARM)
{
  static service_user *startp;
  static lookup_function start_fct;
  service_user *nip;
  lookup_function fct;
  int no_more;
  enum nss_status status = NSS_STATUS_UNAVAIL;
#ifdef USE_NSCD
  int nscd_status;
#endif

#ifdef PREPROCESS
  PREPROCESS;
#endif

#ifdef HANDLE_DIGITS_DOTS
  switch (__nss_hostname_digits_dots (name, resbuf, &buffer, NULL,
				      buflen, result, &status,
				      TYPE_VAR_P, FLAGS_VAR, AF_VAR_P,
				      H_ERRNO_VAR_P))
    {
    case -1:
      return errno;
    case 1:
      goto done;
    }
#endif

#ifdef USE_NSCD
  if (NOT_USENSCD_NAME && ++NOT_USENSCD_NAME > NSS_NSCD_RETRY)
    NOT_USENSCD_NAME = 0;

  if (!NOT_USENSCD_NAME)
    {
      nscd_status = NSCD_NAME (ADD_VARIABLES, resbuf, buffer, buflen
			       H_ERRNO_VAR);
      if (nscd_status >= 0)
	{
	  *result = nscd_status == 0 ? resbuf : NULL;
	  return nscd_status;
	}
    }
#endif

  if (startp == NULL)
    {
      no_more = DB_LOOKUP_FCT (&nip, REENTRANT_NAME_STRING, (void **) &fct);
      if (no_more)
	startp = (service_user *) -1l;
      else
	{
	  startp = nip;
	  start_fct = fct;

#ifdef NEED__RES
	  /* The resolver code will really be used so we have to
	     initialize it.  */
	  if ((_res.options & RES_INIT) == 0 && __res_ninit (&_res) == -1)
	    {
	      *h_errnop = NETDB_INTERNAL;
	      *result = NULL;
	      return errno;
	    }
#endif /* need _res */
#ifdef NEED__RES_HCONF
	  if (!_res_hconf.initialized)
	    _res_hconf_init ();
#endif /* need _res_hconf */
	}
    }
  else
    {
      fct = start_fct;
      no_more = (nip = startp) == (service_user *) -1l;
    }

  while (no_more == 0)
    {
      status = DL_CALL_FCT (fct, (ADD_VARIABLES, resbuf, buffer, buflen,
				   &errno H_ERRNO_VAR));

      /* The status is NSS_STATUS_TRYAGAIN and errno is ERANGE the
	 provided buffer is too small.  In this case we should give
	 the user the possibility to enlarge the buffer and we should
	 not simply go on with the next service (even if the TRYAGAIN
	 action tells us so).  */
      if (status == NSS_STATUS_TRYAGAIN
#ifdef NEED_H_ERRNO
	  && *h_errnop == NETDB_INTERNAL
#endif
	  && errno == ERANGE)
	break;

      no_more = __nss_next (&nip, REENTRANT_NAME_STRING,
			    (void **) &fct, status, 0);
    }

#ifdef HANDLE_DIGITS_DOTS
done:
#endif
  *result = status == NSS_STATUS_SUCCESS ? resbuf : NULL;
#ifdef POSTPROCESS
  POSTPROCESS;
#endif
  return status == NSS_STATUS_SUCCESS ? 0 : errno;
}


#include <shlib-compat.h>
#if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1_2)
#define OLD(name) OLD1 (name)
#define OLD1(name) __old_##name

int
OLD (REENTRANT_NAME) (ADD_PARAMS, LOOKUP_TYPE *resbuf, char *buffer,
		      size_t buflen, LOOKUP_TYPE **result H_ERRNO_PARM)
{
  int ret = INTERNAL (REENTRANT_NAME) (ADD_VARIABLES, resbuf, buffer,
  				       buflen, result H_ERRNO_VAR);

  if (ret != 0)
    ret = -1;

  return ret;
}

#define do_symbol_version(real, name, version) \
  compat_symbol (libc, real, name, version)
do_symbol_version (OLD (REENTRANT_NAME), REENTRANT_NAME, GLIBC_2_0);
#endif

#define do_default_symbol_version(real, name, version) \
  versioned_symbol (libc, real, name, version)
do_default_symbol_version (INTERNAL (REENTRANT_NAME),
			   REENTRANT_NAME, GLIBC_2_1_2);
