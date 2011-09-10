#include "tst-tls10.h"

#define CHECK(N, S)					\
  p = &a##N;						\
  if (p->a != S || p->b != S + 1 || p->c != S + 2)	\
    abort ()

int
main (void)
{
  struct A *p;
  check1 ();
  CHECK (1, 4);
  CHECK (2, 7);

  exit (0);
}
