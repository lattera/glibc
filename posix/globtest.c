/* Copyright (C) 1997 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <glob.h>
#include <unistd.h>

int
main (int argc, char *argv[])
{
  int i;
  int glob_flags = GLOB_NOSORT;
  glob_t filenames;

  if (argc != 3)
    exit (1);
  if (chdir (argv[1]))
    exit (1);
  i = glob (argv[2], glob_flags, NULL, &filenames);

  if (i == GLOB_NOSPACE)
    puts ("GLOB_NOSPACE");
  else if (i == GLOB_ABEND)
    puts ("GLOB_ABEND");
  else if (i == GLOB_NOMATCH)
    puts ("GLOB_NOMATCH");

  printf ("%sNULL\n", filenames.gl_pathv ? "not " : "");

  if (filenames.gl_pathv)
    {
      for (i = 0; i < filenames.gl_pathc; ++i)
	printf ("`%s'\n", filenames.gl_pathv[i]);
    }
  return 0;
}
