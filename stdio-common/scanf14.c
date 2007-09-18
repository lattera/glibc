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
  if (sscanf (" 1.25s x", "%as%2c", &sp, c) != 2)
    FAIL ();
  else
    {
      if (strcmp (sp, "1.25s") != 0 || memcmp (c, " x", 2) != 0)
	FAIL ();
      memset (sp, 'x', sizeof "1.25s");
      free (sp);
    }
  if (sscanf (" 2.25s x", "%las%2c", &d, c) != 2)
    FAIL ();
  else if (d != 2.25 || memcmp (c, " x", 2) != 0)
    FAIL ();
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
  if (sscanf ("5.25[0-9.] x", "%la[0-9.]%2c", &d, c) != 2)
    FAIL ();
  else if (d != 5.25 || memcmp (c, " x", 2) != 0)
    FAIL ();

  return result;
}
