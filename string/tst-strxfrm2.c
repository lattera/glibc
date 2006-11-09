#include <locale.h>
#include <stdio.h>
#include <string.h>

static int
do_test (void)
{
  int res = 0;

  char buf[10];
  size_t l1 = strxfrm (NULL, "ab", 0);
  size_t l2 = strxfrm (buf, "ab", 1);
  size_t l3 = strxfrm (buf, "ab", sizeof (buf));

  if (l1 != l2 || l1 != l3)
    {
      puts ("C locale test failed");
      res = 1;
    }

  if (setlocale (LC_ALL, "de_DE.UTF-8") == NULL)
    {
      puts ("setlocale failed");
      res = 1;
    }
  else
    {
      l1 = strxfrm (NULL, "ab", 0);
      l2 = strxfrm (buf, "ab", 1);
      l3 = strxfrm (buf, "ab", sizeof (buf));

      if (l1 != l2 || l1 != l3)
	{
	  puts ("UTF-8 locale test failed");
	  res = 1;
	}
    }

  return res;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
