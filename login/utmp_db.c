/* Copyright (C) 1996 Free Software Foundation, Inc.
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

#include <assert.h>
#include <db.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utmp.h>
#include <sys/stat.h>

#include "utmp-private.h"


/* This is the default name.  */
static const char default_file_name[] = _PATH_UTMP_DB;

/* Current file name.  */
static const char *file_name = (const char *) default_file_name;

/* Descriptor for database.  */
static DB *db_fd;
static char last_date[16];


/* Our local functions.  */
static int setutent_db (int reset);
static void endutent_db (void);
static int utmpname_db (const char *name);


/* The jump table for the local functions.  */
struct utfuncs __libc_utmp_db_functions =
{
  setutent_db,
  NULL,
  NULL,
  NULL,
  NULL,
  endutent_db,
  utmpname_db
};


static int
setutent_db (int reset)
{
  return 0;
}


static void
endutent_db (void)
{
}


static int
utmpname_db (const char *name)
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
