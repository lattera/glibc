#include <stdio.h>
#include <string.h>

int
main(int argc, char *argv[])
{
  static const size_t lens[] = { 0, 1, 0, 2, 0, 1, 0, 3,
				 0, 1, 0, 2, 0, 1, 0, 4 };
  char buf[24];
  size_t words;

  for (words = 0; words < 4; ++words)
    {
      size_t last;
      memset (buf, 'a', words * 4);

      for (last = 0; last < 16; ++last)
        {
	  buf[words * 4 + 0] = (last & 1) != 0 ? 'b' : '\0';
	  buf[words * 4 + 1] = (last & 2) != 0 ? 'c' : '\0';
	  buf[words * 4 + 2] = (last & 4) != 0 ? 'd' : '\0';
	  buf[words * 4 + 3] = (last & 8) != 0 ? 'e' : '\0';
	  buf[words * 4 + 4] = '\0';

	  if (strlen (buf) != words * 4 + lens[last])
	    {
	      printf ("failed for words=%d and last=%d\n", words, last);
	      return 1;
	    }
        }
    }
  return 0;
}
