#include <stdio.h>
#include <wchar.h>

int
main (int argc, char *argv[])
{
  wchar_t buf[100];
  int n;
  int result = 0;

  puts ("test 1");
  n = swprintf (buf, sizeof (buf) / sizeof (buf[0]), L"Hello %s", "world");
  if (n != 11)
    {
      printf ("incorrect return value: %d instead of 11\n", n);
      result = 1;
    }

  if (wcscmp (buf, L"Hello world") != 0)
    {
      printf ("incorrect string: L\"%ls\" instead of L\"Hello world\"\n", buf);
      result = 1;
    }

  puts ("test 2");
  n = swprintf (buf, sizeof (buf) / sizeof (buf[0]), L"Is this >%g< 3.1?",
		3.1);
  if (n != 18)
{
      printf ("incorrect return value: %d instead of 18\n", n);
      result = 1;
    }

  if (wcscmp (buf, L"Is this >3.1< 3.1?") != 0)
    {
      printf ("incorrect string: L\"%ls\" instead of L\"Is this >3.1< 3.1?\"\n",
	      buf);
      result = 1;
    }

  return result;
}
