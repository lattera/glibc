/* Copyright (C) 1996, 1997, 1998, 1999, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1996.

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
#include <pwd.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <netdb.h>
#include <string.h>
#include <bits/libc-lock.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>
#include <rpcsvc/nis.h>
#include <nsswitch.h>

#include "netgroup.h"
#include "nss-nisplus.h"
#include "nisplus-parser.h"

static service_user *ni;
static bool_t use_nisplus; /* default: passwd_compat: nis */
static nis_name pwdtable; /* Name of the pwd table */
static size_t pwdtablelen;

/* Get the declaration of the parser function.  */
#define ENTNAME pwent
#define STRUCTURE passwd
#define EXTERN_PARSER
#include <nss/nss_files/files-parse.c>

/* Structure for remembering -@netgroup and -user members ... */
#define BLACKLIST_INITIAL_SIZE 512
#define BLACKLIST_INCREMENT 256
struct blacklist_t
  {
    char *data;
    int current;
    int size;
  };

struct ent_t
  {
    bool_t netgroup;
    bool_t nis;
    bool_t first;
    char *oldkey;
    int oldkeylen;
    nis_result *result;
    FILE *stream;
    struct blacklist_t blacklist;
    struct passwd pwd;
    struct __netgrent netgrdata;
  };
typedef struct ent_t ent_t;

static ent_t ext_ent = {0, 0, 0, NULL, 0, NULL, NULL, {NULL, 0, 0},
			{NULL, NULL, 0, 0, NULL, NULL, NULL}};

/* Protect global state against multiple changers.  */
__libc_lock_define_initialized (static, lock)

/* Prototypes for local functions.  */
static void blacklist_store_name (const char *, ent_t *);
static int in_blacklist (const char *, int, ent_t *);

static void
give_pwd_free (struct passwd *pwd)
{
  if (pwd->pw_name != NULL)
    free (pwd->pw_name);
  if (pwd->pw_passwd != NULL)
    free (pwd->pw_passwd);
  if (pwd->pw_gecos != NULL)
    free (pwd->pw_gecos);
  if (pwd->pw_dir != NULL)
    free (pwd->pw_dir);
  if (pwd->pw_shell != NULL)
    free (pwd->pw_shell);

  memset (pwd, '\0', sizeof (struct passwd));
}

static size_t
pwd_need_buflen (struct passwd *pwd)
{
  size_t len = 0;

  if (pwd->pw_passwd != NULL)
    len += strlen (pwd->pw_passwd) + 1;

  if (pwd->pw_gecos != NULL)
    len += strlen (pwd->pw_gecos) + 1;

  if (pwd->pw_dir != NULL)
    len += strlen (pwd->pw_dir) + 1;

  if (pwd->pw_shell != NULL)
    len += strlen (pwd->pw_shell) + 1;

  return len;
}

static void
copy_pwd_changes (struct passwd *dest, struct passwd *src,
		  char *buffer, size_t buflen)
{
  if (src->pw_passwd != NULL && strlen (src->pw_passwd))
    {
      if (buffer == NULL)
	dest->pw_passwd = strdup (src->pw_passwd);
      else if (dest->pw_passwd &&
	       strlen (dest->pw_passwd) >= strlen (src->pw_passwd))
	strcpy (dest->pw_passwd, src->pw_passwd);
      else
	{
	  dest->pw_passwd = buffer;
	  strcpy (dest->pw_passwd, src->pw_passwd);
	  buffer += strlen (dest->pw_passwd) + 1;
	  buflen = buflen - (strlen (dest->pw_passwd) + 1);
	}
    }

  if (src->pw_gecos != NULL && strlen (src->pw_gecos))
    {
      if (buffer == NULL)
	dest->pw_gecos = strdup (src->pw_gecos);
      else if (dest->pw_gecos &&
	       strlen (dest->pw_gecos) >= strlen (src->pw_gecos))
	strcpy (dest->pw_gecos, src->pw_gecos);
      else
	{
	  dest->pw_gecos = buffer;
	  strcpy (dest->pw_gecos, src->pw_gecos);
	  buffer += strlen (dest->pw_gecos) + 1;
	  buflen = buflen - (strlen (dest->pw_gecos) + 1);
	}
    }
  if (src->pw_dir != NULL && strlen (src->pw_dir))
    {
      if (buffer == NULL)
	dest->pw_dir = strdup (src->pw_dir);
      else if (dest->pw_dir &&
	       strlen (dest->pw_dir) >= strlen (src->pw_dir))
	strcpy (dest->pw_dir, src->pw_dir);
      else
	{
	  dest->pw_dir = buffer;
	  strcpy (dest->pw_dir, src->pw_dir);
	  buffer += strlen (dest->pw_dir) + 1;
	  buflen = buflen - (strlen (dest->pw_dir) + 1);
	}
    }

  if (src->pw_shell != NULL && strlen (src->pw_shell))
    {
      if (buffer == NULL)
	dest->pw_shell = strdup (src->pw_shell);
      else if (dest->pw_shell &&
	       strlen (dest->pw_shell) >= strlen (src->pw_shell))
	strcpy (dest->pw_shell, src->pw_shell);
      else
	{
	  dest->pw_shell = buffer;
	  strcpy (dest->pw_shell, src->pw_shell);
	  buffer += strlen (dest->pw_shell) + 1;
	  buflen = buflen - (strlen (dest->pw_shell) + 1);
	}
    }
}

static enum nss_status
insert_passwd_adjunct (char **result, int *len, char *domain, int *errnop)
{
  char *p1, *p2, *result2, *res;
  int len2;
  size_t namelen;

  /* Check for adjunct style secret passwords.  They can be
     recognized by a password starting with "##".  */
  p1 = strchr (*result, ':');
  if (p1 == NULL || p1[1] != '#' || p1[2] != '#')
    return NSS_STATUS_SUCCESS;
  p2 = strchr (p1 + 3, ':');

  namelen = p2 - p1 - 3;

  if (yp_match (domain, "passwd.adjunct.byname", &p1[3], namelen,
		&result2, &len2) == YPERR_SUCCESS)
    {
      /* We found a passwd.adjunct entry.  Merge encrypted
	 password therein into original result.  */
      char *encrypted = strchr (result2, ':');
      char *endp;
      size_t restlen;

      if (encrypted == NULL || (endp = strchr (++encrypted, ':')) == NULL)
	{
	  /* Invalid format of the entry.  This never should happen
	     unless the data from which the NIS table is generated is
	     wrong.  We simply ignore it.  */
	  free (result2);
	  return NSS_STATUS_SUCCESS;
	}

      restlen = *len - (p2 - *result);
      if ((res = malloc (namelen + restlen + (endp - encrypted) + 2)) == NULL)
	{
	  free (result2);
	  *errnop = ENOMEM;
	  return NSS_STATUS_TRYAGAIN;
	}

      __mempcpy (__mempcpy (__mempcpy (__mempcpy
				       (res, *result, (p1 - *result)),
				       ":", 1),
			    encrypted, endp - encrypted),
		 p2, restlen + 1);

      free (result2);
      free (*result);
      *result = res;
      *len = strlen (res);
    }
  return NSS_STATUS_SUCCESS;
}

static enum nss_status
internal_setpwent (ent_t *ent)
{
  enum nss_status status = NSS_STATUS_SUCCESS;

  ent->nis = ent->first = ent->netgroup = 0;

  /* If something was left over free it.  */
  if (ent->netgroup)
    __internal_endnetgrent (&ent->netgrdata);

  if (ent->oldkey != NULL)
    {
      free (ent->oldkey);
      ent->oldkey = NULL;
      ent->oldkeylen = 0;
    }

  if (ent->result != NULL)
    {
      nis_freeresult (ent->result);
      ent->result = NULL;
    }

  if (pwdtable == NULL)
    {
      static const char key[] = "passwd.org_dir.";
      const char *local_dir = nis_local_directory ();
      size_t len_local_dir = strlen (local_dir);

      pwdtable = malloc (sizeof (key) + len_local_dir);
      if (pwdtable == NULL)
        return NSS_STATUS_TRYAGAIN;

      pwdtablelen = ((char *) mempcpy (mempcpy (pwdtable,
						key, sizeof (key) - 1),
				       local_dir, len_local_dir + 1)
		     - pwdtable) - 1;
    }

  if (ent->blacklist.data != NULL)
    {
      ent->blacklist.current = 1;
      ent->blacklist.data[0] = '|';
      ent->blacklist.data[1] = '\0';
    }
  else
    ent->blacklist.current = 0;

  if (ent->stream == NULL)
    {
      ent->stream = fopen ("/etc/passwd", "r");

      if (ent->stream == NULL)
	status = errno == EAGAIN ? NSS_STATUS_TRYAGAIN : NSS_STATUS_UNAVAIL;
      else
	{
	  /* We have to make sure the file is  `closed on exec'.  */
	  int result, flags;

	  result = flags = fcntl (fileno (ent->stream), F_GETFD, 0);
	  if (result >= 0)
	    {
	      flags |= FD_CLOEXEC;
	      result = fcntl (fileno (ent->stream), F_SETFD, flags);
	    }
	  if (result < 0)
	    {
	      /* Something went wrong.  Close the stream and return a
		 failure.  */
	      fclose (ent->stream);
	      ent->stream = NULL;
	      status = NSS_STATUS_UNAVAIL;
	    }
	}
    }
  else
    rewind (ent->stream);

  give_pwd_free (&ent->pwd);

  return status;
}


enum nss_status
_nss_compat_setpwent (int stayopen)
{
  enum nss_status result;

  __libc_lock_lock (lock);

  if (ni == NULL)
    {
      __nss_database_lookup ("passwd_compat", NULL, "nis", &ni);
      use_nisplus = (strcmp (ni->name, "nisplus") == 0);
    }

  result = internal_setpwent (&ext_ent);

  __libc_lock_unlock (lock);

  return result;
}


static enum nss_status
internal_endpwent (ent_t *ent)
{
  if (ent->stream != NULL)
    {
      fclose (ent->stream);
      ent->stream = NULL;
    }

  if (ent->netgroup)
    __internal_endnetgrent (&ent->netgrdata);

  ent->nis = ent->first = ent->netgroup = 0;

  if (ent->oldkey != NULL)
    {
      free (ent->oldkey);
      ent->oldkey = NULL;
      ent->oldkeylen = 0;
    }

  if (ent->result != NULL)
    {
      nis_freeresult (ent->result);
      ent->result = NULL;
    }

  if (ent->blacklist.data != NULL)
    {
      ent->blacklist.current = 1;
      ent->blacklist.data[0] = '|';
     ent->blacklist.data[1] = '\0';
    }
  else
    ent->blacklist.current = 0;

  give_pwd_free (&ent->pwd);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_compat_endpwent (void)
{
  enum nss_status result;

  __libc_lock_lock (lock);

  result = internal_endpwent (&ext_ent);

  __libc_lock_unlock (lock);

  return result;
}

static enum nss_status
getpwent_next_nis_netgr (const char *name, struct passwd *result, ent_t *ent,
			 char *group, char *buffer, size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  char *ypdomain, *host, *user, *domain, *outval, *p, *p2;
  int status, outvallen;
  size_t p2len;

  if (yp_get_default_domain (&ypdomain) != YPERR_SUCCESS)
    {
      ent->netgroup = 0;
      ent->first = 0;
      give_pwd_free (&ent->pwd);
      return NSS_STATUS_UNAVAIL;
    }

  if (ent->first == TRUE)
    {
      memset (&ent->netgrdata, 0, sizeof (struct __netgrent));
      __internal_setnetgrent (group, &ent->netgrdata);
      ent->first = FALSE;
    }

  while (1)
    {
      char *saved_cursor;
      int parse_res;

      saved_cursor = ent->netgrdata.cursor;
      status = __internal_getnetgrent_r (&host, &user, &domain,
					 &ent->netgrdata, buffer, buflen,
					 errnop);
      if (status != 1)
	{
	  __internal_endnetgrent (&ent->netgrdata);
	  ent->netgroup = 0;
	  give_pwd_free (&ent->pwd);
	  return NSS_STATUS_RETURN;
	}

      if (user == NULL || user[0] == '-')
	continue;

      if (domain != NULL && strcmp (ypdomain, domain) != 0)
	continue;

      /* If name != NULL, we are called from getpwnam.  */
      if (name != NULL)
	if (strcmp (user, name) != 0)
	  continue;

      if (yp_match (ypdomain, "passwd.byname", user,
		    strlen (user), &outval, &outvallen)
	  != YPERR_SUCCESS)
	continue;

      if (insert_passwd_adjunct (&outval, &outvallen, ypdomain, errnop)
	  != NSS_STATUS_SUCCESS)
	{
	  free (outval);
	  return NSS_STATUS_TRYAGAIN;
	}

      p2len = pwd_need_buflen (&ent->pwd);
      if (p2len > buflen)
	{
	  free (outval);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      p2 = buffer + (buflen - p2len);
      buflen -= p2len;

      if (buflen < ((size_t) outvallen + 1))
	{
	  free (outval);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      p = strncpy (buffer, outval, buflen);

      while (isspace (*p))
	p++;
      free (outval);
      parse_res = _nss_files_parse_pwent (p, result, data, buflen, errnop);
      if (parse_res == -1)
	{
	  ent->netgrdata.cursor = saved_cursor;
	  return NSS_STATUS_TRYAGAIN;
	}

      if (parse_res && !in_blacklist (result->pw_name,
				      strlen (result->pw_name), ent))
	{
	  /* Store the User in the blacklist for the "+" at the end of
	     /etc/passwd */
	  blacklist_store_name (result->pw_name, ent);
	  copy_pwd_changes (result, &ent->pwd, p2, p2len);
	  break;
	}
    }

  return NSS_STATUS_SUCCESS;
}

static enum nss_status
getpwent_next_nisplus_netgr (const char *name, struct passwd *result,
			     ent_t *ent, char *group, char *buffer,
			     size_t buflen, int *errnop)
{
  char *ypdomain, *host, *user, *domain, *p2;
  int status, parse_res;
  size_t p2len;
  nis_result *nisres;

  /* Maybe we should use domainname here ? We need the current
     domainname for the domain field in netgroups */
  if (yp_get_default_domain (&ypdomain) != YPERR_SUCCESS)
    {
      ent->netgroup = 0;
      ent->first = 0;
      give_pwd_free (&ent->pwd);
      return NSS_STATUS_UNAVAIL;
    }

  if (ent->first == TRUE)
    {
      bzero (&ent->netgrdata, sizeof (struct __netgrent));
      __internal_setnetgrent (group, &ent->netgrdata);
      ent->first = FALSE;
    }

  while (1)
    {
      char *saved_cursor;

      saved_cursor = ent->netgrdata.cursor;
      status = __internal_getnetgrent_r (&host, &user, &domain,
					 &ent->netgrdata, buffer, buflen,
					 errnop);
      if (status != 1)
	{
	  __internal_endnetgrent (&ent->netgrdata);
	  ent->netgroup = 0;
	  give_pwd_free (&ent->pwd);
	  return NSS_STATUS_RETURN;
	}

      if (user == NULL || user[0] == '-')
	continue;

      if (domain != NULL && strcmp (ypdomain, domain) != 0)
	continue;

      /* If name != NULL, we are called from getpwnam */
      if (name != NULL)
	if (strcmp (user, name) != 0)
	  continue;

      p2len = pwd_need_buflen (&ent->pwd);
      if (p2len > buflen)
	{
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      p2 = buffer + (buflen - p2len);
      buflen -= p2len;
      {
	char buf[strlen (user) + 30 + pwdtablelen];
	sprintf(buf, "[name=%s],%s", user, pwdtable);
	nisres = nis_list(buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);
      }
      if (niserr2nss (nisres->status) != NSS_STATUS_SUCCESS)
	{
	  nis_freeresult (nisres);
	  continue;
	}
      parse_res = _nss_nisplus_parse_pwent (nisres, result, buffer,
					    buflen, errnop);
      if (parse_res == -1)
	{
	  nis_freeresult (nisres);
	  ent->netgrdata.cursor = saved_cursor;
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      nis_freeresult (nisres);

      if (parse_res && !in_blacklist (result->pw_name,
				      strlen (result->pw_name), ent))
	{
	  /* Store the User in the blacklist for the "+" at the end of
	     /etc/passwd */
	  blacklist_store_name (result->pw_name, ent);
	  copy_pwd_changes (result, &ent->pwd, p2, p2len);
	  break;
	}
    }

  return NSS_STATUS_SUCCESS;
}

/* get the next user from NIS+  (+ entry) */
static enum nss_status
getpwent_next_nisplus (struct passwd *result, ent_t *ent, char *buffer,
		       size_t buflen, int *errnop)
{
  int parse_res;
  size_t p2len;
  char *p2;

  p2len = pwd_need_buflen (&ent->pwd);
  if (p2len > buflen)
    {
      *errnop = ERANGE;
      return NSS_STATUS_TRYAGAIN;
    }
  p2 = buffer + (buflen - p2len);
  buflen -= p2len;
  do
    {
      bool_t saved_first;
      nis_result *saved_res;

      if (ent->first)
	{
	  saved_first = TRUE;
	  saved_res = ent->result;

	  ent->result = nis_first_entry (pwdtable);
          if (niserr2nss (ent->result->status) != NSS_STATUS_SUCCESS)
	    {
	      ent->nis = 0;
	      give_pwd_free (&ent->pwd);
	      return niserr2nss (ent->result->status);
	    }
	  ent->first = FALSE;
	}
      else
	{
	  nis_result *res;

	  res = nis_next_entry (pwdtable, &ent->result->cookie);
	  saved_res = ent->result;
	  saved_first = FALSE;
	  ent->result = res;
	  if (niserr2nss (ent->result->status) != NSS_STATUS_SUCCESS)
	    {
	      ent->nis = 0;
	      nis_freeresult (saved_res);
	      give_pwd_free (&ent->pwd);
	      return niserr2nss (ent->result->status);
	    }
	}
      parse_res = _nss_nisplus_parse_pwent (ent->result, result, buffer,
					    buflen, errnop);
      if (parse_res == -1)
	{
	  nis_freeresult (ent->result);
	  ent->result = saved_res;
	  ent->first = saved_first;
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      else
	{
	  if (!saved_first)
	    nis_freeresult (saved_res);
	}

      if (parse_res &&
	  in_blacklist (result->pw_name, strlen (result->pw_name), ent))
	parse_res = 0; /* if result->pw_name in blacklist,search next entry */
    }
  while (!parse_res);

  copy_pwd_changes (result, &ent->pwd, p2, p2len);

  return NSS_STATUS_SUCCESS;
}

static enum nss_status
getpwent_next_nis (struct passwd *result, ent_t *ent, char *buffer,
		   size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  char *domain, *outkey, *outval, *p, *p2;
  int outkeylen, outvallen, parse_res;
  size_t p2len;

  if (yp_get_default_domain (&domain) != YPERR_SUCCESS)
    {
      ent->nis = 0;
      give_pwd_free (&ent->pwd);
      return NSS_STATUS_UNAVAIL;
    }

  p2len = pwd_need_buflen (&ent->pwd);
  if (p2len > buflen)
    {
      *errnop = ERANGE;
      return NSS_STATUS_TRYAGAIN;
    }
  p2 = buffer + (buflen - p2len);
  buflen -= p2len;
  do
    {
      bool_t saved_first;
      char *saved_oldkey;
      int saved_oldlen;

      if (ent->first)
	{
	  if (yp_first (domain, "passwd.byname", &outkey, &outkeylen,
			&outval, &outvallen) != YPERR_SUCCESS)
	    {
	      ent->nis = 0;
	      give_pwd_free (&ent->pwd);
	      return NSS_STATUS_UNAVAIL;
	    }

	  if (insert_passwd_adjunct (&outval, &outvallen, domain, errnop) !=
	      NSS_STATUS_SUCCESS)
	    {
	      free (outval);
	      return NSS_STATUS_TRYAGAIN;
	    }

	  if (buflen < ((size_t) outvallen + 1))
	    {
	      free (outval);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }

	  saved_first = TRUE;
	  saved_oldkey = ent->oldkey;
	  saved_oldlen = ent->oldkeylen;
	  ent->oldkey = outkey;
	  ent->oldkeylen = outkeylen;
	  ent->first = FALSE;
	}
      else
	{
	  if (yp_next (domain, "passwd.byname", ent->oldkey, ent->oldkeylen,
		       &outkey, &outkeylen, &outval, &outvallen)
	      != YPERR_SUCCESS)
	    {
	      ent->nis = 0;
	      give_pwd_free (&ent->pwd);
	      *errnop = ENOENT;
	      return NSS_STATUS_NOTFOUND;
	    }

	  if (insert_passwd_adjunct (&outval, &outvallen, domain, errnop)
	      != NSS_STATUS_SUCCESS)
	    {
	      free (outval);
	      return NSS_STATUS_TRYAGAIN;
	    }

	  if (buflen < ((size_t) outvallen + 1))
	    {
	      free (outval);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }

	  saved_first = FALSE;
	  saved_oldkey = ent->oldkey;
	  saved_oldlen = ent->oldkeylen;
	  ent->oldkey = outkey;
	  ent->oldkeylen = outkeylen;
	}

      /* Copy the found data to our buffer  */
      p = strncpy (buffer, outval, buflen);

      /* ...and free the data.  */
      free (outval);

      while (isspace (*p))
	++p;
      parse_res = _nss_files_parse_pwent (p, result, data, buflen, errnop);
      if (parse_res == -1)
	{
	  free (ent->oldkey);
	  ent->oldkey = saved_oldkey;
	  ent->oldkeylen = saved_oldlen;
	  ent->first = saved_first;
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      else
	{
	  if (!saved_first)
	    free (saved_oldkey);
	}
      if (parse_res
	  && in_blacklist (result->pw_name, strlen (result->pw_name), ent))
	parse_res = 0;
    }
  while (!parse_res);

  copy_pwd_changes (result, &ent->pwd, p2, p2len);

  return NSS_STATUS_SUCCESS;
}

/* This function handle the +user entrys in /etc/passwd */
static enum nss_status
getpwnam_plususer (const char *name, struct passwd *result, ent_t *ent,
		   char *buffer, size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  struct passwd pwd;
  int parse_res;
  char *p;
  size_t plen;

  memset (&pwd, '\0', sizeof (struct passwd));

  copy_pwd_changes (&pwd, result, NULL, 0);

  plen = pwd_need_buflen (&pwd);
  if (plen > buflen)
    {
      *errnop = ERANGE;
      return NSS_STATUS_TRYAGAIN;
    }
  p = buffer + (buflen - plen);
  buflen -= plen;

  if (use_nisplus) /* Do the NIS+ query here */
    {
      nis_result *res;
      char buf[strlen (name) + 24 + pwdtablelen];

      sprintf(buf, "[name=%s],%s", name, pwdtable);
      res = nis_list(buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);
      if (niserr2nss (res->status) != NSS_STATUS_SUCCESS)
	{
	  enum nss_status status =  niserr2nss (res->status);

	  nis_freeresult (res);
	  return status;
	}
      parse_res = _nss_nisplus_parse_pwent (res, result, buffer,
					    buflen, errnop);

      nis_freeresult (res);

      if (parse_res == -1)
	{
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      if (in_blacklist (result->pw_name, strlen (result->pw_name), ent))
	{
	  *errnop = ENOENT;
	  return NSS_STATUS_NOTFOUND;
	}
    }
  else /* Use NIS */
    {
      char *domain, *outval, *ptr;
      int outvallen;

      if (yp_get_default_domain (&domain) != YPERR_SUCCESS)
	{
	  *errnop = ENOENT;
	  return NSS_STATUS_NOTFOUND;
	}

      if (yp_match (domain, "passwd.byname", name, strlen (name),
		    &outval, &outvallen) != YPERR_SUCCESS)
	{
	  *errnop = ENOENT;
	  return NSS_STATUS_NOTFOUND;
	}

      if (insert_passwd_adjunct (&outval, &outvallen, domain, errnop)
	  != NSS_STATUS_SUCCESS)
	{
	  free (outval);
	  return NSS_STATUS_TRYAGAIN;
	}

      if (buflen < ((size_t) outvallen + 1))
	{
	  free (outval);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      ptr = strncpy (buffer, outval, buflen);
      free (outval);

      while (isspace (*ptr))
	ptr++;

      parse_res = _nss_files_parse_pwent (ptr, result, data, buflen, errnop);
      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;

      if (in_blacklist (result->pw_name, strlen (result->pw_name), ent))
	{
	  *errnop = ENOENT;
	  return NSS_STATUS_NOTFOUND;
	}
    }

  if (parse_res > 0)
    {
      copy_pwd_changes (result, &pwd, p, plen);
      give_pwd_free (&pwd);
      /* We found the entry.  */
      return NSS_STATUS_SUCCESS;
    }
  else
    {
      /* Give buffer the old len back */
      buflen += plen;
      give_pwd_free (&pwd);
    }
  return NSS_STATUS_RETURN;
}

static enum nss_status
getpwent_next_file (struct passwd *result, ent_t *ent,
		    char *buffer, size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  while (1)
    {
      fpos_t pos;
      char *p;
      int parse_res;

      do
	{
	  fgetpos (ent->stream, &pos);
	  buffer[buflen - 1] = '\xff';
	  p = fgets (buffer, buflen, ent->stream);
	  if (p == NULL && feof (ent->stream))
	    {
	      *errnop = ENOENT;
	      return NSS_STATUS_NOTFOUND;
	    }
	  if (p == NULL || buffer[buflen - 1] != '\xff')
	    {
	      fsetpos (ent->stream, &pos);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }

	  /* Terminate the line for any case.  */
	  buffer[buflen - 1] = '\0';

	  /* Skip leading blanks.  */
	  while (isspace (*p))
	    ++p;
	}
      while (*p == '\0' || *p == '#' || /* Ignore empty and comment lines.  */
      /* Parse the line.  If it is invalid, loop to
         get the next line of the file to parse.  */
	     !(parse_res = _nss_files_parse_pwent (p, result, data, buflen,
						   errnop)));

      if (parse_res == -1)
	{
	  /* The parser ran out of space.  */
	  fsetpos (ent->stream, &pos);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      if (result->pw_name[0] != '+' && result->pw_name[0] != '-')
	/* This is a real entry.  */
	break;

      /* -@netgroup */
      if (result->pw_name[0] == '-' && result->pw_name[1] == '@'
	  && result->pw_name[2] != '\0')
	{
	  /* XXX Do not use fixed length buffer.  */
	  char buf2[1024];
	  char *user, *host, *domain;
	  struct __netgrent netgrdata;

	  bzero (&netgrdata, sizeof (struct __netgrent));
	  __internal_setnetgrent (&result->pw_name[2], &netgrdata);
	  while (__internal_getnetgrent_r (&host, &user, &domain, &netgrdata,
					   buf2, sizeof (buf2), errnop))
	    {
	      if (user != NULL && user[0] != '-')
		blacklist_store_name (user, ent);
	    }
	  __internal_endnetgrent (&netgrdata);
	  continue;
	}

      /* +@netgroup */
      if (result->pw_name[0] == '+' && result->pw_name[1] == '@'
	  && result->pw_name[2] != '\0')
	{
	  enum nss_status status;

	  ent->netgroup = TRUE;
	  ent->first = TRUE;
	  copy_pwd_changes (&ent->pwd, result, NULL, 0);

	  if (use_nisplus)
	    status =  getpwent_next_nisplus_netgr (NULL, result, ent,
						   &result->pw_name[2],
						   buffer, buflen, errnop);
	  else
	    status =  getpwent_next_nis_netgr (NULL, result, ent,
					       &result->pw_name[2],
					       buffer, buflen, errnop);
	  if (status == NSS_STATUS_RETURN)
	    continue;
	  else
	    {
	      if (status == NSS_STATUS_NOTFOUND)
		*errnop = ENOENT;
	      return status;
	    }
	}

      /* -user */
      if (result->pw_name[0] == '-' && result->pw_name[1] != '\0'
	  && result->pw_name[1] != '@')
	{
	  blacklist_store_name (&result->pw_name[1], ent);
	  continue;
	}

      /* +user */
      if (result->pw_name[0] == '+' && result->pw_name[1] != '\0'
	  && result->pw_name[1] != '@')
	{
	  char buf[strlen (result->pw_name)];
	  enum nss_status status;

	  /* Store the User in the blacklist for the "+" at the end of
	     /etc/passwd */
	  strcpy (buf, &result->pw_name[1]);
	  status = getpwnam_plususer (&result->pw_name[1], result, ent,
				      buffer, buflen, errnop);
	  blacklist_store_name (buf, ent);

	  if (status == NSS_STATUS_SUCCESS) /* We found the entry. */
	    break;
	  else
	    if (status == NSS_STATUS_RETURN /* We couldn't parse the entry */
		|| status == NSS_STATUS_NOTFOUND) /* entry doesn't exist */
	      continue;
	    else
	      {
		if (status == NSS_STATUS_TRYAGAIN)
		  {
		    /* The parser ran out of space */
		    fsetpos (ent->stream, &pos);
		    *errnop = ERANGE;
		  }
		return status;
	      }
	}

      /* +:... */
      if (result->pw_name[0] == '+' && result->pw_name[1] == '\0')
	{
	  ent->nis = TRUE;
	  ent->first = TRUE;
	  copy_pwd_changes (&ent->pwd, result, NULL, 0);

	  if (use_nisplus)
	    return getpwent_next_nisplus (result, ent, buffer, buflen, errnop);
	  else
	    return getpwent_next_nis (result, ent, buffer, buflen, errnop);
	}
    }

  return NSS_STATUS_SUCCESS;
}


static enum nss_status
internal_getpwent_r (struct passwd *pw, ent_t *ent, char *buffer,
		     size_t buflen, int *errnop)
{
  if (ent->netgroup)
    {
      enum nss_status status;

      /* We are searching members in a netgroup */
      /* Since this is not the first call, we don't need the group name */
      if (use_nisplus)
	status = getpwent_next_nisplus_netgr (NULL, pw, ent, NULL, buffer,
					      buflen, errnop);
      else
	status = getpwent_next_nis_netgr (NULL, pw, ent, NULL, buffer, buflen,
					  errnop);
      if (status == NSS_STATUS_RETURN)
	return getpwent_next_file (pw, ent, buffer, buflen, errnop);
      else
	return status;
    }
  else
    if (ent->nis)
      {
	if (use_nisplus)
	  return getpwent_next_nisplus (pw, ent, buffer, buflen, errnop);
	else
	  return getpwent_next_nis (pw, ent, buffer, buflen, errnop);
      }
    else
      return getpwent_next_file (pw, ent, buffer, buflen, errnop);
}

enum nss_status
_nss_compat_getpwent_r (struct passwd *pwd, char *buffer, size_t buflen,
			int *errnop)
{
  enum nss_status status = NSS_STATUS_SUCCESS;

  __libc_lock_lock (lock);

  if (ni == NULL)
    {
      __nss_database_lookup ("passwd_compat", NULL, "nis", &ni);
      use_nisplus = (strcmp (ni->name, "nisplus") == 0);
    }

  /* Be prepared that the setpwent function was not called before.  */
  if (ext_ent.stream == NULL)
    status = internal_setpwent (&ext_ent);

  if (status == NSS_STATUS_SUCCESS)
    status = internal_getpwent_r (pwd, &ext_ent, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}

/* Searches in /etc/passwd and the NIS/NIS+ map for a special user */
static enum nss_status
internal_getpwnam_r (const char *name, struct passwd *result, ent_t *ent,
		     char *buffer, size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;

  while (1)
    {
      fpos_t pos;
      char *p;
      int parse_res;

      do
	{
	  fgetpos (ent->stream, &pos);
	  buffer[buflen - 1] = '\xff';
	  p = fgets (buffer, buflen, ent->stream);
	  if (p == NULL && feof (ent->stream))
	    {
	      *errnop = ENOENT;
	      return NSS_STATUS_NOTFOUND;
	    }
	  if (p == NULL || buffer[buflen - 1] != '\xff')
	    {
	      fsetpos (ent->stream, &pos);
	      *errnop = ERANGE;
	      return NSS_STATUS_TRYAGAIN;
	    }

	  /* Terminate the line for any case.  */
	  buffer[buflen - 1] = '\0';

	  /* Skip leading blanks.  */
	  while (isspace (*p))
	    ++p;
	}
      while (*p == '\0' || *p == '#' || /* Ignore empty and comment lines.  */
	     /* Parse the line.  If it is invalid, loop to
		get the next line of the file to parse.  */
	     !(parse_res = _nss_files_parse_pwent (p, result, data, buflen,
						   errnop)));

      if (parse_res == -1)
	{
	  /* The parser ran out of space.  */
	  fsetpos (ent->stream, &pos);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      /* This is a real entry.  */
      if (result->pw_name[0] != '+' && result->pw_name[0] != '-')
	{
	  if (strcmp (result->pw_name, name) == 0)
	    return NSS_STATUS_SUCCESS;
	  else
	    continue;
	}

      /* -@netgroup */
      if (result->pw_name[0] == '-' && result->pw_name[1] == '@'
	  && result->pw_name[2] != '\0')
	{
	  if (innetgr (&result->pw_name[2], NULL, name, NULL))
	    return NSS_STATUS_NOTFOUND;
	  continue;
	}

      /* +@netgroup */
      if (result->pw_name[0] == '+' && result->pw_name[1] == '@'
	  && result->pw_name[2] != '\0')
	{
	  enum nss_status status;

	  if (innetgr (&result->pw_name[2], NULL, name, NULL))
	    {
	      status = getpwnam_plususer (name, result, ent, buffer,
					  buflen, errnop);

              if (status == NSS_STATUS_RETURN)
                continue;

	      return status;
	    }
	  continue;
	}

      /* -user */
      if (result->pw_name[0] == '-' && result->pw_name[1] != '\0'
	  && result->pw_name[1] != '@')
	{
	  if (strcmp (&result->pw_name[1], name) == 0)
	    {
	      *errnop = ENOENT;
	      return NSS_STATUS_NOTFOUND;
	    }
	  else
	    continue;
	}

      /* +user */
      if (result->pw_name[0] == '+' && result->pw_name[1] != '\0'
	  && result->pw_name[1] != '@')
	{
	  if (strcmp (name, &result->pw_name[1]) == 0)
	    {
	      enum nss_status status;

	      status = getpwnam_plususer (name, result, ent, buffer, buflen,
					  errnop);
	      if (status == NSS_STATUS_RETURN)
		/* We couldn't parse the entry */
		return NSS_STATUS_NOTFOUND;
	      else
		return status;
	    }
	}

      /* +:... */
      if (result->pw_name[0] == '+' && result->pw_name[1] == '\0')
	{
	  enum nss_status status;

	  status = getpwnam_plususer (name, result, ent,
				      buffer, buflen, errnop);
	  if (status == NSS_STATUS_SUCCESS) /* We found the entry. */
	    break;
	  else
	    if (status == NSS_STATUS_RETURN) /* We couldn't parse the entry */
	      return NSS_STATUS_NOTFOUND;
	    else
	      return status;
	}
    }
  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_compat_getpwnam_r (const char *name, struct passwd *pwd,
			char *buffer, size_t buflen, int *errnop)
{
  ent_t ent = {0, 0, 0, NULL, 0, NULL, NULL, {NULL, 0, 0},
	       {NULL, NULL, 0, 0, NULL, NULL, NULL}};
  enum nss_status status;

  if (name[0] == '-' || name[0] == '+')
    {
      *errnop = ENOENT;
      return NSS_STATUS_NOTFOUND;
    }

  __libc_lock_lock (lock);

  if (ni == NULL)
    {
      __nss_database_lookup ("passwd_compat", NULL, "nis", &ni);
      use_nisplus = (strcmp (ni->name, "nisplus") == 0);
    }

  __libc_lock_unlock (lock);

  status = internal_setpwent (&ent);
  if (status != NSS_STATUS_SUCCESS)
    return status;

  status = internal_getpwnam_r (name, pwd, &ent, buffer, buflen, errnop);

  internal_endpwent (&ent);

  return status;
}

/* This function handle the + entry in /etc/passwd for getpwuid */
static enum nss_status
getpwuid_plususer (uid_t uid, struct passwd *result, char *buffer,
                   size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;
  struct passwd pwd;
  int parse_res;
  char *p;
  size_t plen;

  memset (&pwd, '\0', sizeof (struct passwd));

  copy_pwd_changes (&pwd, result, NULL, 0);

  plen = pwd_need_buflen (&pwd);
  if (plen > buflen)
    {
      *errnop = ERANGE;
      return NSS_STATUS_TRYAGAIN;
    }
  p = buffer + (buflen - plen);
  buflen -= plen;

  if (use_nisplus) /* Do the NIS+ query here */
    {
      nis_result *res;
      char buf[1024 + pwdtablelen];

      snprintf(buf, sizeof (buf), "[uid=%d],%s", uid, pwdtable);
      res = nis_list(buf, FOLLOW_PATH | FOLLOW_LINKS, NULL, NULL);
      if (niserr2nss (res->status) != NSS_STATUS_SUCCESS)
        {
          enum nss_status status =  niserr2nss (res->status);

          nis_freeresult (res);
          return status;
        }
      if ((parse_res = _nss_nisplus_parse_pwent (res, result, buffer,
                                                 buflen, errnop)) == -1)
        {
          nis_freeresult (res);
          *errnop = ERANGE;
          return NSS_STATUS_TRYAGAIN;
        }
      nis_freeresult (res);
    }
  else /* Use NIS */
    {
      char buf[1024];
      char *domain, *outval, *ptr;
      int outvallen;

      if (yp_get_default_domain (&domain) != YPERR_SUCCESS)
        {
          *errnop = ENOENT;
          return NSS_STATUS_NOTFOUND;
        }

      sprintf (buf, "%d", uid);
      if (yp_match (domain, "passwd.byuid", buf, strlen (buf),
                    &outval, &outvallen)
          != YPERR_SUCCESS)
        {
          *errnop = ENOENT;
          return NSS_STATUS_NOTFOUND;
        }

      if (insert_passwd_adjunct (&outval, &outvallen, domain, errnop)
          != NSS_STATUS_SUCCESS)
        {
          free (outval);
          return NSS_STATUS_TRYAGAIN;
        }

      if (buflen < ((size_t) outvallen + 1))
        {
          free (outval);
          *errnop = ERANGE;
          return NSS_STATUS_TRYAGAIN;
        }

      ptr = strncpy (buffer, outval, buflen);
      free (outval);

      while (isspace (*ptr))
        ptr++;
      parse_res = _nss_files_parse_pwent (ptr, result, data, buflen, errnop);
      if (parse_res == -1)
        return NSS_STATUS_TRYAGAIN;
    }

  if (parse_res > 0)
    {
      copy_pwd_changes (result, &pwd, p, plen);
      give_pwd_free (&pwd);
      /* We found the entry.  */
      return NSS_STATUS_SUCCESS;
    }
  else
    {
      /* Give buffer the old len back */
      buflen += plen;
      give_pwd_free (&pwd);
    }
  return NSS_STATUS_RETURN;
}

/* Searches in /etc/passwd and the NIS/NIS+ map for a special user id */
static enum nss_status
internal_getpwuid_r (uid_t uid, struct passwd *result, ent_t *ent,
                     char *buffer, size_t buflen, int *errnop)
{
  struct parser_data *data = (void *) buffer;

  while (1)
    {
      fpos_t pos;
      char *p;
      int parse_res;

      do
        {
          fgetpos (ent->stream, &pos);
          buffer[buflen - 1] = '\xff';
          p = fgets (buffer, buflen, ent->stream);
          if (p == NULL && feof (ent->stream))
	    {
	      *errnop = ENOENT;
	      return NSS_STATUS_NOTFOUND;
	    }
          if (p == NULL || buffer[buflen - 1] != '\xff')
            {
              fsetpos (ent->stream, &pos);
              *errnop = ERANGE;
              return NSS_STATUS_TRYAGAIN;
            }

          /* Terminate the line for any case.  */
          buffer[buflen - 1] = '\0';

          /* Skip leading blanks.  */
          while (isspace (*p))
            ++p;
        }
      while (*p == '\0' || *p == '#' || /* Ignore empty and comment lines.  */
             /* Parse the line.  If it is invalid, loop to
                get the next line of the file to parse.  */
             !(parse_res = _nss_files_parse_pwent (p, result, data, buflen,
                                                   errnop)));

      if (parse_res == -1)
        {
          /* The parser ran out of space.  */
          fsetpos (ent->stream, &pos);
          *errnop = ERANGE;
          return NSS_STATUS_TRYAGAIN;
        }

      /* This is a real entry.  */
      if (result->pw_name[0] != '+' && result->pw_name[0] != '-')
        {
          if (result->pw_uid == uid)
            return NSS_STATUS_SUCCESS;
          else
            continue;
        }

      /* -@netgroup */
      if (result->pw_name[0] == '-' && result->pw_name[1] == '@'
          && result->pw_name[2] != '\0')
        {
	  char buf[strlen (result->pw_name)];
	  enum nss_status status;

	  strcpy (buf, &result->pw_name[2]);

	  status = getpwuid_plususer (uid, result, buffer, buflen, errnop);
	  if (status == NSS_STATUS_SUCCESS &&
	      innetgr (buf, NULL, result->pw_name, NULL))
	    {
	      *errnop = ENOENT;
	      return NSS_STATUS_NOTFOUND;
	    }
          continue;
        }

      /* +@netgroup */
      if (result->pw_name[0] == '+' && result->pw_name[1] == '@'
          && result->pw_name[2] != '\0')
        {
	  char buf[strlen (result->pw_name)];
	  enum nss_status status;

	  strcpy (buf, &result->pw_name[2]);

	  status = getpwuid_plususer (uid, result, buffer, buflen, errnop);

	  if (status == NSS_STATUS_RETURN)
	    continue;

	  if (status == NSS_STATUS_SUCCESS)
	    {
	      if (innetgr (buf, NULL, result->pw_name, NULL))
		return NSS_STATUS_SUCCESS;
	    }
	  else
            if (status == NSS_STATUS_RETURN) /* We couldn't parse the entry */
	      {
		*errnop = ENOENT;
		return NSS_STATUS_NOTFOUND;
	      }
            else
              return status;

          continue;
	}

      /* -user */
      if (result->pw_name[0] == '-' && result->pw_name[1] != '\0'
          && result->pw_name[1] != '@')
        {
	  char buf[strlen (result->pw_name)];
	  enum nss_status status;

	  strcpy (buf, &result->pw_name[1]);

	  status = getpwuid_plususer (uid, result, buffer, buflen, errnop);
	  if (status == NSS_STATUS_SUCCESS &&
	      innetgr (buf, NULL, result->pw_name, NULL))
	    {
	      *errnop = ENOENT;
	      return NSS_STATUS_NOTFOUND;
	    }
          continue;
        }

      /* +user */
      if (result->pw_name[0] == '+' && result->pw_name[1] != '\0'
          && result->pw_name[1] != '@')
        {
	  char buf[strlen (result->pw_name)];
	  enum nss_status status;

	  strcpy (buf, &result->pw_name[1]);

	  status = getpwuid_plususer (uid, result, buffer, buflen, errnop);

	  if (status == NSS_STATUS_RETURN)
	    continue;

	  if (status == NSS_STATUS_SUCCESS)
	    {
	      if (strcmp (buf, result->pw_name) == 0)
		return NSS_STATUS_SUCCESS;
	    }
	  else
            if (status == NSS_STATUS_RETURN) /* We couldn't parse the entry */
	      {
		*errnop = ENOENT;
		return NSS_STATUS_NOTFOUND;
	      }
            else
              return status;

          continue;
        }

      /* +:... */
      if (result->pw_name[0] == '+' && result->pw_name[1] == '\0')
        {
          enum nss_status status;

          status = getpwuid_plususer (uid, result, buffer, buflen, errnop);
          if (status == NSS_STATUS_SUCCESS) /* We found the entry. */
            break;
          else
            if (status == NSS_STATUS_RETURN) /* We couldn't parse the entry */
	      {
		*errnop = ENOENT;
		return NSS_STATUS_NOTFOUND;
	      }
            else
              return status;
        }
    }
  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_compat_getpwuid_r (uid_t uid, struct passwd *pwd,
                        char *buffer, size_t buflen, int *errnop)
{
  ent_t ent = {0, 0, 0, NULL, 0, NULL, NULL, {NULL, 0, 0},
               {NULL, NULL, 0, 0, NULL, NULL, NULL}};
  enum nss_status status;

  __libc_lock_lock (lock);

  if (ni == NULL)
    {
      __nss_database_lookup ("passwd_compat", NULL, "nis", &ni);
      use_nisplus = (strcmp (ni->name, "nisplus") == 0);
    }

  __libc_lock_unlock (lock);

  status = internal_setpwent (&ent);
  if (status != NSS_STATUS_SUCCESS)
    return status;

  status = internal_getpwuid_r (uid, pwd, &ent, buffer, buflen, errnop);

  internal_endpwent (&ent);

  return status;
}


/* Support routines for remembering -@netgroup and -user entries.
   The names are stored in a single string with `|' as separator. */
static void
blacklist_store_name (const char *name, ent_t *ent)
{
  int namelen = strlen (name);
  char *tmp;

  /* first call, setup cache */
  if (ent->blacklist.size == 0)
    {
      ent->blacklist.size = MAX (BLACKLIST_INITIAL_SIZE, 2 * namelen);
      ent->blacklist.data = malloc (ent->blacklist.size);
      if (ent->blacklist.data == NULL)
	return;
      ent->blacklist.data[0] = '|';
      ent->blacklist.data[1] = '\0';
      ent->blacklist.current = 1;
    }
  else
    {
      if (in_blacklist (name, namelen, ent))
	return;			/* no duplicates */

      if (ent->blacklist.current + namelen + 1 >= ent->blacklist.size)
	{
	  ent->blacklist.size += MAX (BLACKLIST_INCREMENT, 2 * namelen);
	  tmp = realloc (ent->blacklist.data, ent->blacklist.size);
	  if (tmp == NULL)
	    {
	      free (ent->blacklist.data);
	      ent->blacklist.size = 0;
	      return;
	    }
	  ent->blacklist.data = tmp;
	}
    }

  tmp = stpcpy (ent->blacklist.data + ent->blacklist.current, name);
  *tmp++ = '|';
  *tmp = '\0';
  ent->blacklist.current += namelen + 1;

  return;
}

/* returns TRUE if ent->blacklist contains name, else FALSE */
static bool_t
in_blacklist (const char *name, int namelen, ent_t *ent)
{
  char buf[namelen + 3];
  char *cp;

  if (ent->blacklist.data == NULL)
    return FALSE;

  buf[0] = '|';
  cp = stpcpy (&buf[1], name);
  *cp++= '|';
  *cp = '\0';
  return strstr (ent->blacklist.data, buf) != NULL;
}
