#include <mcheck.h>
#include <obstack.h>
#include <stdio.h>
#include <stdlib.h>


int
main (void)
{
  struct obstack ob;
  int n;

  mcheck_pedantic (NULL);

#define obstack_chunk_alloc malloc
#define obstack_chunk_free free

  obstack_init (&ob);

  for (n = 0; n < 10000; ++n)
    {
      mcheck_check_all ();
      obstack_printf (&ob, "%.*s%05d", 1 + n % 7, "foobarbaz", n);
    }

  /* And a final check.  */
  mcheck_check_all ();

  return 0;
}
