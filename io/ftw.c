/* Copyright (C) 1992, 1995, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ian Lance Taylor (ian@airs.com).

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
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ftw.h>


#ifndef PATH_MAX
#define PATH_MAX 1024		/* XXX */
#endif


/* Traverse one level of a directory tree.  */

static int
ftw_dir (DIR **dirs, int level, int descriptors, char *dir, size_t len,
	 int (*func) (const char *file, const struct stat *status, int flag))
{
  int got;
  struct dirent *entry;

  got = 0;

  __set_errno (0);

  while ((entry = readdir (dirs[level])) != NULL)
    {
      struct stat s;
      int flag, retval, newlev;
      size_t namlen;

      ++got;

      if (entry->d_name[0] == '.'
	  && (entry->d_name[1] == '\0' ||
	      (entry->d_name[1] == '.' && entry->d_name[2] == '\0')))
	{
	  __set_errno (0);
	  continue;
	}

      namlen = _D_EXACT_NAMLEN (entry);

      if (namlen + len + 1 > PATH_MAX)
	{
#ifdef ENAMETOOLONG
	  __set_errno (ENAMETOOLONG);
#else
	  __set_errno (ENOMEM);
#endif
	  return -1;
	}

      dir[len] = '/';
      memcpy ((void *) (dir + len + 1), (void *) entry->d_name,
	      namlen + 1);

      if (stat (dir, &s) < 0)
	{
	  if (errno != EACCES && errno != ENOENT)
	    return -1;
	  flag = FTW_NS;
	}
      else if (S_ISDIR (s.st_mode))
	{
	  newlev = (level + 1) % descriptors;

	  if (dirs[newlev] != NULL)
	    closedir (dirs[newlev]);

	  dirs[newlev] = opendir (dir);
	  if (dirs[newlev] != NULL)
	    flag = FTW_D;
	  else
	    {
	      if (errno != EACCES)
		return -1;
	      flag = FTW_DNR;
	    }
	}
      else
	flag = FTW_F;

      retval = (*func) (dir, &s, flag);

      if (flag == FTW_D)
	{
	  if (retval == 0)
	    retval = ftw_dir (dirs, newlev, descriptors, dir,
			      namlen + len + 1, func);
	  if (dirs[newlev] != NULL)
	    {
	      int save;

	      save = errno;
	      closedir (dirs[newlev]);
	      __set_errno (save);
	      dirs[newlev] = NULL;
	    }
	}

      if (retval != 0)
	return retval;

      if (dirs[level] == NULL)
	{
	  int skip;

	  dir[len] = '\0';
	  dirs[level] = opendir (dir);
	  if (dirs[level] == NULL)
	    return -1;
	  skip = got;
	  while (skip-- != 0)
	    {
	      __set_errno (0);
	      if (readdir (dirs[level]) == NULL)
		return errno == 0 ? 0 : -1;
	    }
	}

      __set_errno (0);
    }

  return errno == 0 ? 0 : -1;
}

/* Call a function on every element in a directory tree.  */

int
ftw (const char *dir,
     int (*func) (const char *file, const struct stat *status, int flag),
     int descriptors)
{
  DIR **dirs;
  size_t len;
  char buf[PATH_MAX + 1];
  struct stat s;
  int flag, retval;
  int i;

  if (descriptors <= 0)
    descriptors = 1;

  dirs = (DIR **) __alloca (descriptors * sizeof (DIR *));
  i = descriptors;
  while (i-- > 0)
    dirs[i] = NULL;

  if (stat (dir, &s) < 0)
    {
      if (errno != EACCES && errno != ENOENT)
	return -1;
      flag = FTW_NS;
    }
  else if (S_ISDIR (s.st_mode))
    {
      dirs[0] = opendir (dir);
      if (dirs[0] != NULL)
	flag = FTW_D;
      else
	{
	  if (errno != EACCES)
	    return -1;
	  flag = FTW_DNR;
	}
    }
  else
    flag = FTW_F;

  len = strlen (dir);
  memcpy ((void *) buf, (void *) dir, len + 1);

  retval = (*func) (buf, &s, flag);

  if (flag == FTW_D)
    {
      if (retval == 0)
	retval = ftw_dir (dirs, 0, descriptors, buf, len, func);
      if (dirs[0] != NULL)
	{
	  int save;

	  save = errno;
	  closedir (dirs[0]);
	  __set_errno (save);
	}
    }

  return retval;
}
