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
#include <nss.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Get the declaration of the parser function.  */
#define ENTNAME pwent
#define STRUCTURE passwd
#define EXTERN_PARSER
#include <nss/nss_files/files-parse.c>

/* Locks the static variables in this file.  */
__libc_lock_define_initialized (static, lock);

static void *context = NULL;

static enum nss_status
internal_setpwent (void)
{
  if (!context)
    {
      if (hesiod_init (&context) == -1)
	return NSS_STATUS_UNAVAIL;
    }

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_hesiod_setpwent (void)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = internal_setpwent ();

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_hesiod_endpwent (void)
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
lookup (const char *name, const char *type, struct passwd *pwd,
	char *buffer, size_t buflen, int *errnop)
{
  enum nss_status status;
  struct parser_data *data = (void *) buffer;
  size_t linebuflen;
  char **list;
  int parse_res;
  size_t len;

  status = internal_setpwent ();
  if (status != NSS_STATUS_SUCCESS)
    return status;

  list = hesiod_resolve (context, name, type);
  if (list == NULL)
    return errno == ENOENT ? NSS_STATUS_NOTFOUND : NSS_STATUS_UNAVAIL;

  linebuflen = buffer + buflen - data->linebuffer;
  len = strlen (*list) + 1;
  if (linebuflen < len)
    {
      hesiod_free_list (context, list);
      *errnop = ERANGE;
      return NSS_STATUS_TRYAGAIN;
    }

  memcpy (data->linebuffer, *list, len);
  hesiod_free_list (context, list);

  parse_res = _nss_files_parse_pwent (buffer, pwd, data, buflen, errnop);
  if (parse_res < 1)
    return parse_res == -1 ? NSS_STATUS_TRYAGAIN : NSS_STATUS_NOTFOUND;

  return NSS_STATUS_SUCCESS;
}

enum nss_status
_nss_hesiod_getpwnam_r (const char *name, struct passwd *pwd,
			char *buffer, size_t buflen, int *errnop)
{
  enum nss_status status;

  __libc_lock_lock (lock);

  status = lookup (name, "passwd", pwd, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}

enum nss_status
_nss_hesiod_getpwuid_r (uid_t uid, struct passwd *pwd,
			char *buffer, size_t buflen, int *errnop)
{
  enum nss_status status = NSS_STATUS_UNAVAIL;
  char uidstr[21];	/* We will probably never have a gid_t with more
			   than 64 bits.  */

  snprintf (uidstr, sizeof uidstr, "%d", uid);

  __libc_lock_lock (lock);

  status = lookup (uidstr, "uid", pwd, buffer, buflen, errnop);

  __libc_lock_unlock (lock);

  return status;
}
