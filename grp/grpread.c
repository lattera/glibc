/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <grp.h>

/* This is the function that all the others are based on.
   The format of the group file is known only here.  */

/* Structure containing info kept by each __grpread caller.  */
typedef struct
  {
    char *buf;
    size_t buflen;
    size_t max_members;
    char **members;
    struct group g;
  } grpread_info;


/* Return a chunk of memory containing a pre-initialized `grpread_info'.  */
PTR
DEFUN_VOID(__grpalloc)
{
  grpread_info *info = (PTR) malloc (sizeof(grpread_info));
  if (info == NULL)
    return NULL;

  info->buf = NULL;
  info->buflen = 0;

  info->max_members = 5;
  info->members = (char **) malloc (5 * sizeof(char *));
  if (info->members == NULL)
    {
      free ((PTR) info);
      return NULL;
    }

  return info;
}

/* Read a group entry from STREAM, filling in G.  */
struct group *
DEFUN(__grpread, (stream, g), FILE *stream AND PTR CONST g)
{
  register grpread_info *CONST info = (grpread_info *) g;
  char *start, *end;
  register size_t i;

  /* Idiocy checks.  */
  if (stream == NULL)
    {
      errno = EINVAL;
      return NULL;
    }

  do
    if (__getline (&info->buf, &info->buflen, stream) == -1)
      return NULL;
  while (info->buf[0] == '#');

  start = info->buf;
  end = strchr (start, ':');
  if (end == NULL)
    return NULL;
  *end = '\0';
  info->g.gr_name = start;

  start = end + 1;
  end = strchr (start, ':');
  if (end == NULL)
    return NULL;
  *end = '\0';
  info->g.gr_passwd = start;

  info->g.gr_gid = (gid_t) strtol (end + 1, &end, 10);
  if (*end != ':')
    return NULL;

  i = 0;
  do
    {
      start = end + 1;
      end = strchr (start, ',');
      if (end == NULL)
	{
	  end = strchr (start, '\n');
	  if (end == start)
	    break;
	  if (end == NULL)
	    return NULL;
	  *end = '\0';
	  end = NULL;
	}
      else
	*end = '\0';

      if (i == info->max_members - 2)
	{
	  info->max_members += 5;
	  info->members = (char **)
	    realloc ((PTR) info->members, info->max_members * sizeof (char *));
	  if (info->members == NULL)
	    return NULL;
	}

      info->members[i++] = start;
    } while (end != NULL);
  info->members[i] = NULL;
  info->g.gr_mem = info->members;

  return &info->g;
}
