/* Copyright (C) 1996, 2001, 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <ctype.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <rpcsvc/ypclnt.h>

#include "nss-nis.h"
#include "nsswitch.h"


/* Convert YP error number to NSS error number.  */
const enum nss_status __yperr2nss_tab[] =
{
  [YPERR_SUCCESS] = NSS_STATUS_SUCCESS,
  [YPERR_BADARGS] = NSS_STATUS_UNAVAIL,
  [YPERR_RPC]     = NSS_STATUS_UNAVAIL,
  [YPERR_DOMAIN]  = NSS_STATUS_UNAVAIL,
  [YPERR_MAP]     = NSS_STATUS_UNAVAIL,
  [YPERR_KEY]     = NSS_STATUS_NOTFOUND,
  [YPERR_YPERR]   = NSS_STATUS_UNAVAIL,
  [YPERR_RESRC]   = NSS_STATUS_TRYAGAIN,
  [YPERR_NOMORE]  = NSS_STATUS_NOTFOUND,
  [YPERR_PMAP]    = NSS_STATUS_UNAVAIL,
  [YPERR_YPBIND]  = NSS_STATUS_UNAVAIL,
  [YPERR_YPSERV]  = NSS_STATUS_UNAVAIL,
  [YPERR_NODOM]   = NSS_STATUS_UNAVAIL,
  [YPERR_BADDB]   = NSS_STATUS_UNAVAIL,
  [YPERR_VERS]    = NSS_STATUS_UNAVAIL,
  [YPERR_ACCESS]  = NSS_STATUS_UNAVAIL,
  [YPERR_BUSY]    = NSS_STATUS_TRYAGAIN
};
const unsigned int __yperr2nss_count = (sizeof (__yperr2nss_tab)
				        / sizeof (__yperr2nss_tab[0]));

int _nis_default_nss_flags;

static const char default_nss[] = "/etc/default/nss";

int
_nis_check_default_nss (void)
{
  FILE *fp = fopen (default_nss, "rc");
  int flags = NSS_FLAG_SET;
  if (fp != NULL)
    {
      char *line = NULL;
      size_t linelen = 0;

      __fsetlocking (fp, FSETLOCKING_BYCALLER);

      while (!feof_unlocked (fp))
	{
	  ssize_t n = getline (&line, &linelen, fp);
	  if (n <= 0)
	    break;

	  /* There currently are only two variables we expect, so
	     simplify the parsing.  Recognize only

	       NETID_AUTHORITATIVE = TRUE
	       SERVICES_AUTHORITATIVE = TRUE

	     with arbitrary white spaces.  */
	  char *cp = line;
	  while (isspace (*cp))
	    ++cp;

	  /* Recognize comment lines.  */
	  if (*cp == '#')
	    continue;

	  static const char netid_authoritative[] = "NETID_AUTHORITATIVE";
	  static const char services_authoritative[]
	    = "SERVICES_AUTHORITATIVE";
	  size_t flag_len;
	  if (strncmp (cp, netid_authoritative,
		       flag_len = sizeof (netid_authoritative) - 1) != 0
	      && strncmp (cp, services_authoritative,
			  flag_len = sizeof (services_authoritative) - 1)
		 != 0)
	    continue;

	  cp += flag_len;
	  while (isspace (*cp))
	    ++cp;
	  if (*cp++ != '=')
	    continue;
	  while (isspace (*cp))
	    ++cp;

	  if (strncmp (cp, "TRUE", 4) != 0)
	    continue;
	  cp += 4;

	  while (isspace (*cp))
	    ++cp;

	  if (*cp == '\0')
	    flags |= flag_len == sizeof (netid_authoritative) - 1
		     ? NSS_FLAG_NETID_AUTHORITATIVE
		     : NSS_FLAG_SERVICES_AUTHORITATIVE;
	}

      free (line);

      fclose (fp);
    }

  _nis_default_nss_flags = flags;
  return flags;
}
