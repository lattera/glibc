/* Test for ungetc bugs.  */

#include <stdio.h>

#define assert(x) \
  if (!(x)) \
    { \
      fputs ("test failed: " #x "\n", stderr); \
      retval = 1; \
      goto the_end; \
    }

int
main (int argc, char *argv[])
{
  char *name;
  FILE *fp = NULL;
  int retval = 0;
  int c;

  name = tmpnam (NULL);
  fp = fopen (name, "w");
  assert (fp != NULL)
  fputs ("bl", fp);
  fclose (fp);
  fp = NULL;

  fp = fopen (name, "r");
  assert (fp != NULL)
  assert (getc (fp) != EOF);
  assert ((c = getc (fp)) != EOF);
  assert (getc (fp) == EOF);
  assert (ungetc (c, fp) == c);
  assert (feof (fp) == 0);

the_end:
  if (fp != NULL)
    fclose (fp);
  unlink (name);

  return retval;
}
