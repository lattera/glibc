/* Test program for stpncpy reading off the end of the source string.  */

#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

static int do_test (void);
#define TEST_FUNCTION do_test ()
#include <test-skeleton.c>

static int
do_test (void)
{
  /* We get two pages of memory and then protect the second one so
     we are sure to fault if we access past the end of the first page.
     Then we test the odd-size string ending just on the page boundary.  */

  static const char test_string[] = "Seven.";
  const size_t pagesz = getpagesize ();
  char *buf = mmap (NULL, pagesz * 2,
		    PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0);
  char *s1, *r;
  volatile size_t len = sizeof test_string;

  if (buf == MAP_FAILED)
    {
      perror ("mmap");
      return 1;
    }
  if (mprotect (buf + pagesz, pagesz, PROT_NONE) != 0)
    {
      perror ("mprotect");
      return 2;
    }

  s1 = buf + pagesz - sizeof test_string;
  memcpy (s1, test_string, sizeof test_string);

  r = stpncpy (buf, s1, len);
  if (r != buf + len - 1)
    {
      printf ("r = buf + %d != %zu\n", r - buf, len - 1);
      return 3;
    }
  r = stpncpy (s1, buf, len);
  if (r != s1 + len - 1)
    {
      printf ("r = s1 + %d != %zu\n", r - s1, len - 1);
      return 3;
    }

  return 0;
}
