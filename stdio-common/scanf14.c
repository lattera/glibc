#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <libc-internal.h>

#define FAIL() \
  do {							\
    result = 1;						\
    printf ("test at line %d failed\n", __LINE__);	\
  } while (0)

int
main (void)
{
  wchar_t *lsp;
  char *sp;
  float f;
  double d;
  char c[8];
  int result = 0;

  if (sscanf (" 0.25s x", "%e%3c", &f, c) != 2)
    FAIL ();
  else if (f != 0.25 || memcmp (c, "s x", 3) != 0)
    FAIL ();
  /* GCC in C99 mode treats %a as the C99 format expecting float *,
     but glibc with _GNU_SOURCE treats %as as the GNU allocation
     extension, so resulting in "warning: format '%a' expects argument
     of type 'float *', but argument 3 has type 'char **'".  This
     applies to the other %as, %aS and %a[] formats below as well.  */
  DIAG_PUSH_NEEDS_COMMENT;
  DIAG_IGNORE_NEEDS_COMMENT (4.9, "-Wformat");
  if (sscanf (" 1.25s x", "%as%2c", &sp, c) != 2)
    FAIL ();
  else
    {
      if (strcmp (sp, "1.25s") != 0 || memcmp (c, " x", 2) != 0)
	FAIL ();
      memset (sp, 'x', sizeof "1.25s");
      free (sp);
    }
  DIAG_POP_NEEDS_COMMENT;
  if (sscanf (" 2.25s x", "%las%2c", &d, c) != 2)
    FAIL ();
  else if (d != 2.25 || memcmp (c, " x", 2) != 0)
    FAIL ();
  /* See explanation above.  */
  DIAG_PUSH_NEEDS_COMMENT;
  DIAG_IGNORE_NEEDS_COMMENT (4.9, "-Wformat");
  if (sscanf (" 3.25S x", "%4aS%3c", &lsp, c) != 2)
    FAIL ();
  else
    {
      if (wcscmp (lsp, L"3.25") != 0 || memcmp (c, "S x", 3) != 0)
	FAIL ();
      memset (lsp, 'x', sizeof L"3.25");
      free (lsp);
    }
  if (sscanf ("4.25[0-9.] x", "%a[0-9.]%8c", &sp, c) != 2)
    FAIL ();
  else
    {
      if (strcmp (sp, "4.25") != 0 || memcmp (c, "[0-9.] x", 8) != 0)
	FAIL ();
      memset (sp, 'x', sizeof "4.25");
      free (sp);
    }
  DIAG_POP_NEEDS_COMMENT;
  if (sscanf ("5.25[0-9.] x", "%la[0-9.]%2c", &d, c) != 2)
    FAIL ();
  else if (d != 5.25 || memcmp (c, " x", 2) != 0)
    FAIL ();

  const char *tmpdir = getenv ("TMPDIR");
  if (tmpdir == NULL || tmpdir[0] == '\0')
    tmpdir = "/tmp";

  char fname[strlen (tmpdir) + sizeof "/tst-scanf14.XXXXXX"];
  sprintf (fname, "%s/tst-scanf14.XXXXXX", tmpdir);
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
      /* See explanation above.  */
      DIAG_PUSH_NEEDS_COMMENT;
      DIAG_IGNORE_NEEDS_COMMENT (4.9, "-Wformat");
      if (fscanf (fp, "%as%2c", &sp, c) != 2)
	FAIL ();
      else
	{
	  if (strcmp (sp, "1.25s") != 0 || memcmp (c, " x", 2) != 0)
	    FAIL ();
	  memset (sp, 'x', sizeof "1.25s");
	  free (sp);
	}
      DIAG_POP_NEEDS_COMMENT;

      if (freopen (fname, "r", stdin) == NULL)
	FAIL ();
      else
	{
	  /* See explanation above.  */
	  DIAG_PUSH_NEEDS_COMMENT;
	  DIAG_IGNORE_NEEDS_COMMENT (4.9, "-Wformat");
	  if (scanf ("%as%2c", &sp, c) != 2)
	    FAIL ();
	  else
	    {
	      if (strcmp (sp, "1.25s") != 0 || memcmp (c, " x", 2) != 0)
		FAIL ();
	      memset (sp, 'x', sizeof "1.25s");
	      free (sp);
	    }
	  DIAG_POP_NEEDS_COMMENT;
	}

      fclose (fp);
    }

  remove (fname);

  return result;
}
