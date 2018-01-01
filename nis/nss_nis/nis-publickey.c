/* Copyright (C) 1996-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@suse.de>, 1996.

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

#include <nss.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <syslog.h>
#include <rpc/rpc.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>
#include <rpc/key_prot.h>
#include <rpc/des_crypt.h>

#include "nss-nis.h"

/* If we haven't found the entry, we give a SUCCESS and an empty key back.
   Solaris docu says: sizeof (pkey) == HEXKEYBYTES + 1.
*/
enum nss_status
_nss_nis_getpublickey (const char *netname, char *pkey, int *errnop)
{
  pkey[0] = 0;

  if (netname == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  char *domain = strchr (netname, '@');
  if (domain == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }
  ++domain;

  char *result;
  int len;
  int yperr = yp_match (domain, "publickey.byname", netname, strlen (netname),
			&result, &len);

  if (__glibc_unlikely (yperr != YPERR_SUCCESS))
    {
      enum nss_status retval = yperr2nss (yperr);

      if (retval == NSS_STATUS_TRYAGAIN)
	*errnop = errno;
      return retval;
    }

  if (result != NULL)
    {
      char *p = strchr (result, ':');
      if (p != NULL)
	*p = 0;
      strncpy (pkey, result, HEXKEYBYTES + 1);
      pkey[HEXKEYBYTES] = '\0';
      free (result);
    }
  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getsecretkey (const char *netname, char *skey, char *passwd,
		       int *errnop)
{
  skey[0] = 0;

  if (netname == NULL || passwd == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  char *domain = strchr (netname, '@');
  if (domain == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }
  ++domain;

  char *result;
  int len;
  int yperr = yp_match (domain, "publickey.byname", netname, strlen (netname),
			&result, &len);

  if (__glibc_unlikely (yperr != YPERR_SUCCESS))
    {
      enum nss_status retval = yperr2nss (yperr);

      if (retval == NSS_STATUS_TRYAGAIN)
	*errnop = errno;
      return retval;
    }

  if (result != NULL)
    {
      char *p = strchr (result, ':');
      if (p != NULL)
	{
	  char buf[2 * (HEXKEYBYTES + 1)];

	  ++p;
	  strncpy (buf, p, 2 * (HEXKEYBYTES + 1));
	  buf[2 * HEXKEYBYTES + 1] = '\0';
	  if (xdecrypt (buf, passwd)
	      && memcmp (buf, &(buf[HEXKEYBYTES]), KEYCHECKSUMSIZE) == 0)
	    {
	      buf[HEXKEYBYTES] = '\0';
	      strcpy (skey, buf);
	    }
	}

      free (result);
    }
  return NSS_STATUS_SUCCESS;
}

/* Parse uid and group information from the passed string.
   The format of the string passed is uid:gid,grp,grp, ...  */
static enum nss_status
parse_netid_str (const char *s, uid_t *uidp, gid_t *gidp, int *gidlenp,
		 gid_t *gidlist)
{
  char *p, *ep;
  int gidlen;

  if (!s || !isdigit (*s))
    {
      syslog (LOG_ERR, "netname2user: expecting uid '%s'", s);
      return NSS_STATUS_NOTFOUND;	/* XXX need a better error */
    }

  /* Fetch the uid */
  *uidp = strtoul (s, NULL, 10);

  if (*uidp == 0)
    {
      syslog (LOG_ERR, "netname2user: should not have uid 0");
      return NSS_STATUS_NOTFOUND;
    }

  /* Now get the group list */
  p = strchr (s, ':');
  if (!p)
    {
      syslog (LOG_ERR, "netname2user: missing group id list in '%s'", s);
      return NSS_STATUS_NOTFOUND;
    }
  ++p;				/* skip ':' */
  if (!p || (!isdigit (*p)))
    {
      syslog (LOG_ERR, "netname2user: missing group id list in '%s'.", p);
      return NSS_STATUS_NOTFOUND;
    }

  *gidp = strtoul (p, &ep, 10);

  gidlen = 0;

  /* After strtoul() ep should point to the first invalid character.
     This is the marker "," we search for the next value.  */
  while (ep != NULL && *ep == ',')
    {
      ep++;
      p = ep;
      gidlist[gidlen++] = strtoul (p, &ep, 10);
    }

  *gidlenp = gidlen;

  return NSS_STATUS_SUCCESS;
}


enum nss_status
_nss_nis_netname2user (char netname[MAXNETNAMELEN + 1], uid_t *uidp,
		       gid_t *gidp, int *gidlenp, gid_t *gidlist, int *errnop)
{
  char *domain = strchr (netname, '@');
  if (domain == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  /* Point past the '@' character */
  ++domain;
  char *lookup = NULL;
  int len;
  int yperr = yp_match (domain, "netid.byname", netname, strlen (netname),
			&lookup, &len);
  switch (yperr)
    {
    case YPERR_SUCCESS:
      break;			/* the successful case */
    case YPERR_DOMAIN:
    case YPERR_KEY:
      return NSS_STATUS_NOTFOUND;
    case YPERR_MAP:
    default:
      return NSS_STATUS_UNAVAIL;
    }

  if (lookup == NULL)
    return NSS_STATUS_NOTFOUND;


  lookup[len] = '\0';

  enum nss_status err = parse_netid_str (lookup, uidp, gidp, gidlenp, gidlist);

  free (lookup);

  return err;
}
