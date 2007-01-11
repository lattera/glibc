#include <fenv.h>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdio.h>

int
main (void)
{
  int result = 0;

  long double tl = (long double) FLT_MAX + 0x1.0p128L;
  float fi = INFINITY;
  float m = FLT_MAX;
  feclearexcept (FE_ALL_EXCEPT);
  if (nexttowardf (m, tl) != fi)
    {
      puts ("nexttowardf+ failed");
      ++result;
    }
  if (fetestexcept (FE_OVERFLOW) == 0)
    {
      puts ("nexttowardf+ did not overflow");
      ++result;
    }
  feclearexcept (FE_ALL_EXCEPT);
  if (nexttowardf (-m, -tl) != -fi)
    {
      puts ("nexttowardf- failed");
      ++result;
    }
  if (fetestexcept (FE_OVERFLOW) == 0)
    {
      puts ("nexttowardf- did not overflow");
      ++result;
    }

  tl = (long double) DBL_MAX + 1.0e305L;
  double di = INFINITY;
  double dm = DBL_MAX;
  feclearexcept (FE_ALL_EXCEPT);
  if (nexttoward (dm, tl) != di)
    {
      puts ("nexttoward+ failed");
      ++result;
    }
  if (fetestexcept (FE_OVERFLOW) == 0)
    {
      puts ("nexttoward+ did not overflow");
      ++result;
    }
  feclearexcept (FE_ALL_EXCEPT);
  if (nexttoward (-dm, -tl) != -di)
    {
      puts ("nexttoward- failed");
      ++result;
    }
  if (fetestexcept (FE_OVERFLOW) == 0)
    {
      puts ("nexttoward- did not overflow");
      ++result;
    }

  return result;
}
