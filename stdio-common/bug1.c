#include <ansidecl.h>
#include <stdio.h>
#include <string.h>

int
DEFUN_VOID(main)
{
  char *bp;
  size_t size;
  FILE *stream;
  int lose = 0;

  stream = open_memstream (&bp, &size);
  fprintf (stream, "hello");
  fflush (stream);
  printf ("buf = %s, size = %d\n", bp, size);
  lose |= size != 5;
  lose |= strncmp (bp, "hello", size);
  fprintf (stream, ", world");
  fclose (stream);
  printf ("buf = %s, size = %d\n", bp, size);
  lose |= size != 12;
  lose |= strncmp (bp, "hello, world", 12);

  puts (lose ? "Test FAILED!" : "Test succeeded.");

  return lose;
}
