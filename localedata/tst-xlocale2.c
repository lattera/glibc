#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>


static int do_test (__locale_t l);

int
main (void)
{
  __locale_t l;
  __locale_t l2;
  int result;

  l = __newlocale (1 << LC_ALL, "de_DE.ISO-8859-1", NULL);
  if (l == NULL)
    {
      printf ("__newlocale failed: %m\n");
      exit (EXIT_FAILURE);
    }
  puts ("Running tests of created locale");
  result = do_test (l);

  l2 = __duplocale (l);
  if (l2 == NULL)
    {
      printf ("__duplocale failed: %m\n");
      exit (EXIT_FAILURE);
    }
  __freelocale (l);
  puts ("Running tests of duplicated locale");
  result |= do_test (l2);

  return result;
}


static const char str[] = "0123456789abcdef ABCDEF ghijklmnopqrstuvwxyzäÄöÖüÜ";
static const char exd[] = "11111111110000000000000000000000000000000000000000";
static const char exa[] = "00000000001111110111111011111111111111111111111111";
static const char exx[] = "11111111111111110111111000000000000000000000000000";


static int
do_test (__locale_t l)
{
  int result = 0;
  int n;

#define DO_TEST(TEST, RES) \
  for (n = 0; n < sizeof (str) - 1; ++n)				      \
    if ('0' + (TEST (str[n], l) != 0) != RES[n])			      \
      {									      \
	printf ("%s(%c) failed\n", #TEST, str[n]);			      \
	result = 1;							      \
      }

  DO_TEST (__isdigit_l, exd);
  DO_TEST (__isalpha_l, exa);
  DO_TEST (__isxdigit_l, exx);

  return result;
}
