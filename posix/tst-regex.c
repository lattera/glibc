/* Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <spawn.h>
#include "spawn_int.h"

#include <assert.h>
#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <iconv.h>
#include <locale.h>
#include <mcheck.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <regex.h>


#ifdef _POSIX_CPUTIME
static clockid_t cl;
static int use_clock;
#endif
static iconv_t cd;
static char *mem;
static char *umem;
static size_t memlen;

static int test_expr (const char *expr, int expected);
static int run_test (const char *expr, const char *mem, size_t memlen,
		     int expected);


int
main (void)
{
  const char *file;
  int fd;
  struct stat st;
  int result;
  char *inmem;
  char *outmem;
  size_t inlen;
  size_t outlen;

  mtrace ();

  /* Make the content of the file available in memory.  */
  file = "../ChangeLog.8";
  fd = open (file, O_RDONLY);
  if (fd == -1)
    error (EXIT_FAILURE, errno, "cannot open %s", basename (file));

  if (fstat (fd, &st) != 0)
    error (EXIT_FAILURE, errno, "cannot stat %s", basename (file));
  memlen = st.st_size;

  mem = (char *) malloc (memlen + 1);
  if (mem == NULL)
    error (EXIT_FAILURE, errno, "while allocating buffer");

  if (read (fd, mem, memlen) != memlen)
    error (EXIT_FAILURE, 0, "cannot read entire file");
  mem[memlen] = '\0';

  close (fd);

  /* We have to convert a few things from Latin-1 to UTF-8.  */
  cd = iconv_open ("UTF-8", "ISO-8859-1");
  if (cd == (iconv_t) -1)
    error (EXIT_FAILURE, errno, "cannot get conversion descriptor");

  /* For the second test we have to convert the file content to UTF-8.
     Since the text is mostly ASCII it should be enough to allocate
     twice as much memory for the UTF-8 text than for the Latin-1
     text.  */
  umem = (char *) calloc (2, memlen);
  if (umem == NULL)
    error (EXIT_FAILURE, errno, "while allocating buffer");

  inmem = mem;
  inlen = memlen;
  outmem = umem;
  outlen = 2 * memlen - 1;
  iconv (cd, &inmem, &inlen, &outmem, &outlen);
  if (inlen != 0)
    error (EXIT_FAILURE, errno, "cannot convert buffer");

#ifdef _POSIX_CPUTIME
  /* See whether we can use the CPU clock.  */
  use_clock = clock_getcpuclockid (0, &cl) == 0;
#endif

#ifdef DEBUG
  re_set_syntax (RE_DEBUG);
#endif

  /* Run the actual tests.  All tests are run in a single-byte and a
     multi-byte locale.  */
  result = test_expr ("[‰·‡‚ÈËÍÌÏÓÒˆÛÚÙ¸˙˘˚]", 2);
  result |= test_expr ("G.ran", 2);
  result |= test_expr ("G.\\{1\\}ran", 2);
  result |= test_expr ("G.*ran", 3);
  result |= test_expr ("[‰·‡‚]", 0);

  /* Free the resources.  */
  free (umem);
  iconv_close (cd);
  free (mem);

  return result;
}


static int
test_expr (const char *expr, int expected)
{
  int result;
  char *inmem;
  char *outmem;
  size_t inlen;
  size_t outlen;
  char *uexpr;

  /* First test: search with an ISO-8859-1 locale.  */
  if (setlocale (LC_ALL, "de_DE.ISO-8859-1") == NULL)
    error (EXIT_FAILURE, 0, "cannot set locale de_DE.ISO-8859-1");

  printf ("\nTest \"%s\" with 8-bit locale\n", expr);
  result = run_test (expr, mem, memlen, expected);

  /* Second test: search with an UTF-8 locale.  */
  if (setlocale (LC_ALL, "de_DE.UTF-8") == NULL)
    error (EXIT_FAILURE, 0, "cannot set locale de_DE.UTF-8");

  inmem = (char *) expr;
  inlen = strlen (expr);
  outlen = inlen * MB_CUR_MAX;
  outmem = uexpr = alloca (outlen + 1);
  memset (outmem, '\0', outlen + 1);
  iconv (cd, &inmem, &inlen, &outmem, &outlen);
  if (inlen != 0)
    error (EXIT_FAILURE, errno, "cannot convert expression");

  /* Run the tests.  */
  printf ("\nTest \"%s\" with multi-byte locale\n", expr);
  result |= run_test (uexpr, umem, 2 * memlen - outlen, expected);

  return result;
}


static int
run_test (const char *expr, const char *mem, size_t memlen, int expected)
{
#ifdef _POSIX_CPUTIME
  struct timespec start;
  struct timespec finish;
#endif
  regex_t re;
  int err;
  size_t offset;
  int cnt;

#ifdef _POSIX_CPUTIME
  if (use_clock)
    use_clock = clock_gettime (cl, &start) == 0;
#endif

  err = regcomp (&re, expr, REG_NEWLINE);
  if (err != REG_NOERROR)
    {
      char buf[200];
      regerror (err, &re, buf, sizeof buf);
      error (EXIT_FAILURE, 0, "cannot compile expression: %s", buf);
    }

  cnt = 0;
  offset = 0;
  assert (mem[memlen] == '\0');
  while (offset < memlen)
    {
      regmatch_t ma[1];
      const char *sp;
      const char *ep;

      err = regexec (&re, mem + offset, 1, ma, 0);
      if (err == REG_NOMATCH)
	break;

      if (err != REG_NOERROR)
	{
	  char buf[200];
	  regerror (err, &re, buf, sizeof buf);
	  error (EXIT_FAILURE, 0, "cannot use expression: %s", buf);
	}

      assert (ma[0].rm_so >= 0);
      sp = mem + offset + ma[0].rm_so;
      while (sp > mem && sp[-1] != '\n')
	--sp;

      ep = mem + offset + ma[0].rm_so;
      while (*ep != '\0' && *ep != '\n')
	++ep;

      printf ("match %d: \"%.*s\"\n", ++cnt, (int) (ep - sp), sp);

      offset = ep + 1 - mem;
    }

  regfree (&re);

#ifdef _POSIX_CPUTIME
  if (use_clock)
    {
      use_clock = clock_gettime (cl, &finish) == 0;
      if (use_clock)
	{
	  if (finish.tv_nsec < start.tv_nsec)
	    {
	      finish.tv_nsec -= start.tv_nsec - 1000000000;
	      finish.tv_sec -= 1 + start.tv_sec;
	    }
	  else
	    {
	      finish.tv_nsec -= start.tv_nsec;
	      finish.tv_sec -= start.tv_sec;
	    }

	  printf ("elapsed time: %ld.%09ld sec\n",
		  finish.tv_sec, finish.tv_nsec);
	}
    }
#endif

  /* Return an error if the number of matches found is not match we
     expect.  */
  return cnt != expected;
}
