#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


static int do_test (void);
#define TEST_FUNCTION do_test ()
#include <test-skeleton.c>


static int
do_test (void)
{
  char *buf;
  int fd;
  FILE *fp;

  buf = (char *) malloc (strlen (test_dir) + sizeof "/tst-eof.XXXXXX");
  if (buf == NULL)
    {
      printf ("cannot allocate memory: %m\n");
      return 1;
    }
  stpcpy (stpcpy (buf, test_dir), "/tst-eof.XXXXXX");

  fd = mkstemp (buf);
  if (fd == -1)
    {
      printf ("cannot open temporary file: %m\n");
      return 1;
    }

  /* Make sure it gets removed.  */
  add_temp_file (buf);

  if (write (fd, "some string\n", 12) != 12)
    {
      printf ("cannot write temporary file: %m\n");
      return 1;
    }

  if (lseek (fd, 0, SEEK_SET) == (off_t) -1)
    {
      printf ("cannot reposition temporary file: %m\n");
      return 1;
    }

  fp = fdopen (fd, "r");
  if (fp == NULL)
    {
      printf ("cannot create stream: %m\n");
      return 1;
    }

  if (feof (fp))
    {
      puts ("EOF set after fdopen");
      return 1;
    }

  if (fread (buf, 1, 20, fp) != 12)
    {
      puts ("didn't read the correct number of bytes");
      return 1;
    }

  if (! feof (fp))
    {
      puts ("EOF not set after fread");
      return 1;
    }

  fclose (fp);

  return 0;
}
