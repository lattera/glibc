#undef _GNU_SOURCE
#define _XOPEN_SOURCE 600
#undef _LIBC
#undef _IO_MTSAFE_IO
/* The following macro definitions are a hack.  They word around disabling
   the GNU extension while still using a few internal headers.  */
#define u_char unsigned char
#define u_short unsigned short
#define u_int unsigned int
#define u_long unsigned long
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define FAIL() \
  do {							\
    result = 1;						\
    printf ("test at line %d failed\n", __LINE__);	\
  } while (0)

int
main (void)
{
  float f;
  double d;
  char c[8];
  int result = 0;

  if (sscanf (" 0.25s x", "%e%3c", &f, c) != 2)
    FAIL ();
  else if (f != 0.25 || memcmp (c, "s x", 3) != 0)
    FAIL ();
  if (sscanf (" 1.25s x", "%as%2c", &f, c) != 2)
    FAIL ();
  else if (f != 1.25 || memcmp (c, " x", 2) != 0)
    FAIL ();
  if (sscanf (" 2.25s x", "%las%2c", &d, c) != 2)
    FAIL ();
  else if (d != 2.25 || memcmp (c, " x", 2) != 0)
    FAIL ();
  if (sscanf (" 3.25S x", "%4aS%2c", &f, c) != 2)
    FAIL ();
  else if (f != 3.25 || memcmp (c, " x", 2) != 0)
    FAIL ();
  if (sscanf (" 4.25[0-9.] x", "%a[0-9.]%2c", &f, c) != 2)
    FAIL ();
  else if (f != 4.25 || memcmp (c, " x", 2) != 0)
    FAIL ();
  if (sscanf (" 5.25[0-9.] x", "%la[0-9.]%2c", &d, c) != 2)
    FAIL ();
  else if (d != 5.25 || memcmp (c, " x", 2) != 0)
    FAIL ();

  const char *tmpdir = getenv ("TMPDIR");
  if (tmpdir == NULL || tmpdir[0] == '\0')
    tmpdir = "/tmp";

  char fname[strlen (tmpdir) + sizeof "/tst-scanf15.XXXXXX"];
  sprintf (fname, "%s/tst-scanf15.XXXXXX", tmpdir);
  if (fname == NULL)
    FAIL ();

  /* Create a temporary file.   */
  int fd = mkstemp (fname);
  if (fd == -1)
    FAIL ();

  FILE *fp = fdopen (fd, "w+");
  if (fp == NULL)
    FAIL ();
  else
    {
      if (fputs (" 1.25s x", fp) == EOF)
	FAIL ();
      if (fseek (fp, 0, SEEK_SET) != 0)
	FAIL ();
      if (fscanf (fp, "%as%2c", &f, c) != 2)
	FAIL ();
      else if (f != 1.25 || memcmp (c, " x", 2) != 0)
	FAIL ();

      if (freopen (fname, "r", stdin) == NULL)
	FAIL ();
      else
	{
	  if (scanf ("%as%2c", &f, c) != 2)
	    FAIL ();
	  else if (f != 1.25 || memcmp (c, " x", 2) != 0)
	    FAIL ();
	}

      fclose (fp);
    }

  remove (fname);

  return result;
}
