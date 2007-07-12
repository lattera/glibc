#include <stdlib.h>
#include <wchar.h>

int
main (void)
{
  mbstate_t x;
  return sizeof (x) - sizeof (mbstate_t);
}
