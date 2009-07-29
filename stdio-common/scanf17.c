#undef _GNU_SOURCE
#define _XOPEN_SOURCE 600
#undef _LIBC
/* The following macro definitions are a hack.  They word around disabling
   the GNU extension while still using a few internal headers.  */
#define u_char unsigned char
#define u_short unsigned short
#define u_int unsigned int
#define u_long unsigned long
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define FAIL() \
  do {							\
    result = 1;						\
    printf ("test at line %d failed\n", __LINE__);	\
  } while (0)

static int
xsscanf (const char *str, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  int ret = vsscanf (str, fmt, ap);
  va_end (ap);
  return ret;
}

static int
xscanf (const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  int ret = vscanf (fmt, ap);
  va_end (ap);
  return ret;
}

static int
xfscanf (FILE *f, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  int ret = vfscanf (f, fmt, ap);
  va_end (ap);
  return ret;
}

int
main (void)
{
  float f;
  double d;
  char c[8];
  int result = 0;

  if (xsscanf (" 0.25s x", "%e%3c", &f, c) != 2)
    FAIL ();
  else if (f != 0.25 || memcmp (c, "s x", 3) != 0)
    FAIL ();
  if (xsscanf (" 1.25s x", "%as%2c", &f, c) != 2)
    FAIL ();
  else if (f != 1.25 || memcmp (c, " x", 2) != 0)
    FAIL ();
  if (xsscanf (" 2.25s x", "%las%2c", &d, c) != 2)
    FAIL ();
  else if (d != 2.25 || memcmp (c, " x", 2) != 0)
    FAIL ();
  if (xsscanf (" 3.25S x", "%4aS%2c", &f, c) != 2)
    FAIL ();
  else if (f != 3.25 || memcmp (c, " x", 2) != 0)
    FAIL ();
  if (xsscanf (" 4.25[0-9.] x", "%a[0-9.]%2c", &f, c) != 2)
    FAIL ();
  else if (f != 4.25 || memcmp (c, " x", 2) != 0)
    FAIL ();
  if (xsscanf (" 5.25[0-9.] x", "%la[0-9.]%2c", &d, c) != 2)
    FAIL ();
  else if (d != 5.25 || memcmp (c, " x", 2) != 0)
    FAIL ();

  const char *tmpdir = getenv ("TMPDIR");
  if (tmpdir == NULL || tmpdir[0] == '\0')
    tmpdir = "/tmp";

  char fname[strlen (tmpdir) + sizeof "/tst-scanf17.XXXXXX"];
  sprintf (fname, "%s/tst-scanf17.XXXXXX", tmpdir);
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
      if (xfscanf (fp, "%as%2c", &f, c) != 2)
	FAIL ();
      else if (f != 1.25 || memcmp (c, " x", 2) != 0)
	FAIL ();

      if (freopen (fname, "r", stdin) == NULL)
	FAIL ();
      else
	{
	  if (xscanf ("%as%2c", &f, c) != 2)
	    FAIL ();
	  else if (f != 1.25 || memcmp (c, " x", 2) != 0)
	    FAIL ();
	}

      fclose (fp);
    }

  remove (fname);

  return result;
}
