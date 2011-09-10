/* Test case for early TLS initialization in dynamic linker.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAGIC1 0xabcdef72
#define MAGIC2 0xd8675309
static __thread unsigned int magic[] = { MAGIC1, MAGIC2 };

#undef calloc

/* This calloc definition will be called by the dynamic linker itself.
   We test that it has initialized our TLS block by the time it does so.  */

void *
calloc (size_t n, size_t m)
{
  if (magic[0] != MAGIC1 || magic[1] != MAGIC2)
    {
      printf ("{%x, %x} != {%x, %x}\n", magic[0], magic[1], MAGIC1, MAGIC2);
      abort ();
    }
  magic[0] = MAGIC2;
  magic[1] = MAGIC1;

  n *= m;
  void *ptr = malloc (n);
  if (ptr != NULL)
    memset (ptr, '\0', n);
  return ptr;
}

int
main (void)
{
  if (magic[1] != MAGIC1 || magic[0] != MAGIC2)
    {
      printf ("{%x, %x} != {%x, %x}\n", magic[0], magic[1], MAGIC2, MAGIC1);
      return 1;
    }

  return 0;
}
