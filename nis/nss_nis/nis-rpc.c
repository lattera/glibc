/* Copyright (C) 1996, 1997, 1998, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@suse.de>, 1996.

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

#include <nss.h>
#include <netdb.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <bits/libc-lock.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

#include "nss-nis.h"

/* Get the declaration of the parser function.  */
#define ENTNAME rpcent
#define EXTERN_PARSER
#include <nss/nss_files/files-parse.c>

__libc_lock_define_initialized (static, lock)

struct response_t
{
  char *val;
  struct response_t *next;
};

struct intern_t
{
  struct response_t *start;
  struct response_t *next;
};
typedef struct intern_t intern_t;

static intern_t intern = {NULL, NULL};

static int
saveit (int instatus, char *inkey, int inkeylen, char *inval,
        int invallen, char *indata)
{
  intern_t *intern = (intern_t *)indata;

  if (instatus != YP_TRUE)
    return instatus;

  if (inkey && inkeylen > 0 && inval && invallen > 0)
    {
      if (intern->start == NULL)
        {
          intern->start = malloc (sizeof (struct response_t));
	  if (intern->start == NULL)
	    return YP_FALSE;
          intern->next = intern->start;
        }
      else
        {
          intern->next->next = malloc (sizeof (struct response_t));
	  if (intern->next->next == NULL)
	    return YP_FALSE;
          intern->next = intern->next->next;
        }
      intern->next->next = NULL;
      intern->next->val = malloc (invallen + 1);
      if (intern->next->val == NULL)
	return YP_FALSE;
      strncpy (intern->next->val, inval, invallen);
      intern->next->val[invallen] = '\0';
    }

  return 0;
}

static enum nss_status
internal_nis_setrpcent (intern_t *intern)
{
  char *domainname;
  struct ypall_callback ypcb;
  enum nss_status status;

  if (yp_get_default_domain (&domainname))
    return NSS_STATUS_UNAVAIL;

  while (intern->start != NULL)
    {
      if (intern->start->val != NULL)
        free (intern->start->val);
      intern->next = intern->start;
      intern->start = intern->start->next;
      free (intern->next);
    }
  intern->start = NULL;

  ypcb.foreach = saveit;
  ypcb.data = (char *)intern;
  status = yperr2nss (yp_all(domainname, "rpc.bynumber", &ypcb));
  intern->next = intern->start;

  return status;
}

enum nss_status
_nss_nis_setrpcent (int stayopen)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_setrpcent (&intern);

  __libc_lock_unlock (lock);

  return status;
}

static enum nss_status
internal_nis_endrpcent (intern_t *intern)
{
  while (intern->start != NULL)
    {
      if (intern->start->val != NULL)
        free (intern->start->val);
      intern->next = intern->start;
      intern->start = intern->start->next;
      free (intern->next);
    }
  intern->start = NULL;

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_endrpcent (void)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_endrpcent (&intern);

  __libc_lock_unlock (lock);

  return status;
}

static enum nss_status
internal_nis_getrpcent_r (struct rpcent *rpc, char *buffer, size_t buflen,
			  int *errnop, intern_t *data)
{
  struct parser_data *pdata = (void *) buffer;
  int parse_res;
  char *p;

  if (data->start == NULL)
    internal_nis_setrpcent (data);

  /* Get the next entry until we found a correct one. */
  do
    {
      if (data->next == NULL)
	{
	  *errnop = ENOENT;
	  return NSS_STATUS_NOTFOUND;
	}
      p = strncpy (buffer, data->next->val, buflen);
      while (isspace (*p))
        ++p;

      parse_res = _nss_files_parse_rpcent (p, rpc, pdata, buflen, errnop);
      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;
      data->next = data->next->next;
    }
  while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getrpcent_r (struct rpcent *rpc, char *buffer, size_t buflen,
		      int *errnop)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_getrpcent_r (rpc, buffer, buflen, errnop, &intern);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nis_getrpcbyname_r (const char *name, struct rpcent *rpc,
			 char *buffer, size_t buflen, int *errnop)
{
  intern_t data = {NULL, NULL};
  enum nss_status status;
  int found;

  if (name == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  status = internal_nis_setrpcent (&data);
  if (status != NSS_STATUS_SUCCESS)
    return status;

  found = 0;
  while (!found &&
         ((status = internal_nis_getrpcent_r (rpc, buffer, buflen, errnop,
					      &data)) == NSS_STATUS_SUCCESS))
    {
      if (strcmp (rpc->r_name, name) == 0)
	found = 1;
      else
	{
	  int i = 0;

	  while (rpc->r_aliases[i] != NULL)
	    {
	      if (strcmp (rpc->r_aliases[i], name) == 0)
		{
		  found = 1;
		  break;
		}
	      else
		++i;
	    }
	}
    }

  internal_nis_endrpcent (&data);

  if (!found && status == NSS_STATUS_SUCCESS)
    {
      *errnop = ENOENT;
      return NSS_STATUS_NOTFOUND;
    }
  else
    return status;
}

enum nss_status
_nss_nis_getrpcbynumber_r (int number, struct rpcent *rpc,
			   char *buffer, size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  enum nss_status retval;
  char *domain, *result, *p;
  int len, nlen, parse_res;
  char buf[32];

  if (yp_get_default_domain (&domain))
    return NSS_STATUS_UNAVAIL;

  nlen = sprintf (buf, "%d", number);

  retval = yperr2nss (yp_match (domain, "rpc.bynumber", buf,
				 nlen, &result, &len));

  if (retval != NSS_STATUS_SUCCESS)
    {
      if (retval == NSS_STATUS_NOTFOUND)
	*errnop = ENOENT;
      else if (retval == NSS_STATUS_TRYAGAIN)
	*errnop = errno;
      return retval;
    }

  if ((size_t) (len + 1) > buflen)
    {
      free (result);
      *errnop = ERANGE;
      return NSS_STATUS_TRYAGAIN;
    }

  p = strncpy (buffer, result, len);
  buffer[len] = '\0';
  while (isspace (*p))
    ++p;
  free (result);

  parse_res = _nss_files_parse_rpcent (p, rpc, data, buflen, errnop);

  if (parse_res < 1)
    {
      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;
      else
	{
	  *errnop = ENOENT;
	  return NSS_STATUS_NOTFOUND;
	}
    }
  else
    return NSS_STATUS_SUCCESS;
}
