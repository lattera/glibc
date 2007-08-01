#include <locale.h>
#include <time.h>
#include <stdio.h>

static int
do_test (void)
{
  if (setlocale (LC_ALL, "vi_VN.TCVN5712-1") == NULL)
    {
      puts ("cannot set locale");
      return 1;
    }
  struct tm tm;
  /* This is November in Vietnamese encoded using TCVN5712-1.  */
  static const char s[] = "\
\x54\x68\xb8\x6e\x67\x20\x6d\xad\xea\x69\x20\x6d\xe9\x74";
  char *r = strptime (s, "%b", &tm);
  printf ("r = %p, r-s = %tu, tm.tm_mon = %d\n", r, r - s, tm.tm_mon);
  return r == NULL || r - s != 14 || tm.tm_mon != 10;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
