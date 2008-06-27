#include <locale.h>
#include <time.h>
#include <stdio.h>
#include <string.h>

static int
do_test (void)
{
  int result = 0;

  if (setlocale (LC_ALL, "vi_VN.TCVN5712-1") == NULL)
    {
      puts ("cannot set locale");
      return 1;
    }
  struct tm tm;
  memset (&tm, '\0', sizeof (tm));
  /* This is November in Vietnamese encoded using TCVN5712-1.  */
  static const char s[] = "\
\x54\x68\xb8\x6e\x67\x20\x6d\xad\xea\x69\x20\x6d\xe9\x74\0";
  char *r = strptime (s, "%b", &tm);
  printf ("r = %p, r-s = %tu, tm.tm_mon = %d\n", r, r - s, tm.tm_mon);
  result = r == NULL || r - s != 14 || tm.tm_mon != 10;

  if (setlocale (LC_ALL, "ja_JP.UTF-8") == NULL)
    {
      puts ("cannot set locale");
      return 1;
    }
  static const char s2[] = "\
\x32\x35\x20\x30\x36\x20\xe5\xb9\xb3\xe6\x88\x90\x32\x30\0";
  memset (&tm, '\0', sizeof (tm));
  r = strptime (s2, "%d %m %EC%Ey", &tm);
  printf ("\
r = %p, r-s2 = %tu, tm.tm_mday = %d, tm.tm_mon = %d, tm.tm_year = %d\n",
	  r, r - s2, tm.tm_mday, tm.tm_mon, tm.tm_year);
  result |= (r == NULL || r - s2 != 14 || tm.tm_mday != 25 || tm.tm_mon != 5
	     || tm.tm_year != 108);

  static const char s3[] = "\
\x32\x35\x20\x30\x36\x20\xe5\xb9\xb3\xe6\x88\x90\x32\x30\xe5\xb9\xb4\0";
  memset (&tm, '\0', sizeof (tm));
  r = strptime (s3, "%d %m %EY", &tm);
  printf ("\
r = %p, r-s3 = %tu, tm.tm_mday = %d, tm.tm_mon = %d, tm.tm_year = %d\n",
	  r, r - s3, tm.tm_mday, tm.tm_mon, tm.tm_year);
  result |= (r == NULL || r - s3 != 17 || tm.tm_mday != 25 || tm.tm_mon != 5
	     || tm.tm_year != 108);

  return result;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
