#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>


static int
do_test (void)
{
  int result = 0;
  char buf[100];
  FILE *fp = fmemopen (buf, sizeof (buf), "w");
  if (fp == NULL)
    {
      puts ("fmemopen failed");
      return 0;
    }
  static const char str[] = "hello world";
#define nstr (sizeof (str) - 1)
  fputs (str, fp);
  off_t o = ftello (fp);
  if (o != nstr)
    {
      printf ("first ftello returned %ld, expected %zu\n", o, nstr);
      result = 1;
    }
  rewind (fp);
  o = ftello (fp);
  if (o != 0)
    {
      printf ("second ftello returned %ld, expected 0\n", o);
      result = 1;
    }
  if (fseeko (fp, 0, SEEK_END) != 0)
    {
      puts ("fseeko failed");
      return 1;
    }
  o = ftello (fp);
  if (o != nstr)
    {
      printf ("third ftello returned %ld, expected %zu\n", o, nstr);
      result = 1;
    }
  rewind (fp);
  static const char str2[] = "just hello";
#define nstr2 (sizeof (str2) - 1)
  assert (nstr2 < nstr);
  fputs (str2, fp);
  o = ftello (fp);
  if (o != nstr2)
    {
      printf ("fourth ftello returned %ld, expected %zu\n", o, nstr2);
      result = 1;
    }
  fclose (fp);
  static const char str3[] = "just hellod";
  if (strcmp (buf, str3) != 0)
    {
      printf ("final string is \"%s\", expected \"%s\"\n",
              buf, str3);
      result = 1;
    }
  return result;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
