/* Public key file parser in nss_files module.
   Copyright (C) 1996-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <rpc/key_prot.h>
#include <rpc/des_crypt.h>
#include "nsswitch.h"

#define DATAFILE "/etc/publickey"


static enum nss_status
search (const char *netname, char *result, int *errnop, int secret)
{
  FILE *stream = fopen (DATAFILE, "rce");
  if (stream == NULL)
    return errno == EAGAIN ? NSS_STATUS_TRYAGAIN : NSS_STATUS_UNAVAIL;

  for (;;)
    {
      char buffer[HEXKEYBYTES * 2 + KEYCHECKSUMSIZE + MAXNETNAMELEN + 17];
      char *p;
      char *save_ptr;

      buffer[sizeof (buffer) - 1] = '\xff';
      p = fgets_unlocked (buffer, sizeof (buffer), stream);
      if (p == NULL)
	{
	  /* End of file or read error.  */
	  *errnop = errno;
	  fclose (stream);
	  return NSS_STATUS_NOTFOUND;
	}
      else if (buffer[sizeof (buffer) - 1] != '\xff')
	{
	  /* Invalid line in file?  Skip remainder of line.  */
	  if (buffer[sizeof (buffer) - 2] != '\0')
	    while (getc_unlocked (stream) != '\n')
	      continue;
	  continue;
	}

      /* Parse line.  */
      p = __strtok_r (buffer, "# \t:\n", &save_ptr);
      if (p == NULL) /* Skip empty and comment lines.  */
	continue;
      if (strcmp (p, netname) != 0)
	continue;

      /* A hit!  Find the field we want and return.  */
      p = __strtok_r (NULL, ":\n", &save_ptr);
      if (p == NULL)  /* malformed line? */
	continue;
      if (secret)
	p = __strtok_r (NULL, ":\n", &save_ptr);
      if (p == NULL)  /* malformed line? */
	continue;
      fclose (stream);
      strcpy (result, p);
      return NSS_STATUS_SUCCESS;
    }
}

enum nss_status
_nss_files_getpublickey (const char *netname, char *pkey, int *errnop)
{
  return search (netname, pkey, errnop, 0);
}

enum nss_status
_nss_files_getsecretkey (const char *netname, char *skey, char *passwd,
			 int *errnop)
{
  enum nss_status status;
  char buf[HEXKEYBYTES + KEYCHECKSUMSIZE + 16];

  skey[0] = 0;

  status = search (netname, buf, errnop, 1);
  if (status != NSS_STATUS_SUCCESS)
    return status;

  if (!xdecrypt (buf, passwd))
    return NSS_STATUS_SUCCESS;

  if (memcmp (buf, &(buf[HEXKEYBYTES]), KEYCHECKSUMSIZE) != 0)
    return NSS_STATUS_SUCCESS;

  buf[HEXKEYBYTES] = 0;
  strcpy (skey, buf);

  return NSS_STATUS_SUCCESS;
}
