/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Mark Kettenis <kettenis@phys.uva.nl>, 1997.

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

#include <bits/libc-lock.h>
#include <errno.h>
#include <hesiod.h>
#include <netdb.h>
#include <netinet/in.h>
#include <nss.h>
#include <stdlib.h>
#include <string.h>


/* Hesiod uses a format for service entries that differs from the
   traditional format.  We therefore declare our own parser.  */

#define ENTNAME servent

#define ENTDATA servent_data
struct servent_data {};

#define TRAILING_LIST_MEMBER		s_aliases
#define TRAILING_LIST_SEPARATOR_P	isspace
#include <nss/nss_files/files-parse.c>
#define ISSEMICOLON(c)	((c) ==  ';')
LINE_PARSER
("",
 (void) entdata;
 STRING_FIELD (result->s_name, ISSEMICOLON, 1);
 STRING_FIELD (result->s_proto, ISSEMICOLON, 1);
 INT_FIELD (result->s_port, ISSEMICOLON, 10, 0, htons);
 )


/* Locks the static variables in this file.  */
__libc_lock_define_initialized (static, lock);

static void *context = NULL;

static enum nss_status
internal_setservent (void)
{
  if (!context)
    {
      if (hesiod_init (&context) == -1)
	return NSS_STATUS_UNAVAIL;
    }

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_hesiod_setservent (void)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_setservent ();

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_hesiod_endservent (void)
{
  __libc_lock_lock (lock);

  if (context)
    {
      hesiod_end (context);
      context = NULL;
    }

  __libc_lock_unlock (lock);

  return NSS_STATUS_SUCCESS;
}

static enum nss_status
lookup (const char *name, const char *protocol, struct servent *serv,
	char *buffer, size_t buflen, int *errnop)
{
  enum nss_status status;
  struct parser_data *data = (void *) buffer;
  size_t linebuflen;
  char **list, **item;
  int parse_res;
  int found;

  status = internal_setservent ();
  if (status != NSS_STATUS_SUCCESS)
    return status;

  list = hesiod_resolve (context, name, "service");
  if (list == NULL)
    return errno == ENOENT ? NSS_STATUS_NOTFOUND : NSS_STATUS_UNAVAIL;

  linebuflen = buffer + buflen - data->linebuffer;

  item = list;
  found = 0;
  do
    {
      size_t len = strlen (*item) + 1;

      if (linebuflen < len)
	{
	  hesiod_free_list (context, list);
	  *errnop = ERANGE;
	  return NSS_STATUS_TRYAGAIN;
	}

      memcpy (data->linebuffer, *item, len);

      parse_res = parse_line (buffer, serv, data, buflen, errnop);
      if (parse_res == -1)
	{
	  hesiod_free_list (context, list);
	  return NSS_STATUS_TRYAGAIN;
	}

      if (parse_res > 0)
	found = protocol == NULL || strcmp (serv->s_proto, protocol) == 0;

      ++item;
    }
  while (*item != NULL && !found);

  hesiod_free_list (context, list);

  return found ? NSS_STATUS_SUCCESS : NSS_STATUS_NOTFOUND;
}

enum nss_status
_nss_hesiod_getservbyname_r (const char *name, const char *protocol,
			     struct servent *serv,
			     char *buffer, size_t buflen, int *errnop)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = lookup (name, protocol, serv, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}
