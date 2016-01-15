/* Test program from Paul Eggert and Tony Leneis.  */

#include <limits.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

/* True if the arithmetic type T is signed.  */
#define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))

/* The maximum and minimum values for the integer type T.  These
   macros have undefined behavior if T is signed and has padding bits.
   If this is a problem for you, please let us know how to fix it for
   your host.  */
#define TYPE_MINIMUM(t) \
  ((t) (! TYPE_SIGNED (t) \
	? (t) 0 \
	: ~ TYPE_MAXIMUM (t)))
#define TYPE_MAXIMUM(t) \
  ((t) (! TYPE_SIGNED (t) \
	? (t) -1 \
	: ((((t) 1 << (sizeof (t) * CHAR_BIT - 2)) - 1) * 2 + 1)))

#ifndef TIME_T_MIN
# define TIME_T_MIN TYPE_MINIMUM (time_t)
#endif
#ifndef TIME_T_MAX
# define TIME_T_MAX TYPE_MAXIMUM (time_t)
#endif

/* Values we'll use to set the TZ environment variable.  */
static const char *tz_strings[] =
  {
    (const char *) 0, "GMT0", "JST-9",
    "EST+3EDT+2,M10.1.0/00:00:00,M2.3.0/00:00:00"
  };
#define N_STRINGS ((int) (sizeof (tz_strings) / sizeof (tz_strings[0])))

/* Fail if mktime fails to convert a date in the spring-forward gap.
   Based on a problem report from Andreas Jaeger.  */
static void
spring_forward_gap (void)
{
  /* glibc (up to about 1998-10-07) failed this test. */
  struct tm tm;

  /* Use the portable POSIX.1 specification "TZ=PST8PDT,M4.1.0,M10.5.0"
     instead of "TZ=America/Vancouver" in order to detect the bug even
     on systems that don't support the Olson extension, or don't have the
     full zoneinfo tables installed.  */
  setenv ("TZ", "PST8PDT,M4.1.0,M10.5.0", 1);

  tm.tm_year = 98;
  tm.tm_mon = 3;
  tm.tm_mday = 5;
  tm.tm_hour = 2;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  tm.tm_isdst = -1;
  if (mktime (&tm) == (time_t)-1)
    exit (1);
}

static void
mktime_test1 (time_t now)
{
  struct tm *lt = localtime (&now);
  if (lt && mktime (lt) != now)
    exit (2);
}

static void
mktime_test (time_t now)
{
  mktime_test1 (now);
  mktime_test1 ((time_t) (TIME_T_MAX - now));
  mktime_test1 ((time_t) (TIME_T_MIN + now));
}

static void
irix_6_4_bug (void)
{
  /* Based on code from Ariel Faigon.  */
  struct tm tm;
  tm.tm_year = 96;
  tm.tm_mon = 3;
  tm.tm_mday = 0;
  tm.tm_hour = 0;
  tm.tm_min = 0;
  tm.tm_sec = 0;
  tm.tm_isdst = -1;
  mktime (&tm);
  if (tm.tm_mon != 2 || tm.tm_mday != 31)
    exit (3);
}

static void
bigtime_test (int j)
{
  struct tm tm;
  time_t now;
  tm.tm_year = tm.tm_mon = tm.tm_mday = tm.tm_hour = tm.tm_min = tm.tm_sec = j;
  tm.tm_isdst = -1;
  now = mktime (&tm);
  if (now != (time_t) -1)
    {
      struct tm *lt = localtime (&now);
      if (! (lt
	     && lt->tm_year == tm.tm_year
	     && lt->tm_mon == tm.tm_mon
	     && lt->tm_mday == tm.tm_mday
	     && lt->tm_hour == tm.tm_hour
	     && lt->tm_min == tm.tm_min
	     && lt->tm_sec == tm.tm_sec
	     && lt->tm_yday == tm.tm_yday
	     && lt->tm_wday == tm.tm_wday
	     && ((lt->tm_isdst < 0 ? -1 : 0 < lt->tm_isdst)
		  == (tm.tm_isdst < 0 ? -1 : 0 < tm.tm_isdst))))
	exit (4);
    }
}

static int
do_test (void)
{
  time_t t, delta;
  int i;
  unsigned int j;

  setenv ("TZ", "America/Sao_Paulo", 1);
  /* This test makes some buggy mktime implementations loop.
     Give up after 60 seconds; a mktime slower than that
     isn't worth using anyway.  */
  alarm (60);

  delta = TIME_T_MAX / 997; /* a suitable prime number */
  for (i = 0; i < N_STRINGS; i++)
    {
      if (tz_strings[i])
	setenv ("TZ", tz_strings[i], 1);

      for (t = 0; t <= TIME_T_MAX - delta; t += delta)
	mktime_test (t);
      mktime_test ((time_t) 1);
      mktime_test ((time_t) (60 * 60));
      mktime_test ((time_t) (60 * 60 * 24));

      for (j = 1; j <= INT_MAX; j *= 2)
	bigtime_test (j);
      bigtime_test (j - 1);
    }
  irix_6_4_bug ();
  spring_forward_gap ();
  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
