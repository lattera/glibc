/* Copyright (c) 1997, 1998, 1999, 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <libintl.h>
#include <rpcsvc/nis.h>


static const char *nis_errlist[] =
{
  N_("Success"),
  N_("Probable success"),
  N_("Not found"),
  N_("Probably not found"),
  N_("Cache expired"),
  N_("NIS+ servers unreachable"),
  N_("Unknown object"),
  N_("Server busy, try again"),
  N_("Generic system error"),
  N_("First/next chain broken"),
  N_("Permission denied"),
  N_("Not owner"),
  N_("Name not served by this server"),
  N_("Server out of memory"),
  N_("Object with same name exists"),
  N_("Not master server for this domain"),
  N_("Invalid object for operation"),
  N_("Malformed name, or illegal name"),
  N_("Unable to create callback"),
  N_("Results sent to callback proc"),
  N_("Not found, no such name"),
  N_("Name/entry isn't unique"),
  N_("Modification failed"),
  N_("Database for table does not exist"),
  N_("Entry/table type mismatch"),
  N_("Link points to illegal name"),
  N_("Partial success"),
  N_("Too many attributes"),
  N_("Error in RPC subsystem"),
  N_("Missing or malformed attribute"),
  N_("Named object is not searchable"),
  N_("Error while talking to callback proc"),
  N_("Non NIS+ namespace encountered"),
  N_("Illegal object type for operation"),
  N_("Passed object is not the same object on server"),
  N_("Modify operation failed"),
  N_("Query illegal for named table"),
  N_("Attempt to remove a non-empty table"),
  N_("Error in accessing NIS+ cold start file.  Is NIS+ installed?"),
  N_("Full resync required for directory"),
  N_("NIS+ operation failed"),
  N_("NIS+ service is unavailable or not installed"),
  N_("Yes, 42 is the meaning of life"),
  N_("Unable to authenticate NIS+ server"),
  N_("Unable to authenticate NIS+ client"),
  N_("No file space on server"),
  N_("Unable to create process on server"),
  N_("Master server busy, full dump rescheduled.")
};

const char *
nis_sperrno (const nis_error status)
{
  if (status >= (sizeof (nis_errlist) / sizeof (nis_errlist[0])))
    return "???";
  else
    return gettext (nis_errlist[status]);
}
libnsl_hidden_def (nis_sperrno)

void
nis_perror (const nis_error status, const char *label)
{
  fprintf (stderr, "%s: %s\n", label, nis_sperrno (status));
}

void
nis_lerror (const nis_error status, const char *label)
{
  syslog (LOG_ERR, "%s: %s", label, nis_sperrno (status));
}

char *
nis_sperror_r (const nis_error status, const char *label,
	       char *buffer, size_t buflen)
{
  const char *cptr;

  cptr = nis_sperrno (status);

  if ((strlen (cptr) + strlen (label) + 3) > buflen)
    {
      errno = ERANGE;
      return NULL;
    }

    sprintf (buffer, "%s: %s", label, cptr);

    return buffer;
}
libnsl_hidden_def (nis_sperror_r)

char *
nis_sperror (const nis_error status, const char *label)
{
  static char buffer[NIS_MAXNAMELEN +1];

  return nis_sperror_r (status, label, buffer, sizeof (buffer));
}
