/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>
   and Paul Janzen <pcj@primenet.com>, 1996.

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

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utmp.h>
#include <sys/file.h>
#include <sys/stat.h>

#include "utmp-private.h"


/* This is the default name.  */
static const char default_file_name[] = _PATH_UTMP;

/* Current file name.  */
static const char *file_name = (const char *) default_file_name;

/* Descriptor for the file and position.  */
static int file_fd = INT_MIN;
static off_t file_offset;

static struct utmp last_entry;

/* Functions defined here.  */
static int setutent_file (int reset);
static int getutent_r_file (struct utmp *buffer, struct utmp **result);
static int getutid_r_file (const struct utmp *key, struct utmp *buffer,
			   struct utmp **result);
static int getutline_r_file (const struct utmp *key, struct utmp *buffer,
			     struct utmp **result);
static struct utmp *pututline_file (const struct utmp *data);
static void endutent_file (void);
static int utmpname_file (const char *name);


/* Jump table for file functions.  */
struct utfuncs __libc_utmp_file_functions =
{
  setutent_file,
  getutent_r_file,
  getutid_r_file,
  getutline_r_file,
  pututline_file,
  endutent_file,
  utmpname_file
};


static int
setutent_file (int reset)
{
  if (file_fd == INT_MIN)
    {
      file_fd = open (file_name, O_RDWR);
      if (file_fd == -1)
	{
	  /* Hhm, read-write access did not work.  Try read-only.  */
	  file_fd = open (file_name, O_RDONLY);
	  if (file_fd == -1)
	    {
	      perror (_("while opening UTMP file"));
	      return 0;
	    }
	}
      file_offset = 0;

#if _HAVE_UT_TYPE - 0
      /* Make sure the entry won't match.  */
      last_entry.ut_type = -1;
#endif
    }
  else if (reset)
    {
      lseek (file_fd, 0, SEEK_SET);

      /* Remember we are at beginning of file.  */
      file_offset = 0;

#if _HAVE_UT_TYPE - 0
      /* Make sure the entry won't match.  */
      last_entry.ut_type = -1;
#endif
    }

  return 1;
}


static void
endutent_file (void)
{
  if (file_fd >= 0)
    close (file_fd);

  file_fd = INT_MIN;
}


static int
getutent_r_file (struct utmp *buffer, struct utmp **result)
{
  int nbytes;
  struct flock fl;			/* Information struct for locking.  */

  /* Open utmp file if not already done.  */
  if (file_fd == INT_MIN)
    setutent_file (1);

  if (file_fd == -1 || file_offset == -1l)
    {
      /* Not available.  */
      *result = NULL;
      return -1;
    }

  /* XXX The following is not perfect.  Instead of locking the file itself
     Marek Michalkiewicz <marekm@i17linuxb.ists.pwr.wroc.pl> suggests to
     use an extra locking file.  */

  /* Try to get the lock.  */
  memset (&fl, '\0', sizeof (struct flock));
  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fcntl (file_fd, F_SETLKW, &fl);

  /* Read the next entry.  */
  nbytes = read (file_fd, &last_entry, sizeof (struct utmp));

  /* And unlock the file.  */
  fl.l_type = F_UNLCK;
  fcntl (file_fd, F_SETLKW, &fl);

  if (nbytes != sizeof (struct utmp))
    {
      file_offset = -1l;
      *result = NULL;
      return -1;
    }

  /* Update position pointer.  */
  file_offset += sizeof (struct utmp);

  memcpy (buffer, &last_entry, sizeof (struct utmp));
  *result = buffer;

  return 0;
}


/* For implementing this function we don't use the getutent_r function
   because we can avoid the reposition on every new entry this way.  */
static int
getutline_r_file (const struct utmp *line, struct utmp *buffer,
		  struct utmp **result)
{
  if (file_fd < 0 || file_offset == -1l)
    {
      *result = NULL;
      return -1;
    }

  while (1)
    {
      /* Read the next entry.  */
      if (read (file_fd, &last_entry, sizeof (struct utmp))
	  != sizeof (struct utmp))
	{
	  __set_errno (ESRCH);
	  file_offset = -1l;
	  *result = NULL;
	  return -1;
	}
      file_offset += sizeof (struct utmp);

      /* Stop if we found a user or login entry.  */
      if (
#if _HAVE_UT_TYPE - 0
	  (last_entry.ut_type == USER_PROCESS
	   || last_entry.ut_type == LOGIN_PROCESS)
	  &&
#endif
	  !strncmp (line->ut_line, last_entry.ut_line, sizeof line->ut_line))
	break;
    }

  memcpy (buffer, &last_entry, sizeof (struct utmp));
  *result = buffer;

  return 0;
}


static int
proc_utmp_eq (const struct utmp *entry, const struct utmp *match)
{
  return
    (
#if _HAVE_UT_TYPE - 0
     (entry->ut_type == INIT_PROCESS
      || entry->ut_type == LOGIN_PROCESS
      || entry->ut_type == USER_PROCESS
      || entry->ut_type == DEAD_PROCESS)
     &&
     (match->ut_type == INIT_PROCESS
      || match->ut_type == LOGIN_PROCESS
      || match->ut_type == USER_PROCESS
      || match->ut_type == DEAD_PROCESS)
     &&
#endif
#if _HAVE_UT_ID - 0
     (entry->ut_id[0] && match->ut_id[0]
      ? strncmp (entry->ut_id, match->ut_id, sizeof match->ut_id) == 0
      : strncmp (entry->ut_line, match->ut_line, sizeof match->ut_line) == 0)
#else
     strncmp (entry->ut_line, match->ut_line, sizeof match->ut_line) == 0
#endif
     );
}

static int
internal_getut_r (const struct utmp *id, struct utmp *buffer)
{
#if _HAVE_UT_TYPE - 0
  if (id->ut_type == RUN_LVL || id->ut_type == BOOT_TIME
      || id->ut_type == OLD_TIME || id->ut_type == NEW_TIME)
    {
      /* Search for next entry with type RUN_LVL, BOOT_TIME,
	 OLD_TIME, or NEW_TIME.  */

      while (1)
	{
	  /* Read the next entry.  */
	  if (read (file_fd, buffer, sizeof (struct utmp))
	      != sizeof (struct utmp))
	    {
	      __set_errno (ESRCH);
	      file_offset = -1l;
	      return -1;
	    }
	  file_offset += sizeof (struct utmp);

	  if (id->ut_type == buffer->ut_type)
	    break;
	}
    }
  else
#endif /* _HAVE_UT_TYPE */
    {
      /* Search for the next entry with the specified ID and with type
	 INIT_PROCESS, LOGIN_PROCESS, USER_PROCESS, or DEAD_PROCESS.  */

      while (1)
	{
	  /* Read the next entry.  */
	  if (read (file_fd, buffer, sizeof (struct utmp))
	      != sizeof (struct utmp))
	    {
	      __set_errno (ESRCH);
	      file_offset = -1l;
	      return -1;
	    }
	  file_offset += sizeof (struct utmp);

	  if (proc_utmp_eq (buffer, id))
	    break;
	}
    }

  return 0;
}


/* For implementing this function we don't use the getutent_r function
   because we can avoid the reposition on every new entry this way.  */
static int
getutid_r_file (const struct utmp *id, struct utmp *buffer,
		struct utmp **result)
{
  if (file_fd < 0 || file_offset == -1l)
    {
      *result = NULL;
      return -1;
    }

  if (internal_getut_r (id, &last_entry) < 0)
    {
      *result = NULL;
      return -1;
    }

  memcpy (buffer, &last_entry, sizeof (struct utmp));
  *result = buffer;

  return 0;
}


static struct utmp *
pututline_file (const struct utmp *data)
{
  struct flock fl;			/* Information struct for locking.  */
  struct utmp buffer;
  struct utmp *pbuf;
  int found;

  if (file_fd < 0)
    /* Something went wrong.  */
    return NULL;

  if (file_fd == INT_MIN)
    /* The file is closed.  Open it again.  */
    setutent_file (0);

  /* Find the correct place to insert the data.  */
  if (file_offset > 0
      && (
#if _HAVE_UT_TYPE - 0
	  (last_entry.ut_type == data->ut_type
	   && (last_entry.ut_type == RUN_LVL
	       || last_entry.ut_type == BOOT_TIME
	       || last_entry.ut_type == OLD_TIME
	       || last_entry.ut_type == NEW_TIME))
	  ||
#endif
	  proc_utmp_eq (&last_entry, data)))
    found = 1;
  else
    found = internal_getut_r (data, &buffer);

  /* Try to lock the file.  */
  memset (&fl, '\0', sizeof (struct flock));
  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fcntl (file_fd, F_SETLKW, &fl);

  if (found < 0)
    {
      /* We append the next entry.  */
      file_offset = lseek (file_fd, 0, SEEK_END);
      if (file_offset % sizeof (struct utmp) != 0)
	{
	  file_offset -= file_offset % sizeof (struct utmp);
	  ftruncate (file_fd, file_offset);

	  if (lseek (file_fd, 0, SEEK_END) < 0)
	    {
	      pbuf = NULL;
	      goto unlock_return;
	    }
	}
    }
  else
    {
      /* We replace the just read entry.  */
      file_offset -= sizeof (struct utmp);
      lseek (file_fd, file_offset, SEEK_SET);
    }

  /* Write the new data.  */
  if (write (file_fd, data, sizeof (struct utmp)) != sizeof (struct utmp)
      /* If we appended a new record this is only partially written.
	 Remove it.  */
      && found < 0)
    {
      (void) ftruncate (file_fd, file_offset);
      pbuf = NULL;
    }
  else
    {
      file_offset += sizeof (struct utmp);
      pbuf = (struct utmp *) data;
    }

 unlock_return:
   /* And unlock the file.  */
  fl.l_type = F_UNLCK;
  fcntl (file_fd, F_SETLKW, &fl);

  return pbuf;
}


static int
utmpname_file (const char *name)
{
  if (strcmp (name, file_name) != 0)
    {
      if (strcmp (name, default_file_name) == 0)
	{
	  if (file_name != default_file_name)
	    free ((char *) file_name);

	  file_name = default_file_name;
	}
      else
	{
	  char *new_name = __strdup (name);
	  if (new_name == NULL)
	    /* Out of memory.  */
	    return -1;

	  if (file_name != default_file_name)
	    free ((char *) file_name);

	  file_name = new_name;
	}
    }
  return 0;
}
