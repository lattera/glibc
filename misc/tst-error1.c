#include <error.h>
#include <mcheck.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

static int
do_test (int argc, char *argv[])
{
  mtrace ();
  (void) freopen (argc == 1 ? "/dev/stdout" : argv[1], "a", stderr);
  /* Orient the stream.  */
  fwprintf (stderr, L"hello world\n");
  char buf[20000];
  static const char str[] = "hello world! ";
  for (int i = 0; i < 1000; ++i)
    memcpy (&buf[i * (sizeof (str) - 1)], str, sizeof (str));
  error (0, 0, str);
  error (0, 0, buf);
  error (0, 0, buf);
  error (0, 0, str);
  return 0;
}

#define TEST_FUNCTION do_test (argc, argv)
#include "../test-skeleton.c"
