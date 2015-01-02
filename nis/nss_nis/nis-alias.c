/* Copyright (C) 1996-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <nss.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <aliases.h>
#include <bits/libc-lock.h>
#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

#include "nss-nis.h"

__libc_lock_define_initialized (static, lock)

static bool_t new_start = 1;
static char *oldkey;
static int oldkeylen;

static int
_nss_nis_parse_aliasent (const char *key, char *alias, struct aliasent *result,
			 char *buffer, size_t buflen, int *errnop)
{
  char *first_unused = buffer + strlen (alias) + 1;
  size_t room_left =
    buflen - (buflen % __alignof__ (char *)) - strlen (alias) - 2;
  char *line;
  char *cp;

  result->alias_members_len = 0;
  *first_unused = '\0';
  first_unused++;
  strcpy (first_unused, key);

  if (first_unused[room_left - 1] != '\0')
    {
      /* The line is too long for our buffer.  */
    no_more_room:
      *errnop = ERANGE;
      return -1;
    }

  result->alias_name = first_unused;

  /* Terminate the line for any case.  */
  cp = strpbrk (alias, "#\n");
  if (cp != NULL)
    *cp = '\0';

  first_unused += strlen (result->alias_name) + 1;
  /* Adjust the pointer so it is aligned for
     storing pointers.  */
  first_unused += __alignof__ (char *) - 1;
  first_unused -= ((first_unused - (char *) 0) % __alignof__ (char *));
  result->alias_members = (char **) first_unused;

  line = alias;

  while (*line != '\0')
    {
      /* Skip leading blanks.  */
      while (isspace (*line))
	line++;

      if (*line == '\0')
	break;

      if (room_left < sizeof (char *))
	  goto no_more_room;
      room_left -= sizeof (char *);
      result->alias_members[result->alias_members_len] = line;

      while (*line != '\0' && *line != ',')
	line++;

      if (line != result->alias_members[result->alias_members_len])
	{
	  *line = '\0';
	  line++;
	  result->alias_members_len++;
	}
    }
  return result->alias_members_len == 0 ? 0 : 1;
}

enum nss_status
_nss_nis_setaliasent (void)
{
  __libc_lock_lock (lock);

  new_start = 1;
  if (oldkey != NULL)
    {
      free (oldkey);
      oldkey = NULL;
      oldkeylen = 0;
    }

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}
/* The 'endaliasent' function is identical.  */
strong_alias (_nss_nis_setaliasent, _nss_nis_endaliasent)

static enum nss_status
internal_nis_getaliasent_r (struct aliasent *alias, char *buffer,
			    size_t buflen, int *errnop)
{
  char *domain;

  if (__glibc_unlikely (yp_get_default_domain (&domain)))
    return NSS_STATUS_UNAVAIL;

  alias->alias_local = 0;

  /* Get the next entry until we found a correct one. */
  int parse_res;
  do
    {
      char *result;
      int len;
      char *outkey;
      int keylen;
      int yperr;

      if (new_start)
	yperr = yp_first (domain, "mail.aliases", &outkey, &keylen, &result,
			  &len);
      else
	yperr = yp_next (domain, "mail.aliases", oldkey, oldkeylen, &outkey,
			 &keylen, &result, &len);

      if (__glibc_unlikely (yperr != YPERR_SUCCESS))
	{
	  enum nss_status retval = yperr2nss (yperr);

	  if (retval == NSS_STATUS_TRYAGAIN)
	    *errnop = errno;
	  return retval;
	}

      if (__glibc_unlikely ((size_t) (len + 1) > buflen))
	{
	  free (result);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}
      char *p = strncpy (buffer, result, len);
      buffer[len] = '\0';
      while (isspace (*p))
	++p;
      free (result);

      parse_res = _nss_nis_parse_aliasent (outkey, p, alias, buffer,
					   buflen, errnop);
      if (__glibc_unlikely (parse_res == -1))
	{
	  free (outkey);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      free (oldkey);
      oldkey = outkey;
      oldkeylen = keylen;
      new_start = 0;
    }
  while (!parse_res);

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_nis_getaliasent_r (struct aliasent *alias, char *buffer, size_t buflen,
			int *errnop)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_nis_getaliasent_r (alias, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_nis_getaliasbyname_r (const char *name, struct aliasent *alias,
			   char *buffer, size_t buflen, int *errnop)
{
  if (name == NULL)
    {
      *errnop = EINVAL;
      return NSS_STATUS_UNAVAIL;
    }

  char *domain;
  if (__glibc_unlikely (yp_get_default_domain (&domain)))
    return NSS_STATUS_UNAVAIL;

  size_t namlen = strlen (name);
  char *name2;
  int use_alloca = __libc_use_alloca (namlen + 1);
  if (use_alloca)
    name2 = __alloca (namlen + 1);
  else
    {
      name2 = malloc (namlen + 1);
      if (name2 == NULL)
	{
	  *errnop = ENOMEM;
	  return NSS_STATUS_TRYAGAIN;
	}
    }

  /* Convert name to lowercase.  */
  size_t i;
  for (i = 0; i < namlen; ++i)
    name2[i] = _tolower (name[i]);
  name2[i] = '\0';

  char *result;
  int len;
  int yperr = yp_match (domain, "mail.aliases", name2, namlen, &result, &len);

  if (!use_alloca)
    free (name2);

  if (__glibc_unlikely (yperr != YPERR_SUCCESS))
    {
      enum nss_status retval = yperr2nss (yperr);

      if (retval == NSS_STATUS_TRYAGAIN)
	*errnop = errno;
      return retval;
    }

  if (__glibc_unlikely ((size_t) (len + 1) > buflen))
    {
      free (result);
      *errnop = ERANGE;
      return NSS_STATUS_TRYAGAIN;
    }

  char *p = strncpy (buffer, result, len);
  buffer[len] = '\0';
  while (isspace (*p))
    ++p;
  free (result);

  alias->alias_local = 0;
  int parse_res = _nss_nis_parse_aliasent (name, p, alias, buffer, buflen,
					   errnop);
  if (__glibc_unlikely (parse_res < 1))
    {
      if (parse_res == -1)
	return NSS_STATUS_TRYAGAIN;
      else
	return NSS_STATUS_NOTFOUND;
    }

  return NSS_STATUS_SUCCESS;
}
