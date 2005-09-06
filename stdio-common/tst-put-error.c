#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


static int
do_test (void)
{
  char tmpl[] = "/tmp/tst-put-error.XXXXXX";
  int fd = mkstemp (tmpl);
  if (fd == -1)
    error (EXIT_FAILURE, errno, "cannot create temporary file");
  FILE *fp = fdopen (fd, "w");
  if (fp == NULL)
    error (EXIT_FAILURE, errno, "fdopen");
  setlinebuf (fp);
  close (fd);
  unlink (tmpl);
  int n = fprintf (fp, "hello world\n");
  printf ("fprintf = %d\n", n);
  if (n >= 0)
    error (EXIT_FAILURE, 0, "first fprintf succeeded");
  n = fprintf (fp, "hello world\n");
  printf ("fprintf = %d\n", n);
  if (n >= 0)
    error (EXIT_FAILURE, 0, "second fprintf succeeded");
  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
