/* Utilities for reading/writing fstab, mtab, etc.
Copyright (C) 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <mntent.h>
#include <stdio.h>
#include <string.h>

/* Prepare to begin reading and/or writing mount table entries from the
   beginning of FILE.  MODE is as for `fopen'.  */
FILE *
setmntent (const char *file, const char *mode)
{
  return fopen (file, mode);
}

/* Close a stream opened with `setmntent'.  */
int
endmntent (FILE *stream)
{
  return fclose (stream);
}


/* Read one mount table entry from STREAM.  Returns a pointer to storage
   reused on the next call, or null for EOF or error (use feof/ferror to
   check).  */
struct mntent *
getmntent (FILE *stream)
{
  static char *buf;
  static size_t bufsiz;
  static struct mntent m;
  char *head;

  do
    {
      if (getline (&buf, &bufsiz, stream) < 0)
	return NULL;

      head = buf;
    } while (head[0] == '#');	/* Skip comment lines.  */
    
  m.mnt_fsname = strsep (&head, " \t") ?: (char *) "";
  m.mnt_dir = strsep (&head, " \t") ?: (char *) "";
  m.mnt_type = strsep (&head, " \t") ?: (char *) "";
  m.mnt_opts = strsep (&head, " \t") ?: (char *) "";
  switch (sscanf (head, "%d %d\n", &m.mnt_freq, &m.mnt_passno))
    {
    case 0:
      m.mnt_freq = 0;
    case 1:
      m.mnt_passno = 0;
    case 2:
    }

  return &m;
}

/* Write the mount table entry described by MNT to STREAM.
   Return zero on success, nonzero on failure.  */
int
addmntent (FILE *stream, const struct mntent *mnt)
{
  return (fprintf (stream, "%s %s %s %s %d %d\n",
		   mnt->mnt_fsname,
		   mnt->mnt_dir,
		   mnt->mnt_type,
		   mnt->mnt_opts,
		   mnt->mnt_freq,
		   mnt->mnt_passno)
	  < 0 ? -1 : 0);
}

/* Search MNT->mnt_opts for an option matching OPT.
   Returns the address of the substring, or null if none found.  */
char *
hasmntopt (const struct mntent *mnt, const char *opt)
{
  const size_t optlen = strlen (opt);
  char *rest = mnt->mnt_opts, *p;

  while ((p = strstr (rest, opt)) != NULL)
    {
      if (p == rest || p[-1] == ',' &&
	  (p[optlen] == '\0' ||
	   p[optlen] == '=' ||
	   p[optlen] == ','))
	return p;

      rest = strchr (rest, ',');
    }

  return NULL;
}

