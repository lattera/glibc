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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <utmp.h>


#include "utmpd-private.h"
#include "xtmp.h"


/* Prototypes for the local functions.  */
static int initialize_database (utmp_database *database);
static int store_state_entry (utmp_database *database, int old_position,
			      const struct utmp *old_entry);
static int store_process_entry (utmp_database *database, int old_position,
				const struct utmp *old_entry);
static int replace_entry (utmp_database *database, int old_position,
			  int new_position, const struct utmp *entry);
static int store_entry (utmp_database *database, int position,
			const struct utmp *entry);
static int get_mtime (const char *file, time_t *timer);


/* Open the database specified by FILE and merge it with the
   contents of the old format file specified by OLD_FILE.  Returns a
   pointer to a newly allocated structure describing the database, or
   NULL on error.  */
utmp_database *
open_database (const char *file, const char *old_file)
{
  utmp_database *database;

  /* Allocate memory.  */
  database = (utmp_database *) malloc (sizeof (utmp_database));
  if (database == NULL)
    return NULL;

  memset (database, 0, sizeof (utmp_database));

  /* Open database.  */
  database->fd = open (file, O_RDWR);
  if (database->fd < 0)
    goto fail;

  database->old_fd = open (old_file, O_RDWR);
  if (database->old_fd < 0)
    goto fail;
  
  if ((file && !(database->file = strdup (file)))
      || (old_file && !(database->old_file = strdup (old_file))))
    goto fail;

  if (initialize_database (database) < 0
      || synchronize_database (database) < 0)
    goto fail;
  
  return database;

fail:
  close_database (database);
  return NULL;
}

/* Synchronize DATABASE.  */
int
synchronize_database (utmp_database *database)
{
  assert (database);

  /* Check if there is a file in the old format, that we have to
     synchronize with.  */
  if (database->old_file)
    {
      time_t curtime;
      time_t mtime;
      
      curtime = time (NULL);
      
      if (get_mtime (database->old_file, &mtime) < 0)
	return -1;
      
      if (mtime >= database->mtime)
	{
	  int position = 0;
	  struct utmp entry;
	  struct utmp old_entry;

	  while (1)
	    {
	      if (read_old_entry (database, position, &old_entry) < 0)
		break;
	      
	      if (read_entry (database, position, &entry) < 0
		  || !compare_entry (&old_entry, &entry))
		{
		  if (write_entry (database, position, &old_entry) < 0)
		    return -1;
		}

	      position++;
	    }

	  database->mtime = curtime;
	}
      
    }

  return 0;
}


/* Close DATABASE.  */
void
close_database (utmp_database *database)
{
  assert (database);

  if (database->fd >= 0)
    close (database->fd);

  if (database->old_fd >= 0)
    close (database->old_fd);
  
  /* Free allocated memory.  */
  if (database->file)
    free (database->file);
  if (database->old_file)
    free (database->old_file);
  free (database);
}


/* Read the entry at POSITION in DATABASE and store the result in
   ENTRY.  Returns 0 if successful, -1 if not.  */
int
read_entry (utmp_database *database, int position, struct utmp *entry)
{
  ssize_t nbytes;
  off_t offset;

  offset = position * sizeof (struct utmp);
  if (lseek (database->fd, offset, SEEK_SET) < 0)
    return -1;

  nbytes = read (database->fd, entry, sizeof (struct utmp));
  if (nbytes != sizeof (struct utmp))
    return -1;
  
  return 0;
}


/* Write ENTRY at POSITION in DATABASE.  Returns 0 if successful, -1
   on error.  */
int
write_entry (utmp_database *database, int position,
	     const struct utmp *entry)
{
  int result = -1;
  struct flock fl;
  ssize_t nbytes;
  off_t offset;

  /* Try to lock the file.  */
  memset (&fl, 0, sizeof (struct flock));
  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fcntl (database->fd, F_SETLKW, &fl);
  
  offset = position * sizeof (struct utmp);
  if (lseek (database->fd, offset, SEEK_SET) < 0)
    goto fail;

  nbytes = write (database->fd, entry, sizeof (struct utmp));
  if (nbytes != sizeof (struct utmp))
    {
      ftruncate (database->fd, offset);
      goto fail;
    }

  result = 0;

fail:
  /* And unlock the file.  */
  fl.l_type = F_UNLCK;
  fcntl (database->fd, F_SETLKW, &fl);

  return result;
}


/* Append ENTRY to DATABASE.  Returns the position of the appended
   entry if successful, or -1 on error.  */
int
append_entry (utmp_database *database, const struct utmp *entry)
{
  int result = -1;
  struct flock fl;
  ssize_t nbytes;
  off_t offset;

  /* Try to lock the file.  */
  memset (&fl, 0, sizeof (struct flock));
  fl.l_type = F_WRLCK;
  fl.l_whence = SEEK_SET;
  fcntl (database->fd, F_SETLKW, &fl);
  
  offset = lseek (database->fd, 0, SEEK_END);
  if (offset % sizeof (struct utmp) != 0)
    {
      offset -= offset % sizeof (struct utmp);
      ftruncate (database->fd, offset);

      if (lseek (database->fd, 0, SEEK_END) < 0)
	goto fail;
    }

  nbytes = write (database->fd, entry, sizeof (struct utmp));
  if (nbytes != sizeof (struct utmp))
    {
      ftruncate (database->fd, offset);
      goto fail;
    }

  result = offset / sizeof (struct utmp);
  
fail:
  /* And unlock the file.  */
  fl.l_type = F_UNLCK;
  fcntl (database->fd, F_SETLKW, &fl);

  return result;
}


int
read_old_entry (utmp_database *database, int position,
		struct utmp *entry)
{
  struct xtmp old_entry;
  ssize_t nbytes;
  off_t offset;

  offset = position * sizeof (struct xtmp);
  if (lseek (database->old_fd, offset, SEEK_SET) < 0)
    return -1;

  nbytes = read (database->old_fd, &old_entry, sizeof (struct xtmp));
  if (nbytes != sizeof (struct xtmp))
    return -1;
  
  xtmp_to_utmp (&old_entry, entry);
  return 0;
}


int
write_old_entry (utmp_database *database, int position,
		 const struct utmp *entry)
{
  struct xtmp old_entry;
  ssize_t nbytes;
  off_t offset;

  utmp_to_xtmp (entry, &old_entry);
  
  offset = position * sizeof (struct xtmp);
  if (lseek (database->old_fd, offset, SEEK_SET) < 0)
    return -1;

  nbytes = write (database->old_fd, &old_entry, sizeof (struct xtmp));
  if (nbytes != sizeof (struct xtmp))
    return -1;

  return 0;
}


/* Initialize DATABASE.  */
static int
initialize_database (utmp_database *database)
{
  struct utmp entry;
  int position = 0;
  
  assert (database);

  /* Check if there is a file in the old format to read.  */
  if (database->old_file)
    {
      while (1)
	{
	  if (read_old_entry (database, position, &entry) < 0)
	    break;

#if _HAVE_UT_TYPE - 0
	  /* If the login type is one of RUN_LVL, BOOT_TIME, OLD_TIME or
	     NEW_TIME, search for an entry of the same type in the
	     database, and replace it if the entry in the file is newer.  */
	  if (entry.ut_type == RUN_LVL || entry.ut_type == BOOT_TIME
	      || entry.ut_type == OLD_TIME || entry.ut_type == NEW_TIME)
	    {
	      if (store_state_entry (database, position, &entry) < 0)
		return -1;
	    }
	  else
#endif
	    {
	      if (store_process_entry (database, position, &entry) < 0)
		return -1;
	    }

	  /* Update position.  */
	  position++;
	}

      while (1)
	{
	  if (read_entry (database, position, &entry) < 0)
	    break;

	  if (write_old_entry (database, position, &entry) < 0)
	    return -1;

	  /* Update position.  */
	  position++;
	}
    }

  return 0;
}


static int
store_state_entry (utmp_database *database, int old_position,
		   const struct utmp *old_entry)
{
  struct utmp new_entry;
  int new_position = 0;
  int found = 0;

  assert (old_entry->ut_type == RUN_LVL
	  || old_entry->ut_type == BOOT_TIME
	  || old_entry->ut_type == OLD_TIME
	  || old_entry->ut_type == NEW_TIME);

  while (!found)
    {
      /* Read the next entry.  */
      if (read_entry (database, new_position, &new_entry) < 0)
	break;
      
      if (old_entry->ut_type == new_entry.ut_type)
	{
	  found = 1;
	  continue;
	}

      /* Update position.  */
      new_position++;
    }

  if (found)
    {
      const struct utmp *entry;

      if (old_entry->ut_time > new_entry.ut_time)
	entry = old_entry;
      else
	entry = &new_entry;
      
      return replace_entry (database, old_position, new_position, entry);
    }

  return store_entry (database, old_position, old_entry);
}


static int
store_process_entry (utmp_database *database, int old_position,
		     const struct utmp *old_entry)
{
  struct utmp new_entry;
  int new_position = 0;
  int found = 0;

  while (!found)
    {
      /* Read the next entry.  */
      if (read_entry (database, new_position, &new_entry) < 0)
	break;

      if (proc_utmp_eq (old_entry, &new_entry))
	{
	  found = 1;
	  continue;
	}

      /* Update position.  */
      new_position++;
    }

  if (found)
    {
      const struct utmp *entry;

      if (old_entry->ut_time > new_entry.ut_time)
	entry = old_entry;
      else
	entry = &new_entry;
      
      return replace_entry (database, old_position, new_position, entry);
    }

  return store_entry (database, old_position, old_entry);
}


static int
replace_entry (utmp_database *database, int old_position, int new_position,
	       const struct utmp *entry)
{
  struct utmp tmp;
  
  if (read_entry (database, old_position, &tmp) < 0
      || write_entry (database, old_position, entry) < 0
      || write_entry (database, new_position, &tmp) < 0)
    return -1;

  return 0;
}


static int
store_entry (utmp_database *database, int position,
	     const struct utmp *entry)
{
  struct utmp tmp;

  if (read_entry (database, position, &tmp) < 0)
    return write_entry (database, position, entry);

  if (write_entry (database, position, entry) < 0
      || append_entry (database, &tmp) < 0)
    return -1;

  return 0;
}


/* Get modification time of FILE and put it in TIMER.  returns 0 if
   successful, -1 if not.  */
static int
get_mtime (const char *file, time_t *timer)
{
  struct stat st;
  
  if (stat (file, &st) < 0)
    return -1;

  *timer = st.st_mtime;

  return 0;
}
