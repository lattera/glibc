#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdio.h>

int
main (void)
{
  int result = 0;

  float i = INFINITY;
  float m = FLT_MAX;
  if (nextafterf (m, i) != i)
    {
      puts ("nextafterf failed");
      ++result;
    }

  double di = INFINITY;
  double dm = DBL_MAX;
  if (nextafter (dm, di) != di)
    {
      puts ("nextafter failed");
      ++result;
    }

  return result;
}
