/* Copyright (C) 1994 Free Software Foundation, Inc.
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

#include <utime.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

int
main ()
{
  char file[L_tmpnam];
  struct utimbuf ut;
  struct stat st;
  int fd;

  if (tmpnam (file) == 0)
    {
      perror ("tmpnam");
      exit (1);
    }

  fd = creat (file, 0666);
  if (fd < 0)
    {
      perror ("creat");
      exit (1);
    }
  close (fd);

  ut.actime = 500000000;
  ut.modtime = 500000001;
  if (utime (file, &ut))
    {
      perror ("utime");
      remove (file);
      exit (1);
    }

  if (stat (file, &st))
    {
      perror ("stat");
      remove (file);
      exit (1);
    }

  remove (file);

  if (st.st_mtime != ut.modtime)
    {
      printf ("modtime %ld != %ld\n", st.st_mtime, ut.modtime);
      exit (1);
    }

  if (st.st_atime != ut.actime)
    {
      printf ("actime %ld != %ld\n", st.st_atime, ut.actime);
      exit (1);
    }

  puts ("Test succeeded.");
  exit (0);
}
