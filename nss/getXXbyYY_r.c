/* Copyright (C) 1996-2004,2006,2007,2009,2010 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <assert.h>
#include <atomic.h>
#include <errno.h>
#include <stdbool.h>
#include "nsswitch.h"
#include "sysdep.h"
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
|*								   *|
|* LOOKUP_TYPE   - the return type of the function		   *|
|*								   *|
|* FUNCTION_NAME - name of the non-reentrant function		   *|
|*								   *|
|* DATABASE_NAME - name of the database the function accesses	   *|
|*		   (e.g., host, services, ...)			   *|
|*								   *|
|* ADD_PARAMS    - additional parameters, can vary in number	   *|
|*								   *|
|* ADD_VARIABLES - names of additional parameters		   *|
|*								   *|
|* Optionally the following vars can be defined:		   *|
|*								   *|
|* EXTRA_PARAMS  - optional parameters, can vary in number	   *|
|*								   *|
|* EXTRA_VARIABLES - names of optional parameter		   *|
|*								   *|
|* FUNCTION_NAME - alternative name of the non-reentrant function  *|
|*								   *|
|* NEED_H_ERRNO  - an extra parameter will be passed to point to   *|
|*		   the global `h_errno' variable.		   *|
|*								   *|
|* NEED__RES     - the global _res variable might be used so we	   *|
|*		   will have to initialize it if necessary	   *|
|*								   *|
|* PREPROCESS    - code run before anything else		   *|
|*								   *|
|* POSTPROCESS   - code run after the lookup			   *|
|*								   *|
\*******************************************************************/

/* To make the real sources a bit prettier.  */
#define REENTRANT_NAME APPEND_R (FUNCTION_NAME)
#ifdef FUNCTION2_NAME
# define REENTRANT2_NAME APPEND_R (FUNCTION2_NAME)
#else
# define REENTRANT2_NAME NULL
#endif
#define APPEND_R(name) APPEND_R1 (name)
#define APPEND_R1(name) name##_r
#define INTERNAL(name) INTERNAL1 (name)
#define INTERNAL1(name) __##name
#define NEW(name) NEW1 (name)
#define NEW1(name) __new_##name

#ifdef USE_NSCD
# define NSCD_NAME ADD_NSCD (REENTRANT_NAME)
# define ADD_NSCD(name) ADD_NSCD1 (name)
# define ADD_NSCD1(name) __nscd_##name
# define NOT_USENSCD_NAME ADD_NOT_NSCDUSE (DATABASE_NAME)
# define ADD_NOT_NSCDUSE(name) ADD_NOT_NSCDUSE1 (name)
# define ADD_NOT_NSCDUSE1(name) __nss_not_use_nscd_##name
# define CONCAT2(arg1, arg2) CONCAT2_2 (arg1, arg2)
# define CONCAT2_2(arg1, arg2) arg1##arg2
#endif

#define FUNCTION_NAME_STRING STRINGIZE (FUNCTION_NAME)
#define REENTRANT_NAME_STRING STRINGIZE (REENTRANT_NAME)
#ifdef FUNCTION2_NAME
# define REENTRANT2_NAME_STRING STRINGIZE (REENTRANT2_NAME)
#else
# define REENTRANT2_NAME_STRING NULL
#endif
#define DATABASE_NAME_STRING STRINGIZE (DATABASE_NAME)
#define STRINGIZE(name) STRINGIZE1 (name)
#define STRINGIZE1(name) #name

#ifndef DB_LOOKUP_FCT
# define DB_LOOKUP_FCT CONCAT3_1 (__nss_, DATABASE_NAME, _lookup2)
# define CONCAT3_1(Pre, Name, Post) CONCAT3_2 (Pre, Name, Post)
# define CONCAT3_2(Pre, Name, Post) Pre##Name##Post
#endif

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

#ifndef EXTRA_PARAMS
# define EXTRA_PARAMS
#endif
#ifndef EXTRA_VARIABLES
# define EXTRA_VARIABLES
#endif

#ifdef HAVE_AF
# define AF_VAL af
#else
# define AF_VAL AF_INET
#endif

/* Type of the lookup function we need here.  */
typedef enum nss_status (*lookup_function) (ADD_PARAMS, LOOKUP_TYPE *, char *,
					    size_t, int * H_ERRNO_PARM
					    EXTRA_PARAMS);

/* The lookup function for the first entry of this service.  */
extern int DB_LOOKUP_FCT (service_user **nip, const char *name,
			  const char *name2, void **fctp)
     internal_function;
libc_hidden_proto (DB_LOOKUP_FCT)


int
INTERNAL (REENTRANT_NAME) (ADD_PARAMS, LOOKUP_TYPE *resbuf, char *buffer,
			   size_t buflen, LOOKUP_TYPE **result H_ERRNO_PARM
			   EXTRA_PARAMS)
{
  static bool startp_initialized;
  static service_user *startp;
  static lookup_function start_fct;
  service_user *nip;
  union
  {
    lookup_function l;
    void *ptr;
  } fct;

  int no_more;
  enum nss_status status = NSS_STATUS_UNAVAIL;
#ifdef USE_NSCD
  int nscd_status;
#endif
#ifdef NEED_H_ERRNO
  bool any_service = false;
#endif

#ifdef PREPROCESS
  PREPROCESS;
#endif

#ifdef HANDLE_DIGITS_DOTS
  switch (__nss_hostname_digits_dots (name, resbuf, &buffer, NULL,
				      buflen, result, &status, AF_VAL,
				      H_ERRNO_VAR_P))
    {
    case -1:
      return errno;
    case 1:
      goto done;
    }
#endif

#ifdef USE_NSCD
  if (NOT_USENSCD_NAME > 0 && ++NOT_USENSCD_NAME > NSS_NSCD_RETRY)
    NOT_USENSCD_NAME = 0;

  if (!NOT_USENSCD_NAME
      && !__nss_database_custom[CONCAT2 (NSS_DBSIDX_, DATABASE_NAME)])
    {
      nscd_status = NSCD_NAME (ADD_VARIABLES, resbuf, buffer, buflen, result
			       H_ERRNO_VAR);
      if (nscd_status >= 0)
	return nscd_status;
    }
#endif

  if (! startp_initialized)
    {
      no_more = DB_LOOKUP_FCT (&nip, REENTRANT_NAME_STRING,
			       REENTRANT2_NAME_STRING, &fct.ptr);
      if (no_more)
	{
	  void *tmp_ptr = (service_user *) -1l;
	  PTR_MANGLE (tmp_ptr);
	  startp = tmp_ptr;
	}
      else
	{
#ifdef NEED__RES
	  /* The resolver code will really be used so we have to
	     initialize it.  */
	  if (__res_maybe_init (&_res, 0) == -1)
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

	  void *tmp_ptr = fct.l;
	  PTR_MANGLE (tmp_ptr);
	  start_fct = tmp_ptr;
	  tmp_ptr = nip;
	  PTR_MANGLE (tmp_ptr);
	  startp = tmp_ptr;
	}

      /* Make sure start_fct and startp are written before
	 startp_initialized.  */
      atomic_write_barrier ();
      startp_initialized = true;
    }
  else
    {
      fct.l = start_fct;
      PTR_DEMANGLE (fct.l);
      nip = startp;
      PTR_DEMANGLE (nip);
      no_more = nip == (service_user *) -1l;
    }

  while (no_more == 0)
    {
#ifdef NEED_H_ERRNO
      any_service = true;
#endif

      status = DL_CALL_FCT (fct.l, (ADD_VARIABLES, resbuf, buffer, buflen,
				    &errno H_ERRNO_VAR EXTRA_VARIABLES));

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

      no_more = __nss_next2 (&nip, REENTRANT_NAME_STRING,
			     REENTRANT2_NAME_STRING, &fct.ptr, status, 0);
    }

#ifdef HANDLE_DIGITS_DOTS
done:
#endif
  *result = status == NSS_STATUS_SUCCESS ? resbuf : NULL;
#ifdef NEED_H_ERRNO
  if (status != NSS_STATUS_SUCCESS && ! any_service)
    /* We were not able to use any service.  */
    *h_errnop = NO_RECOVERY;
#endif
#ifdef POSTPROCESS
  POSTPROCESS;
#endif

  int res;
  if (status == NSS_STATUS_SUCCESS || status == NSS_STATUS_NOTFOUND)
    res = 0;
  /* Don't pass back ERANGE if this is not for a too-small buffer.  */
  else if (errno == ERANGE && status != NSS_STATUS_TRYAGAIN)
    res = EINVAL;
#ifdef NEED_H_ERRNO
  /* These functions only set errno if h_errno is NETDB_INTERNAL.  */
  else if (status == NSS_STATUS_TRYAGAIN && *h_errnop != NETDB_INTERNAL)
    res = EAGAIN;
#endif
  else
    return errno;

  __set_errno (res);
  return res;
}


#ifdef NO_COMPAT_NEEDED
strong_alias (INTERNAL (REENTRANT_NAME), REENTRANT_NAME);
#elif !defined FUNCTION2_NAME
# include <shlib-compat.h>
# if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_1_2)
#  define OLD(name) OLD1 (name)
#  define OLD1(name) __old_##name

int
attribute_compat_text_section
OLD (REENTRANT_NAME) (ADD_PARAMS, LOOKUP_TYPE *resbuf, char *buffer,
		      size_t buflen, LOOKUP_TYPE **result H_ERRNO_PARM)
{
  int ret = INTERNAL (REENTRANT_NAME) (ADD_VARIABLES, resbuf, buffer,
				       buflen, result H_ERRNO_VAR);

  if (ret != 0 || result == NULL)
    ret = -1;

  return ret;
}

#  define do_symbol_version(real, name, version) \
  compat_symbol (libc, real, name, version)
do_symbol_version (OLD (REENTRANT_NAME), REENTRANT_NAME, GLIBC_2_0);
# endif

/* As INTERNAL (REENTRANT_NAME) may be hidden, we need an alias
   in between so that the REENTRANT_NAME@@GLIBC_2.1.2 is not
   hidden too.  */
strong_alias (INTERNAL (REENTRANT_NAME), NEW (REENTRANT_NAME));

# define do_default_symbol_version(real, name, version) \
  versioned_symbol (libc, real, name, version)
do_default_symbol_version (NEW (REENTRANT_NAME),
			   REENTRANT_NAME, GLIBC_2_1_2);
#endif

static_link_warning (REENTRANT_NAME)
