#include <stdio.h>
#include <string.h>
#include <time.h>

int
main (void)
{
  struct tm time_str;
  char daybuf[20];
  int result;

  time_str.tm_year = 2001 - 1900;
  time_str.tm_mon = 7 - 1;
  time_str.tm_mday = 4;
  time_str.tm_hour = 0;
  time_str.tm_min = 0;
  time_str.tm_sec = 1;
  time_str.tm_isdst = -1;

  if (mktime (&time_str) == -1)
    {
      (void) puts ("-unknown-");
      result = 1;
    }
  else
    {
      (void) strftime (daybuf, sizeof (daybuf), "%A", &time_str);
      (void) puts (daybuf);
      result = strcmp (daybuf, "Wednesday") != 0;
    }

  return result;
}
