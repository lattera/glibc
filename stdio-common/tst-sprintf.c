#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int
main (void)
{
  char buf[100];
  int result = 0;

  if (sprintf (buf, "%.0ls", L"foo") != 0
      || strlen (buf) != 0)
    {
      puts ("sprintf (buf, \"%.0ls\", L\"foo\") produced some output");
      result = 1;
    }

#define SIZE (1024*70000)
#define STR(x) #x

  char *dst = malloc (SIZE + 1);

  if (dst == NULL)
    {
      puts ("memory allocation failure");
      result = 1;
    }
  else
    {
      sprintf (dst, "%*s", SIZE, "");
      if (strnlen (dst, SIZE + 1) != SIZE)
	{
	  puts ("sprintf (dst, \"%*s\", " STR(SIZE) ", \"\") did not produce enough output");
	  result = 1;
	}
      free (dst);
    }

  return result;
}
