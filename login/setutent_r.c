/* Copyright (C) 1996 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utmp.h>

/* This is the default name.  */
static const char default_utmp_name[] = _PATH_UTMP;

/* Current file name.  */
static const char *utmp_file_name = (const char *) default_utmp_name;


void
setutent_r (struct utmp_data *utmp_data)
{
  /* Before the UTMP_DATA is used before the first time the UT_FD
     field must be set to -1.  */
  if (utmp_data->ut_fd == -1)
    {
      utmp_data->ut_fd = open (utmp_file_name, O_RDWR);
      if (utmp_data->ut_fd == -1)
	{
	  /* Hhm, read-write access did not work.  Try read-only.  */
	  utmp_data->ut_fd = open (utmp_file_name, O_RDONLY);
	  if (utmp_data->ut_fd == -1)
	    {
	      perror (_("while opening UTMP file"));
	      return;
	    }
	}
    }

  /* Remember we are at beginning of file.  */
  utmp_data->loc_utmp = 0;
#if _HAVE_UT_TYPE - 0
  utmp_data->ubuf.ut_type = UT_UNKNOWN;
#endif
}


int
utmpname (const char *file)
{
  char *fname = strdup (file);
  if (fname == NULL)
    return 0;

  if (utmp_file_name != default_utmp_name)
    free ((void *) utmp_file_name);

  utmp_file_name = fname;

  return 1;
}
