/* Test for bug in fflush synchronization behavior.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main (void)
{
  FILE *f;
  off_t o;
  char buffer [1024];

  system ("echo 'From foo@bar.com' > test");
  f = fopen ("test", "r");
  fseek (f, 0, SEEK_END);
  o = ftello (f);
  fseek (f, 0, SEEK_SET);
  fflush (f);
  system ("echo 'From bar@baz.edu' >> test");
  fseek (f, o, SEEK_SET);
  if (fgets (buffer, 1024, f) == NULL)
    abort ();
  if (strncmp (buffer, "From ", 5) != 0)
    abort ();
  fclose (f);
  exit (0);
}
