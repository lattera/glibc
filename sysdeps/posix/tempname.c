/* Copyright (C) 1991,92,93,94,95,96,97,98 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#ifdef USE_IN_LIBIO
# include "libioP.h"
# include <libio.h>
#endif

/* Return nonzero if DIR is an existent directory.  */
static int
diraccess (const char *dir)
{
  struct stat buf;
  return __stat (dir, &buf) == 0 && S_ISDIR (buf.st_mode);
}

/* Return nonzero if FILE exists.  */
static int
exists (const char *file)
{
  /* We can stat the file even if we can't read its data.  */
  struct stat st;
  int save = errno;
  if (__stat (file, &st) == 0)
    return 1;
  else
    {
      /* We report that the file exists if stat failed for a reason other
	 than nonexistence.  In this case, it may or may not exist, and we
	 don't know; but reporting that it does exist will never cause any
	 trouble, while reporting that it doesn't exist when it does would
	 violate the interface of __stdio_gen_tempname.  */
      int exists = errno != ENOENT;
      __set_errno (save);
      return exists;
    }
}


/* These are the characters used in temporary filenames.  */
static const char letters[] =
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

/* Generate a temporary filename and return it (in a static buffer).  If
   STREAMPTR is not NULL, open a stream "w+b" on the file and set
   *STREAMPTR to it.  If DIR_SEARCH is nonzero, DIR and PFX are used as
   described for tempnam.  If not, a temporary filename in P_tmpdir with no
   special prefix is generated.  If LENPTR is not NULL, *LENPTR is set the
   to length (including the terminating '\0') of the resultant filename,
   which is returned.  This goes through a cyclic pattern of all possible
   filenames consisting of five decimal digits of the current pid and three
   of the characters in `letters'.  Data for tempnam and tmpnam is kept
   separate, but when tempnam is using P_tmpdir and no prefix (i.e, it is
   identical to tmpnam), the same data is used.  Each potential filename is
   tested for an already-existing file of the same name, and no name of an
   existing file will be returned.  When the cycle reaches its end
   (12345ZZZ), NULL is returned.  */
char *
__stdio_gen_tempname (char *buf, size_t bufsize, const char *dir,
		      const char *pfx, int dir_search, size_t *lenptr,
		      FILE **streamptr, int large_file)
{
  int saverrno = errno;
  static const char tmpdir[] = P_tmpdir;
  size_t plen, dlen, len;
  char *XXXXXX;
  static uint64_t value;
  struct timeval tv;
  int count;

  if (dir_search)
    {
      register const char *d = __secure_getenv ("TMPDIR");
      if (d != NULL && !diraccess (d))
	d = NULL;
      if (d == NULL && dir != NULL && diraccess (dir))
	d = dir;
      if (d == NULL && diraccess (tmpdir))
	d = tmpdir;
      if (d == NULL && diraccess ("/tmp"))
	d = "/tmp";
      if (d == NULL)
	{
	  __set_errno (ENOENT);
	  return NULL;
	}
      dir = d;
    }
  else
    dir = tmpdir;

  dlen = strlen (dir);

 /* Remove trailing slashes from the directory name.  */
  while (dlen > 1 && dir[dlen - 1] == '/')
    --dlen;

  if (pfx != NULL && *pfx != '\0')
    {
      plen = strlen (pfx);
      if (plen > 5)
	plen = 5;
    }
  else
    plen = 0;

  len = __snprintf (buf, bufsize, "%.*s/%.*sXXXXXX",
	      (int) dlen, dir, (int) plen, pfx);

  if (len < dlen + plen + 7)
  {
      __set_errno (EINVAL);
      return NULL;
  }

  XXXXXX = &buf[dlen + plen + 1];

  /* Get some more or less random data.  */
  __gettimeofday (&tv, NULL);
  value += ((uint64_t) tv.tv_usec << 16) ^ tv.tv_sec ^ __getpid ();

  for (count = 0; count < TMP_MAX; value += 7777, ++count)
    {
      uint64_t v = value;

      /* Fill in the random bits.  */
      XXXXXX[0] = letters[v % 62];
      v /= 62;
      XXXXXX[1] = letters[v % 62];
      v /= 62;
      XXXXXX[2] = letters[v % 62];
      v /= 62;
      XXXXXX[3] = letters[v % 62];
      v /= 62;
      XXXXXX[4] = letters[v % 62];
      v /= 62;
      XXXXXX[5] = letters[v % 62];

      if (streamptr != NULL)
	{
	  /* Try to create the file atomically.  */
#ifdef _G_OPEN64
	  int fd = (large_file
		    ? __open (buf, O_RDWR|O_CREAT|O_EXCL, 0666)
		    : _G_OPEN64 (buf, O_RDWR|O_CREAT|O_EXCL, 0666));
#else
	  int fd = __open (buf, O_RDWR|O_CREAT|O_EXCL, 0666);
#endif
	  if (fd >= 0)
	    {
	      /* We got a new file that did not previously exist.
		 Create a stream for it.  */
#ifdef USE_IN_LIBIO
	      int save;
	      struct locked_FILE
		{
		  struct _IO_FILE_plus fp;
#ifdef _IO_MTSAFE_IO
		  _IO_lock_t lock;
#endif
		} *new_f;
	      struct _IO_FILE_plus *fp;

	      new_f = (struct locked_FILE *)
		malloc (sizeof (struct locked_FILE));
	      if (new_f == NULL)
		{
		  /* We lost trying to create a stream (out of memory?).
		     Nothing to do but remove the file, close the descriptor,
		     and return failure.  */
		  save = errno;
		lose:
		  (void) remove (buf);
		  (void) __close (fd);
		  __set_errno (save);
		  return NULL;
		}
	      fp = &new_f->fp;
#ifdef _IO_MTSAFE_IO
	      fp->file._lock = &new_f->lock;
#endif
	      _IO_init (&fp->file, 0);
	      _IO_JUMPS (&fp->file) = &_IO_file_jumps;
	      _IO_file_init (&fp->file);
# if !_IO_UNIFIED_JUMPTABLES
	      fp->vtable = NULL;
# endif
	      if (_IO_file_attach (&fp->file, fd) == NULL)
		{
		  save = errno;
		  free (fp);
		  goto lose;
		}
	      fp->file._flags &= ~_IO_DELETE_DONT_CLOSE;

	      *streamptr = (FILE *) fp;
#else
	      *streamptr = __newstream ();
	      if (*streamptr == NULL)
		{
		  /* We lost trying to create a stream (out of memory?).
		     Nothing to do but remove the file, close the descriptor,
		     and return failure.  */
		  const int save = errno;
		  (void) remove (buf);
		  (void) __close (fd);
		  __set_errno (save);
		  return NULL;
		}
	      (*streamptr)->__cookie = (__ptr_t) (long int) fd;
	      (*streamptr)->__mode.__write = 1;
	      (*streamptr)->__mode.__read = 1;
	      (*streamptr)->__mode.__binary = 1;
#endif
	    }
#if defined EMFILE || defined ENFILE || defined EINTR
	  else if (0
# ifdef EMFILE
		   || errno == EMFILE
# endif
# ifdef ENFILE
		   || errno == ENFILE
# endif
# ifdef EINTR
		   || errno == EINTR
# endif
		   )
	    /* We cannot open anymore files since all descriptors are
	       used or because we got a signal.  */
	    return NULL;
#endif
	  else
	    continue;
	}
      else if (exists (buf))
	continue;

      /* If the file already existed we have continued the loop above,
	 so we only get here when we have a winning name to return.  */

      __set_errno (saverrno);

      if (lenptr != NULL)
	*lenptr = len + 1;
      return buf;
    }

  /* We got out of the loop because we ran out of combinations to try.  */
  __set_errno (EEXIST);		/* ? */
  return NULL;
}
