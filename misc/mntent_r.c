/* Utilities for reading/writing fstab, mtab, etc.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <mntent.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

/* Prepare to begin reading and/or writing mount table entries from the
   beginning of FILE.  MODE is as for `fopen'.  */
FILE *
__setmntent (const char *file, const char *mode)
{
  return fopen (file, mode);
}
weak_alias (__setmntent, setmntent)


/* Close a stream opened with `setmntent'.  */
int
__endmntent (FILE *stream)
{
  if (stream)		/* SunOS 4.x allows for NULL stream */
    fclose (stream);
  return 1;		/* SunOS 4.x says to always return 1 */
}
weak_alias (__endmntent, endmntent)


/* Read one mount table entry from STREAM.  Returns a pointer to storage
   reused on the next call, or null for EOF or error (use feof/ferror to
   check).  */
struct mntent *
__getmntent_r (FILE *stream, struct mntent *mp, char *buffer, int bufsiz)
{
  char *head;

  flockfile (stream);
  do
    {
      char *end_ptr;

      if (fgets_unlocked (buffer, bufsiz, stream) == NULL)
	{
	  funlockfile (stream);
	  return NULL;
	}

      end_ptr = strchr (buffer, '\n');
      if (end_ptr != NULL)	/* chop newline */
	*end_ptr = '\0';
      else
	{
	  /* Not the whole line was read.  Do it now but forget it.  */
	  char tmp[1024];
	  while (fgets_unlocked (tmp, sizeof tmp, stream) != NULL)
	    if (strchr (tmp, '\n') != NULL)
	      break;
	}

      head = buffer + strspn (buffer, " \t");
      /* skip empty lines and comment lines:  */
    } while (head[0] == '\0' || head[0] == '#');

  mp->mnt_fsname = __strsep (&head, " \t") ?: (char *) "";
  if (head)
    head += strspn (head, " \t");
  mp->mnt_dir = __strsep (&head, " \t") ?: (char *) "";
  if (head)
    head += strspn (head, " \t");
  mp->mnt_type = __strsep (&head, " \t") ?: (char *) "";
  if (head)
    head += strspn (head, " \t");
  mp->mnt_opts = __strsep (&head, " \t") ?: (char *) "";
  switch (head ? sscanf (head, " %d %d ", &mp->mnt_freq, &mp->mnt_passno) : 0)
    {
    case 0:
      mp->mnt_freq = 0;
    case 1:
      mp->mnt_passno = 0;
    case 2:
    }
  funlockfile (stream);

  return mp;
}
weak_alias (__getmntent_r, getmntent_r)

/* Write the mount table entry described by MNT to STREAM.
   Return zero on success, nonzero on failure.  */
int
__addmntent (FILE *stream, const struct mntent *mnt)
{
  if (fseek (stream, 0, SEEK_END))
    return 1;

  return (fprintf (stream, "%s %s %s %s %d %d\n",
		   mnt->mnt_fsname,
		   mnt->mnt_dir,
		   mnt->mnt_type,
		   mnt->mnt_opts,
		   mnt->mnt_freq,
		   mnt->mnt_passno)
	  < 0 ? 1 : 0);
}
weak_alias (__addmntent, addmntent)


/* Search MNT->mnt_opts for an option matching OPT.
   Returns the address of the substring, or null if none found.  */
char *
__hasmntopt (const struct mntent *mnt, const char *opt)
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
      if (rest == NULL)
	break;
      ++rest;
    }

  return NULL;
}
weak_alias (__hasmntopt, hasmntopt)
