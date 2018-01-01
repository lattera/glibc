/* Copyright (C) 1997-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1997.

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

#include <atomic.h>
#include <nss.h>
#include <grp.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <libc-lock.h>
#include <rpcsvc/nis.h>

#include "nss-nisplus.h"
#include "nisplus-parser.h"
#include <libnsl.h>
#include <nis_intern.h>
#include <nis_xdr.h>


__libc_lock_define_initialized (static, lock);

/* Connection information.  */
static ib_request *ibreq;
static directory_obj *dir;
static dir_binding bptr;
static char *tablepath;
static char *tableptr;
/* Cursor.  */
static netobj cursor;


nis_name grp_tablename_val attribute_hidden;
size_t grp_tablename_len attribute_hidden;

enum nss_status
_nss_grp_create_tablename (int *errnop)
{
  if (grp_tablename_val == NULL)
    {
      const char *local_dir = nis_local_directory ();
      size_t local_dir_len = strlen (local_dir);
      static const char prefix[] = "group.org_dir.";

      char *p = malloc (sizeof (prefix) + local_dir_len);
      if (p == NULL)
	{
	  *errnop = errno;
	  return NSS_STATUS_TRYAGAIN;
	}

      memcpy (__stpcpy (p, prefix), local_dir, local_dir_len + 1);

      grp_tablename_len = sizeof (prefix) - 1 + local_dir_len;

      atomic_write_barrier ();

      if (atomic_compare_and_exchange_bool_acq (&grp_tablename_val, p, NULL))
	{
	  /* Another thread already installed the value.  */
	  free (p);
	  grp_tablename_len = strlen (grp_tablename_val);
	}
    }

  return NSS_STATUS_SUCCESS;
}


static void
internal_endgrent (void)
{
  __nisbind_destroy (&bptr);
  memset (&bptr, '\0', sizeof (bptr));

  nis_free_directory (dir);
  dir = NULL;

  nis_free_request (ibreq);
  ibreq = NULL;

  xdr_free ((xdrproc_t) xdr_netobj, (char *) &cursor);
  memset (&cursor, '\0', sizeof (cursor));

  free (tablepath);
  tableptr = tablepath = NULL;
}


static enum nss_status
internal_setgrent (int *errnop)
{
  enum nss_status status = NSS_STATUS_SUCCESS;

  if (grp_tablename_val == NULL)
    status = _nss_grp_create_tablename (errnop);

  if (status == NSS_STATUS_SUCCESS)
    {
      ibreq = __create_ib_request (grp_tablename_val, 0);
      if (ibreq == NULL)
	{
	  *errnop = errno;
	  return NSS_STATUS_TRYAGAIN;
	}

      nis_error retcode = __prepare_niscall (grp_tablename_val, &dir, &bptr, 0);
      if (retcode != NIS_SUCCESS)
	{
	  nis_free_request (ibreq);
	  ibreq = NULL;
	  status = niserr2nss (retcode);
	}
    }

  return status;
}


enum nss_status
_nss_nisplus_setgrent (int stayopen)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  internal_endgrent ();

  // XXX We need to be able to set errno.  Pass in new parameter.
  int err;
  status = internal_setgrent (&err);

  __libc_lock_unlock (lock);

  return status;
}


enum nss_status
_nss_nisplus_endgrent (void)
{
  __libc_lock_lock (lock);

  internal_endgrent ();

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}


static enum nss_status
internal_nisplus_getgrent_r (struct group *gr, char *buffer, size_t buflen,
			     int *errnop)
{
  int parse_res = -1;
  enum nss_status retval = NSS_STATUS_SUCCESS;

  /* Get the next entry until we found a correct one. */
  do
    {
      nis_error status;
      nis_result result;
      memset (&result, '\0', sizeof (result));

      if (cursor.n_bytes == NULL)
	{
	  if (ibreq == NULL)
	    {
	      retval = internal_setgrent (errnop);
	      if (retval != NSS_STATUS_SUCCESS)
		return retval;
	    }

	  status = __do_niscall3 (&bptr, NIS_IBFIRST,
				  (xdrproc_t) _xdr_ib_request,
				  (caddr_t) ibreq,
				  (xdrproc_t) _xdr_nis_result,
				  (caddr_t) &result,
				  0, NULL);
	}
      else
	{
	  ibreq->ibr_cookie.n_bytes = cursor.n_bytes;
	  ibreq->ibr_cookie.n_len = cursor.n_len;

	  status = __do_niscall3 (&bptr, NIS_IBNEXT,
				  (xdrproc_t) _xdr_ib_request,
				  (caddr_t) ibreq,
				  (xdrproc_t) _xdr_nis_result,
				  (caddr_t) &result,
				  0, NULL);

	  ibreq->ibr_cookie.n_bytes = NULL;
	  ibreq->ibr_cookie.n_len = 0;
	}

      if (status != NIS_SUCCESS)
	return niserr2nss (status);

      if (NIS_RES_STATUS (&result) == NIS_NOTFOUND)
	{
	  /* No more entries on this server.  This means we have to go
	     to the next server on the path.  */
	  status = __follow_path (&tablepath, &tableptr, ibreq, &bptr);
	  if (status != NIS_SUCCESS)
	    return niserr2nss (status);

	  directory_obj *newdir = NULL;
	  dir_binding newbptr;
	  status = __prepare_niscall (ibreq->ibr_name, &newdir, &newbptr, 0);
	  if (status != NIS_SUCCESS)
	    return niserr2nss (status);

	  nis_free_directory (dir);
	  dir = newdir;
	  __nisbind_destroy (&bptr);
	  bptr = newbptr;

	  xdr_free ((xdrproc_t) xdr_netobj, (char *) &result.cookie);
	  result.cookie.n_bytes = NULL;
	  result.cookie.n_len = 0;
	  parse_res = 0;
	  goto next;
	}
      else if (NIS_RES_STATUS (&result) != NIS_SUCCESS)
	return niserr2nss (NIS_RES_STATUS (&result));

      parse_res = _nss_nisplus_parse_grent (&result, gr,
					    buffer, buflen, errnop);
      if (__glibc_unlikely (parse_res == -1))
	{
	  *errnop = ERANGE;
	  retval = NSS_STATUS_TRYAGAIN;
	  goto freeres;
	}

    next:
      /* Free the old cursor.  */
      xdr_free ((xdrproc_t) xdr_netobj, (char *) &cursor);
      /* Remember the new one.  */
      cursor.n_bytes = result.cookie.n_bytes;
      cursor.n_len = result.cookie.n_len;
      /* Free the result structure.  NB: we do not remove the cookie.  */
      result.cookie.n_bytes = NULL;
      result.cookie.n_len = 0;
    freeres:
      xdr_free ((xdrproc_t) _xdr_nis_result, (char *) &result);
      memset (&result, '\0', sizeof (result));
    }
  while (!parse_res);

  return retval;
}

enum nss_status
_nss_nisplus_getgrent_r (struct group *result, char *buffer, size_t buflen,
			 int *errnop)
{
  int status;

  __libc_lock_lock (lock);

  status = internal_nisplus_getgrent_r (result, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nisplus_getgrnam_r (const char *name, struct group *gr,
			 char *buffer, size_t buflen, int *errnop)
{
  int parse_res;

  if (grp_tablename_val == NULL)
    {
      enum nss_status status = _nss_grp_create_tablename (errnop);

      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  if (name == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_NOTFOUND;
    }

  nis_result *result;
  char buf[strlen (name) + 9 + grp_tablename_len];
  int olderr = errno;

  snprintf (buf, sizeof (buf), "[name=%s],%s", name, grp_tablename_val);

  result = nis_list (buf, FOLLOW_LINKS | FOLLOW_PATH, NULL, NULL);

  if (result == NULL)
    {
      *errnop = ENOMEM;
      return NSS_STATUS_TRYAGAIN;
    }

  if (__glibc_unlikely (niserr2nss (result->status) != NSS_STATUS_SUCCESS))
    {
      enum nss_status status = niserr2nss (result->status);

      nis_freeresult (result);
      return status;
    }

  parse_res = _nss_nisplus_parse_grent (result, gr, buffer, buflen, errnop);
  nis_freeresult (result);
  if (__glibc_unlikely (parse_res < 1))
    {
      if (parse_res == -1)
	{
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      else
	{
	  __set_errno (olderr);
	  return NSS_STATUS_NOTFOUND;
	}
    }

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_getgrgid_r (const gid_t gid, struct group *gr,
			 char *buffer, size_t buflen, int *errnop)
{
  if (grp_tablename_val == NULL)
    {
      enum nss_status status = _nss_grp_create_tablename (errnop);

      if (status != NSS_STATUS_SUCCESS)
	return status;
    }

  int parse_res;
  nis_result *result;
  char buf[8 + 3 * sizeof (unsigned long int) + grp_tablename_len];
  int olderr = errno;

  snprintf (buf, sizeof (buf), "[gid=%lu],%s",
	    (unsigned long int) gid, grp_tablename_val);

  result = nis_list (buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);

  if (result == NULL)
    {
      *errnop = ENOMEM;
      return NSS_STATUS_TRYAGAIN;
    }

  if (__glibc_unlikely (niserr2nss (result->status) != NSS_STATUS_SUCCESS))
    {
      enum nss_status status = niserr2nss (result->status);

      __set_errno (olderr);

      nis_freeresult (result);
      return status;
    }

  parse_res = _nss_nisplus_parse_grent (result, gr, buffer, buflen, errnop);

  nis_freeresult (result);
  if (__glibc_unlikely (parse_res < 1))
    {
      __set_errno (olderr);

      if (parse_res == -1)
	{
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      else
	return NSS_STATUS_NOTFOUND;
    }

  return NSS_STATUS_SUCCESS;
}
