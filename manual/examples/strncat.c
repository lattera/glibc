#include <string.h>
#include <stdio.h>

#define SIZE 10

static char buffer[SIZE];

int
main (void)
{
  strncpy (buffer, "hello", SIZE);
  puts (buffer);
  strncat (buffer, ", world", SIZE - strlen (buffer) - 1);
  puts (buffer);
}
