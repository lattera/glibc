#include <fenv.h>
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
  feclearexcept (FE_ALL_EXCEPT);
  if (nextafterf (m, i) != i)
    {
      puts ("nextafterf+ failed");
      ++result;
    }
  if (fetestexcept (FE_OVERFLOW) == 0)
    {
      puts ("nextafterf+ did not overflow");
      ++result;
    }
  feclearexcept (FE_ALL_EXCEPT);
  if (nextafterf (-m, -i) != -i)
    {
      puts ("nextafterf- failed");
      ++result;
    }
  if (fetestexcept (FE_OVERFLOW) == 0)
    {
      puts ("nextafterf- did not overflow");
      ++result;
    }

  double di = INFINITY;
  double dm = DBL_MAX;
  feclearexcept (FE_ALL_EXCEPT);
  if (nextafter (dm, di) != di)
    {
      puts ("nextafter+ failed");
      ++result;
    }
  if (fetestexcept (FE_OVERFLOW) == 0)
    {
      puts ("nextafter+ did not overflow");
      ++result;
    }
  feclearexcept (FE_ALL_EXCEPT);
  if (nextafter (-dm, -di) != -di)
    {
      puts ("nextafter failed");
      ++result;
    }
  if (fetestexcept (FE_OVERFLOW) == 0)
    {
      puts ("nextafter- did not overflow");
      ++result;
    }

  return result;
}
