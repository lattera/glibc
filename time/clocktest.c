#include <stdio.h>
#include <time.h>

main ()
{
  volatile int i;
  double t1, t2, t;

  t1 = (double) clock ();
  for (i = 0; i < 100000; ++i) ;
  t2 = (double) clock ();

  t = (t2 - t1) / ((double) CLOCKS_PER_SEC);
  printf ("%f - %f = %f\n",t2,t1,t);
  return 0;
}
