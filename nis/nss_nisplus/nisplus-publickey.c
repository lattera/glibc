/* Copyright (c) 1997, 1999, 2001, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@suse.de>, 1997.

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

#include <nss.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include <syslog.h>
#include <rpc/rpc.h>
#include <rpcsvc/nis.h>
#include <rpc/key_prot.h>
extern int xdecrypt (char *, char *);

#include <nss-nisplus.h>

/* If we haven't found the entry, we give a SUCCESS and an empty key back. */
enum nss_status
_nss_nisplus_getpublickey (const char *netname, char *pkey, int *errnop)
{
  nis_result *res;
  enum nss_status retval;
  char buf[NIS_MAXNAMELEN+2];
  size_t slen;
  char *domain, *cptr;
  int len;

  pkey[0] = 0;

  if (netname == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  domain = strchr (netname, '@');
  if (!domain)
    return NSS_STATUS_UNAVAIL;
  domain++;

  slen = snprintf (buf, NIS_MAXNAMELEN,
		   "[auth_name=%s,auth_type=DES],cred.org_dir.%s",
		   netname, domain);

  if (slen >= NIS_MAXNAMELEN)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  if (buf[slen - 1] != '.')
    {
      buf[slen++] = '.';
      buf[slen] = '\0';
    }

  res = nis_list (buf, USE_DGRAM+NO_AUTHINFO+FOLLOW_LINKS+FOLLOW_PATH,
		  NULL, NULL);

  if (res == NULL)
    {
      *errnop = ENOMEM;
      return NSS_STATUS_TRYAGAIN;
    }
  retval = niserr2nss (res->status);

  if (retval != NSS_STATUS_SUCCESS)
    {
      if (retval == NSS_STATUS_TRYAGAIN)
	*errnop = errno;
      if (res->status == NIS_NOTFOUND)
	retval = NSS_STATUS_SUCCESS;
      nis_freeresult (res);
      return retval;
    }

  if (res->objects.objects_len > 1)
    {
      /*
       * More than one principal with same uid?
       * something wrong with cred table. Should be unique
       * Warn user and continue.
       */
      printf (_("DES entry for netname %s not unique\n"), netname);
      nis_freeresult (res);
      return NSS_STATUS_SUCCESS;
    }

  len = ENTRY_LEN (res->objects.objects_val, 3);
  memcpy (pkey, ENTRY_VAL (res->objects.objects_val,3), len);
  pkey[len] = 0;
  cptr = strchr (pkey, ':');
  if (cptr)
    cptr[0] = '\0';
  nis_freeresult (res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_getsecretkey (const char *netname, char *skey, char *passwd,
			   int *errnop)
{
  nis_result *res;
  enum nss_status retval;
  char buf[NIS_MAXNAMELEN+2];
  size_t slen;
  char *domain, *cptr;
  int len;

  skey[0] = 0;

  if (netname == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  domain = strchr (netname, '@');
  if (!domain)
    return NSS_STATUS_UNAVAIL;
  domain++;

  slen = snprintf (buf, NIS_MAXNAMELEN,
		   "[auth_name=%s,auth_type=DES],cred.org_dir.%s",
		   netname, domain);

  if (slen >= NIS_MAXNAMELEN)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  if (buf[slen - 1] != '.')
    {
      buf[slen++] = '.';
      buf[slen] = '\0';
    }

  res = nis_list (buf, USE_DGRAM+NO_AUTHINFO+FOLLOW_LINKS+FOLLOW_PATH,
		  NULL, NULL);

  if (res == NULL)
    {
      *errnop = ENOMEM;
      return NSS_STATUS_TRYAGAIN;
    }
  retval = niserr2nss (res->status);

  if (retval != NSS_STATUS_SUCCESS)
    {
      if (retval == NSS_STATUS_TRYAGAIN)
	*errnop = errno;
      nis_freeresult (res);
      return retval;
    }

  if (res->objects.objects_len > 1)
    {
      /*
       * More than one principal with same uid?
       * something wrong with cred table. Should be unique
       * Warn user and continue.
       */
      printf (_("DES entry for netname %s not unique\n"), netname);
      nis_freeresult (res);
      return NSS_STATUS_SUCCESS;
    }

  len = ENTRY_LEN (res->objects.objects_val, 4);
  memcpy (buf, ENTRY_VAL (res->objects.objects_val,4), len);
  buf[len] = '\0';
  cptr = strchr (buf, ':');
  if (cptr)
    cptr[0] = '\0';
  nis_freeresult (res);

  if (!xdecrypt (buf, passwd))
    return NSS_STATUS_SUCCESS;

  if (memcmp (buf, &(buf[HEXKEYBYTES]), KEYCHECKSUMSIZE) != 0)
    return NSS_STATUS_SUCCESS;

  buf[HEXKEYBYTES] = 0;
  strcpy (skey, buf);

  return NSS_STATUS_SUCCESS;
}

/* Parse information from the passed string.
   The format of the string passed is gid,grp,grp, ...  */
static enum nss_status
parse_grp_str (const char *s, gid_t *gidp, int *gidlenp, gid_t *gidlist,
	       int *errnop)
{
  char *ep;
  int gidlen;

  if (!s || (!isdigit (*s)))
    {
      syslog (LOG_ERR, _("netname2user: missing group id list in `%s'."), s);
      return NSS_STATUS_NOTFOUND;
    }

  *gidp = strtoul (s, &ep, 10);

  gidlen = 0;

  /* After strtoul() ep should point to the marker ',', which means
     here starts a new value. */
  while (ep != NULL && *ep == ',')
    {
      ep++;
      s = ep;
      gidlist[gidlen++] = strtoul (s, &ep, 10);
    }
  *gidlenp = gidlen;

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nisplus_netname2user (char netname[MAXNETNAMELEN + 1], uid_t *uidp,
		       gid_t *gidp, int *gidlenp, gid_t *gidlist, int *errnop)
{
  char *domain;
  nis_result *res;
  char sname[NIS_MAXNAMELEN+2]; /*  search criteria + table name */
  size_t slen;
  char principal[NIS_MAXNAMELEN+1];
  int len;

  /* 1.  Get home domain of user. */
  domain = strchr (netname, '@');
  if (! domain)
    return NSS_STATUS_UNAVAIL;

  ++domain;  /* skip '@' */

  /* 2.  Get user's nisplus principal name.  */
  if ((strlen (netname) + strlen (domain)+45) >
      (size_t) NIS_MAXNAMELEN)
    return NSS_STATUS_UNAVAIL;

  slen = snprintf (sname, NIS_MAXNAMELEN,
		   "[auth_name=%s,auth_type=DES],cred.org_dir.%s",
		   netname, domain);

  if (slen >= NIS_MAXNAMELEN)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  if (sname[slen - 1] != '.')
    {
      sname[slen++] = '.';
      sname[slen] = '\0';
    }

  /* must use authenticated call here */
  /* XXX but we cant, for now. XXX */
  res = nis_list (sname, USE_DGRAM+NO_AUTHINFO+FOLLOW_LINKS+FOLLOW_PATH,
		  NULL, NULL);
  if (res == NULL)
    {
      *errnop = ENOMEM;
      return NSS_STATUS_TRYAGAIN;
    }
  switch (res->status)
    {
    case NIS_SUCCESS:
    case NIS_S_SUCCESS:
      break;   /* go and do something useful */
    case NIS_NOTFOUND:
    case NIS_PARTIAL:
    case NIS_NOSUCHNAME:
    case NIS_NOSUCHTABLE:
      nis_freeresult (res);
      return NSS_STATUS_NOTFOUND;
    case NIS_S_NOTFOUND:
    case NIS_TRYAGAIN:
      syslog (LOG_ERR, _("netname2user: (nis+ lookup): %s\n"),
	      nis_sperrno (res->status));
      nis_freeresult (res);
      *errnop = errno;
      return NSS_STATUS_TRYAGAIN;
    default:
      syslog (LOG_ERR, _("netname2user: (nis+ lookup): %s\n"),
	      nis_sperrno (res->status));
      nis_freeresult (res);
      return NSS_STATUS_UNAVAIL;
    }

  if (res->objects.objects_len > 1)
    /*
     * A netname belonging to more than one principal?
     * Something wrong with cred table. should be unique.
     * Warn user and continue.
     */
    syslog (LOG_ALERT,
	    _("netname2user: DES entry for %s in directory %s not unique"),
	    netname, domain);

  len = ENTRY_LEN (res->objects.objects_val, 0);
  strncpy (principal, ENTRY_VAL (res->objects.objects_val, 0), len);
  principal[len] = '\0';
  nis_freeresult (res);

  if (principal[0] == '\0')
    return NSS_STATUS_UNAVAIL;

  /*
   * 3.  Use principal name to look up uid/gid information in
   *     LOCAL entry in **local** cred table.
   */
  domain = nis_local_directory ();
  if ((strlen (principal) + strlen (domain) + 45) > (size_t) NIS_MAXNAMELEN)
    {
      syslog (LOG_ERR, _("netname2user: principal name `%s' too long"),
	      principal);
      return NSS_STATUS_UNAVAIL;
    }

  slen = sprintf (sname, "[cname=%s,auth_type=LOCAL],cred.org_dir.%s",
		  principal, domain);

  if (sname[slen - 1] != '.')
    {
      sname[slen++] = '.';
      sname[slen] = '\0';
    }

  /* must use authenticated call here */
  /* XXX but we cant, for now. XXX */
  res = nis_list (sname, USE_DGRAM+NO_AUTHINFO+FOLLOW_LINKS+FOLLOW_PATH,
		  NULL, NULL);
  if (res == NULL)
    {
      *errnop = ENOMEM;
      return NSS_STATUS_TRYAGAIN;
    }
  switch(res->status)
    {
    case NIS_NOTFOUND:
    case NIS_PARTIAL:
    case NIS_NOSUCHNAME:
    case NIS_NOSUCHTABLE:
      nis_freeresult (res);
      return NSS_STATUS_NOTFOUND;
    case NIS_S_NOTFOUND:
    case NIS_TRYAGAIN:
      syslog (LOG_ERR, _("netname2user: (nis+ lookup): %s\n"),
	      nis_sperrno (res->status));
      nis_freeresult (res);
      *errnop = errno;
      return NSS_STATUS_TRYAGAIN;
    case NIS_SUCCESS:
    case NIS_S_SUCCESS:
      break;   /* go and do something useful */
    default:
      syslog (LOG_ERR, _("netname2user: (nis+ lookup): %s\n"),
	      nis_sperrno (res->status));
      nis_freeresult (res);
      return NSS_STATUS_UNAVAIL;
    }

  if (res->objects.objects_len > 1)
    /*
     * A principal can have more than one LOCAL entry?
     * Something wrong with cred table.
     * Warn user and continue.
     */
    syslog (LOG_ALERT,
	    _("netname2user: LOCAL entry for %s in directory %s not unique"),
	    netname, domain);
  /* Fetch the uid */
  *uidp = strtoul (ENTRY_VAL (res->objects.objects_val, 2), NULL, 10);

  if (*uidp == 0)
    {
      syslog (LOG_ERR, _("netname2user: should not have uid 0"));
      return NSS_STATUS_NOTFOUND;
    }

  parse_grp_str (ENTRY_VAL (res->objects.objects_val, 3),
		 gidp, gidlenp, gidlist, errnop);

  nis_freeresult (res);
  return NSS_STATUS_SUCCESS;
}
