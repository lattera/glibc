/* Tests of fseek and fseeko.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2000.

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

#include <error.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>


int
main (void)
{
  const char *tmpdir;
  char *fname;
  int fd;
  FILE *fp;
  const char outstr[] = "hello world!\n";
  char strbuf[sizeof outstr];
  char buf[200];
  struct stat64 st1;
  struct stat64 st2;
  int result = 0;

  tmpdir = getenv ("TMPDIR");
  if (tmpdir == NULL || tmpdir[0] == '\0')
    tmpdir = "/tmp";

  asprintf (&fname, "%s/tst-fseek.XXXXXX", tmpdir);
  if (fname == NULL)
    error (EXIT_FAILURE, errno, "cannot generate name for temporary file");

  /* Create a temporary file.   */
  fd = mkstemp (fname);
  if (fd == -1)
    error (EXIT_FAILURE, errno, "cannot open temporary file");

  fp = fdopen (fd, "w+");
  if (fp == NULL)
    error (EXIT_FAILURE, errno, "cannot get FILE for temporary file");

  setbuffer (fp, strbuf, sizeof (outstr) -1);

  if (fwrite (outstr, sizeof (outstr) - 1, 1, fp) != 1)
    {
      puts ("write error");
      result = 1;
      goto out;
    }

  /* The EOF flag must be reset.  */
  if (fgetc (fp) != EOF)
    {
      puts ("managed to read at end of file");
      result = 1;
    }
  else if (! feof (fp))
    {
      puts ("EOF flag not set");
      result = 1;
    }
  if (fseek (fp, 0, SEEK_CUR) != 0)
    {
      puts ("fseek(fp, 0, SEEK_CUR) failed");
      result = 1;
    }
  else if (feof (fp))
    {
      puts ("fseek() didn't reset EOF flag");
      result = 1;
    }

  /* Do the same for fseeko().  */
#ifdef USE_IN_LIBIO
    if (fgetc (fp) != EOF)
    {
      puts ("managed to read at end of file");
      result = 1;
    }
  else if (! feof (fp))
    {
      puts ("EOF flag not set");
      result = 1;
    }
  if (fseeko (fp, 0, SEEK_CUR) != 0)
    {
      puts ("fseek(fp, 0, SEEK_CUR) failed");
      result = 1;
    }
  else if (feof (fp))
    {
      puts ("fseek() didn't reset EOF flag");
      result = 1;
    }
#endif

  /* Go back to the beginning of the file: absolute.  */
  if (fseek (fp, 0, SEEK_SET) != 0)
    {
      puts ("fseek(fp, 0, SEEK_SET) failed");
      result = 1;
    }
  else if (fflush (fp) != 0)
    {
      puts ("fflush() failed");
      result = 1;
    }
  else if (lseek (fd, 0, SEEK_CUR) != 0)
    {
      puts ("lseek() returned different position");
      result = 1;
    }
  else if (fread (buf, sizeof (outstr) - 1, 1, fp) != 1)
    {
      puts ("fread() failed");
      result = 1;
    }
  else if (memcmp (buf, outstr, sizeof (outstr) - 1) != 0)
    {
      puts ("content after fseek(,,SEEK_SET) wrong");
      result = 1;
    }

#ifdef USE_IN_LIBIO
  /* Now with fseeko.  */
  if (fseeko (fp, 0, SEEK_SET) != 0)
    {
      puts ("fseeko(fp, 0, SEEK_SET) failed");
      result = 1;
    }
  else if (fflush (fp) != 0)
    {
      puts ("fflush() failed");
      result = 1;
    }
  else if (lseek (fd, 0, SEEK_CUR) != 0)
    {
      puts ("lseek() returned different position");
      result = 1;
    }
  else if (fread (buf, sizeof (outstr) - 1, 1, fp) != 1)
    {
      puts ("fread() failed");
      result = 1;
    }
  else if (memcmp (buf, outstr, sizeof (outstr) - 1) != 0)
    {
      puts ("content after fseeko(,,SEEK_SET) wrong");
      result = 1;
    }
#endif

  /* Go back to the beginning of the file: relative.  */
  if (fseek (fp, -(sizeof (outstr) - 1), SEEK_CUR) != 0)
    {
      puts ("fseek(fp, 0, SEEK_SET) failed");
      result = 1;
    }
  else if (fflush (fp) != 0)
    {
      puts ("fflush() failed");
      result = 1;
    }
  else if (lseek (fd, 0, SEEK_CUR) != 0)
    {
      puts ("lseek() returned different position");
      result = 1;
    }
  else if (fread (buf, sizeof (outstr) - 1, 1, fp) != 1)
    {
      puts ("fread() failed");
      result = 1;
    }
  else if (memcmp (buf, outstr, sizeof (outstr) - 1) != 0)
    {
      puts ("content after fseek(,,SEEK_SET) wrong");
      result = 1;
    }

#ifdef USE_IN_LIBIO
  /* Now with fseeko.  */
  if (fseeko (fp, -(sizeof (outstr) - 1), SEEK_CUR) != 0)
    {
      puts ("fseeko(fp, 0, SEEK_SET) failed");
      result = 1;
    }
  else if (fflush (fp) != 0)
    {
      puts ("fflush() failed");
      result = 1;
    }
  else if (lseek (fd, 0, SEEK_CUR) != 0)
    {
      puts ("lseek() returned different position");
      result = 1;
    }
  else if (fread (buf, sizeof (outstr) - 1, 1, fp) != 1)
    {
      puts ("fread() failed");
      result = 1;
    }
  else if (memcmp (buf, outstr, sizeof (outstr) - 1) != 0)
    {
      puts ("content after fseeko(,,SEEK_SET) wrong");
      result = 1;
    }
#endif

  /* Go back to the beginning of the file: from the end.  */
  if (fseek (fp, -(sizeof (outstr) - 1), SEEK_END) != 0)
    {
      puts ("fseek(fp, 0, SEEK_SET) failed");
      result = 1;
    }
  else if (fflush (fp) != 0)
    {
      puts ("fflush() failed");
      result = 1;
    }
  else if (lseek (fd, 0, SEEK_CUR) != 0)
    {
      puts ("lseek() returned different position");
      result = 1;
    }
  else if (fread (buf, sizeof (outstr) - 1, 1, fp) != 1)
    {
      puts ("fread() failed");
      result = 1;
    }
  else if (memcmp (buf, outstr, sizeof (outstr) - 1) != 0)
    {
      puts ("content after fseek(,,SEEK_SET) wrong");
      result = 1;
    }

#ifdef USE_IN_LIBIO
  /* Now with fseeko.  */
  if (fseeko (fp, -(sizeof (outstr) - 1), SEEK_END) != 0)
    {
      puts ("fseeko(fp, 0, SEEK_SET) failed");
      result = 1;
    }
  else if (fflush (fp) != 0)
    {
      puts ("fflush() failed");
      result = 1;
    }
  else if (lseek (fd, 0, SEEK_CUR) != 0)
    {
      puts ("lseek() returned different position");
      result = 1;
    }
  else if (fread (buf, sizeof (outstr) - 1, 1, fp) != 1)
    {
      puts ("fread() failed");
      result = 1;
    }
  else if (memcmp (buf, outstr, sizeof (outstr) - 1) != 0)
    {
      puts ("content after fseeko(,,SEEK_SET) wrong");
      result = 1;
    }
#endif

  if (fwrite (outstr, sizeof (outstr) - 1, 1, fp) != 1)
    {
      puts ("write error 2");
      result = 1;
      goto out;
    }

  if (fwrite (outstr, sizeof (outstr) - 1, 1, fp) != 1)
    {
      puts ("write error 3");
      result = 1;
      goto out;
    }

  if (fwrite (outstr, sizeof (outstr) - 1, 1, fp) != 1)
    {
      puts ("write error 4");
      result = 1;
      goto out;
    }

  if (fwrite (outstr, sizeof (outstr) - 1, 1, fp) != 1)
    {
      puts ("write error 5");
      result = 1;
      goto out;
    }

  if (fputc ('1', fp) == EOF || fputc ('2', fp) == EOF)
    {
      puts ("cannot add characters at the end");
      result = 1;
      goto out;
    }

  /* Check the access time.  */
  if (fstat64 (fd, &st1) < 0)
    {
      puts ("fstat64() before fseeko() failed\n");
      result = 1;
    }
  else
    {
      sleep (1);

      if (fseek (fp, -(2 + 2 * (sizeof (outstr) - 1)), SEEK_CUR) != 0)
	{
	  puts ("fseek() after write characters failed");
	  result = 1;
	  goto out;
	}
      else
	{

	  time_t t;
	  /* Make sure the timestamp actually can be different.  */
	  sleep (1);
	  t = time (NULL);

	  if (fstat64 (fd, &st2) < 0)
	    {
	      puts ("fstat64() after fseeko() failed\n");
	      result = 1;
	    }
	  if (st1.st_ctime >= t)
	    {
	      puts ("st_ctime not updated");
	      result = 1;
	    }
	  if (st1.st_mtime >= t)
	    {
	      puts ("st_mtime not updated");
	      result = 1;
	    }
	  if (st1.st_ctime >= st2.st_ctime)
	    {
	      puts ("st_ctime not changed");
	      result = 1;
	    }
	  if (st1.st_mtime >= st2.st_mtime)
	    {
	      puts ("st_mtime not changed");
	      result = 1;
	    }
	}
    }

  if (fread (buf, 1, 2 + 2 * (sizeof (outstr) - 1), fp)
      != 2 + 2 * (sizeof (outstr) - 1))
    {
      puts ("reading 2 records plus bits failed");
      result = 1;
    }
  else if (memcmp (buf, outstr, sizeof (outstr) - 1) != 0
	   || memcmp (&buf[sizeof (outstr) - 1], outstr,
		      sizeof (outstr) - 1) != 0
	   || buf[2 * (sizeof (outstr) - 1)] != '1'
	   || buf[2 * (sizeof (outstr) - 1) + 1] != '2')
    {
      puts ("reading records failed");
      result = 1;
    }
  else if (ungetc ('9', fp) == EOF)
    {
      puts ("ungetc() failed");
      result = 1;
    }
  else if (fseek (fp, -(2 + 2 * (sizeof (outstr) - 1)), SEEK_END) != 0)
    {
      puts ("fseek after ungetc failed");
      result = 1;
    }
  else if (fread (buf, 1, 2 + 2 * (sizeof (outstr) - 1), fp)
      != 2 + 2 * (sizeof (outstr) - 1))
    {
      puts ("reading 2 records plus bits failed");
      result = 1;
    }
  else if (memcmp (buf, outstr, sizeof (outstr) - 1) != 0
	   || memcmp (&buf[sizeof (outstr) - 1], outstr,
		      sizeof (outstr) - 1) != 0
	   || buf[2 * (sizeof (outstr) - 1)] != '1')
    {
      puts ("reading records for the second time failed");
      result = 1;
    }
  else if (buf[2 * (sizeof (outstr) - 1) + 1] == '9')
    {
      puts ("unget character not ignored");
      result = 1;
    }
  else if (buf[2 * (sizeof (outstr) - 1) + 1] != '2')
    {
      puts ("unget somehow changed character");
      result = 1;
    }

 out:
  unlink (fname);

  return result;
}
