/* Tests for ftruncate and truncate.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 2000.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>


/* Allow testing of the 64-bit versions as well.  */
#ifndef TRUNCATE
# define TRUNCATE truncate
# define FTRUNCATE ftruncate
#endif

#define STRINGIFY(s) STRINGIFY2 (s)
#define STRINGIFY2(s) #s

/* Prototype for our test function.  */
extern void do_prepare (int argc, char *argv[]);
extern int do_test (int argc, char *argv[]);

/* We have a preparation function.  */
#define PREPARE do_prepare

/* We might need a bit longer timeout.  */
#define TIMEOUT 20 /* sec */

/* This defines the `main' function and some more.  */
#include <test-skeleton.c>

/* These are for the temporary file we generate.  */
char *name;
int fd;

void
do_prepare (int argc, char *argv[])
{
   size_t name_len;

#define FNAME FNAME2(TRUNCATE)
#define FNAME2(s) "/" STRINGIFY(s) "XXXXXX"

   name_len = strlen (test_dir);
   name = malloc (name_len + sizeof (FNAME));
   mempcpy (mempcpy (name, test_dir, name_len), FNAME, sizeof (FNAME));
   add_temp_file (name);

   /* Open our test file.   */
   fd = mkstemp (name);
   if (fd == -1)
     error (EXIT_FAILURE, errno, "cannot open test file `%s'", name);
}


int
do_test (int argc, char *argv[])
{
  struct stat st;
  char buf[1000];

  memset (buf, '\0', sizeof (buf));

  if (write (fd, buf, sizeof (buf)) != sizeof (buf))
    error (EXIT_FAILURE, errno, "during write");

  if (fstat (fd, &st) < 0 || st.st_size != sizeof (buf))
    error (EXIT_FAILURE, 0, "initial size wrong");


  if (FTRUNCATE (fd, 800) < 0)
    error (EXIT_FAILURE, errno, "size reduction with %s failed",
	   STRINGIFY (FTRUNCATE));

  if (fstat (fd, &st) < 0 || st.st_size != 800)
    error (EXIT_FAILURE, 0, "size after reduction with %s incorrect",
	   STRINGIFY (FTRUNCATE));

  /* The following test covers more than POSIX.  POSIX does not require
     that ftruncate() can increase the file size.  But we are testing
     Unix systems.  */
  if (FTRUNCATE (fd, 1200) < 0)
    error (EXIT_FAILURE, errno, "size increase with %s failed",
	   STRINGIFY (FTRUNCATE));

  if (fstat (fd, &st) < 0 || st.st_size != 1200)
    error (EXIT_FAILURE, 0, "size after increase with %s incorrect",
	   STRINGIFY (FTRUNCATE));


  if (TRUNCATE (name, 800) < 0)
    error (EXIT_FAILURE, errno, "size reduction with %s failed",
	   STRINGIFY (TRUNCATE));

  if (fstat (fd, &st) < 0 || st.st_size != 800)
    error (EXIT_FAILURE, 0, "size after reduction with %s incorrect",
	   STRINGIFY (TRUNCATE));

  /* The following test covers more than POSIX.  POSIX does not require
     that truncate() can increase the file size.  But we are testing
     Unix systems.  */
  if (TRUNCATE (name, 1200) < 0)
    error (EXIT_FAILURE, errno, "size increase with %s failed",
	   STRINGIFY (TRUNCATE));

  if (fstat (fd, &st) < 0 || st.st_size != 1200)
    error (EXIT_FAILURE, 0, "size after increase with %s incorrect",
	   STRINGIFY (TRUNCATE));


  close (fd);
  unlink (name);

  return 0;
}
