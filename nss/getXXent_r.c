/* Copyright (C) 1996, 1997, 1998 Free Software Foundation, Inc.
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

#include <errno.h>
#include <bits/libc-lock.h>

#include "nsswitch.h"

/*******************************************************************\
|* Here we assume several symbols to be defined:		   *|
|* 								   *|
|* LOOKUP_TYPE   - the return type of the function		   *|
|* 								   *|
|* SETFUNC_NAME  - name of the non-reentrant setXXXent function	   *|
|* 								   *|
|* GETFUNC_NAME  - name of the non-reentrant getXXXent function	   *|
|* 								   *|
|* ENDFUNC_NAME  - name of the non-reentrant endXXXent function	   *|
|* 								   *|
|* DATABASE_NAME - name of the database the function accesses	   *|
|*		   (e.g., host, services, ...)			   *|
|* 								   *|
|* Optionally the following vars can be defined:		   *|
|* 								   *|
|* STAYOPEN      - variable declaration for setXXXent function	   *|
|* 								   *|
|* STAYOPEN_VAR  - variable name for setXXXent function		   *|
|* 								   *|
|* NEED_H_ERRNO  - an extra parameter will be passed to point to   *|
|*		   the global `h_errno' variable.		   *|
|* 								   *|
\*******************************************************************/

/* To make the real sources a bit prettier.  */
#define REENTRANT_GETNAME APPEND_R (GETFUNC_NAME)
#define APPEND_R(Name) CONCAT2_2 (Name, _r)
#define INTERNAL(Name) CONCAT2_2 (__, Name)
#define CONCAT2_1(Pre, Post) CONCAT2_2 (Pre, Post)
#define CONCAT2_2(Pre, Post) Pre##Post

#define SETFUNC_NAME_STRING STRINGIZE (SETFUNC_NAME)
#define GETFUNC_NAME_STRING STRINGIZE (REENTRANT_GETNAME)
#define ENDFUNC_NAME_STRING STRINGIZE (ENDFUNC_NAME)
#define DATABASE_NAME_STRING STRINGIZE (DATABASE_NAME)
#define STRINGIZE(Name) STRINGIZE1 (Name)
#define STRINGIZE1(Name) #Name

#define DB_LOOKUP_FCT CONCAT3_1 (__nss_, DATABASE_NAME, _lookup)
#define CONCAT3_1(Pre, Name, Post) CONCAT3_2 (Pre, Name, Post)
#define CONCAT3_2(Pre, Name, Post) Pre##Name##Post

/* Sometimes we need to store error codes in the `h_errno' variable.  */
#ifdef NEED_H_ERRNO
# define H_ERRNO_PARM , int *h_errnop
# define H_ERRNO_VAR , &h_errno
#else
# define H_ERRNO_PARM
# define H_ERRNO_VAR
#endif

/* Some databases take the `stayopen' flag.  */
#ifdef STAYOPEN
# define STAYOPEN_TMP CONCAT2_1 (STAYOPEN, _tmp)
# define STAYOPEN_TMPVAR CONCAT2_1 (STAYOPEN_VAR, _tmp)
#else
# define STAYOPEN void
# define STAYOPEN_VAR
# define STAYOPEN_TMPVAR
#endif

/* Prototype for the setXXXent functions we use here.  */
typedef int (*set_function) (STAYOPEN);

/* Prototype for the endXXXent functions we use here.  */
typedef int (*end_function) (void);

/* Prototype for the setXXXent functions we use here.  */
typedef int (*get_function) (LOOKUP_TYPE *, char *, size_t, int *
			     H_ERRNO_PARM);


/* This handle for the NSS data base is shared between all
   set/get/endXXXent functions.  */
static service_user *nip;
/* Remember the last service used since the last call to  `endXXent'.  */
static service_user *last_nip;
/* Remember the first service_entry, it's always the same.  */
static service_user *startp;

#ifdef STAYOPEN_TMP
/* We need to remember the last `stayopen' flag given by the user
   since the `setent' function is only called for the first available
   service.  */
static STAYOPEN_TMP;
#endif

/* Protect above variable against multiple uses at the same time.  */
__libc_lock_define_initialized (static, lock)

/* The lookup function for the first entry of this service.  */
extern int DB_LOOKUP_FCT (service_user **nip, const char *name, void **fctp);

/* Set up NIP to run through the services.  If ALL is zero, use NIP's
   current location if it's not nil.  Return nonzero if there are no
   services (left).  */
static enum nss_status
setup (void **fctp, const char *func_name, int all)
{
  int no_more;
  if (startp == NULL)
    {
      no_more = DB_LOOKUP_FCT (&nip, func_name, fctp);
      startp = no_more ? (service_user *) -1l : nip;
    }
  else if (startp == (service_user *) -1l)
    /* No services at all.  */
    return 1;
  else
    {
      if (all || !nip)
	/* Reset to the beginning of the service list.  */
	nip = startp;
      /* Look up the first function.  */
      no_more = __nss_lookup (&nip, func_name, fctp);
    }
  return no_more;
}

void
SETFUNC_NAME (STAYOPEN)
{
  set_function fct;
  int no_more;

#ifdef NEED__RES
  if ((_res.options & RES_INIT) == 0 && res_init () == -1)
    {
      __set_h_errno (NETDB_INTERNAL);
      return;
    }
#endif /* need _res */

  __libc_lock_lock (lock);

  /* Cycle through the services and run their `setXXent' functions until
     we find an available service.  */
  no_more = setup ((void **) &fct, SETFUNC_NAME_STRING, 1);
  while (! no_more)
    {
      int is_last_nip = nip == last_nip;
      enum nss_status status = (*fct) (STAYOPEN_VAR);

      no_more = __nss_next (&nip, SETFUNC_NAME_STRING, (void **) &fct,
			    status, 0);
      if (is_last_nip)
	last_nip = nip;
    }

#ifdef STAYOPEN_TMP
  STAYOPEN_TMPVAR = STAYOPEN_VAR;
#endif

  __libc_lock_unlock (lock);
}


void
ENDFUNC_NAME (void)
{
  end_function fct;
  int no_more;

#ifdef NEED__RES
  if ((_res.options & RES_INIT) == 0 && res_init () == -1)
    {
      __set_h_errno (NETDB_INTERNAL);
      return;
    }
#endif /* need _res */

  __libc_lock_lock (lock);

  /* Cycle through all the services and run their endXXent functions.  */
  no_more = setup ((void **) &fct, ENDFUNC_NAME_STRING, 1);
  while (! no_more)
    {
      /* Ignore status, we force check in __NSS_NEXT.  */
      (void) (*fct) ();

      if (nip == last_nip)
	/* We have processed all services which were used.  */
	break;

      no_more = __nss_next (&nip, ENDFUNC_NAME_STRING, (void **) &fct, 0, 1);
    }
  last_nip = nip = NULL;

  __libc_lock_unlock (lock);
}


int
INTERNAL (REENTRANT_GETNAME) (LOOKUP_TYPE *resbuf, char *buffer, size_t buflen,
			      LOOKUP_TYPE **result H_ERRNO_PARM)
{
  get_function fct;
  int no_more;
  enum nss_status status;

#ifdef NEED__RES
  if ((_res.options & RES_INIT) == 0 && res_init () == -1)
    {
      __set_h_errno (NETDB_INTERNAL);
      *result = NULL;
      return -1;
    }
#endif /* need _res */

  /* Initialize status to return if no more functions are found.  */
  status = NSS_STATUS_NOTFOUND;

  __libc_lock_lock (lock);

  /* Run through available functions, starting with the same function last
     run.  We will repeat each function as long as it succeeds, and then go
     on to the next service action.  */
  no_more = setup ((void **) &fct, GETFUNC_NAME_STRING, 0);
  while (! no_more)
    {
      int is_last_nip = nip == last_nip;

      status = (*fct) (resbuf, buffer, buflen, &errno H_ERRNO_VAR);

      /* The the status is NSS_STATUS_TRYAGAIN and errno is ERANGE the
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

      do
	{
	  no_more = __nss_next (&nip, GETFUNC_NAME_STRING, (void **) &fct,
				status, 0);

	  if (is_last_nip)
	    last_nip = nip;

	  if (! no_more)
	    {
	      /* Call the `setXXent' function.  This wasn't done before.  */
	      set_function sfct;

	      no_more = __nss_lookup (&nip, SETFUNC_NAME_STRING,
				      (void **) &sfct);

	      if (! no_more)
		status = (*sfct) (STAYOPEN_TMPVAR);
	      else
		status = NSS_STATUS_NOTFOUND;
	    }
	}
      while (! no_more && status != NSS_STATUS_SUCCESS);
    }

  __libc_lock_unlock (lock);

  *result = status == NSS_STATUS_SUCCESS ? resbuf : NULL;
  return status == NSS_STATUS_SUCCESS ? 0 : -1;
}
#define do_weak_alias(n1, n2) weak_alias (n1, n2)
do_weak_alias (INTERNAL (REENTRANT_GETNAME), REENTRANT_GETNAME)
