/* setjmp vs alloca test case.  Exercised bug on sparc.  */

#include <stdio.h>
#include <setjmp.h>
#include <alloca.h>

void
sub5 (jmp_buf buf)
{
  longjmp (buf, 1);
}

int
main (void)
{
  jmp_buf buf;
  char *foo;
  int arr[100];

  arr[77] = 76;
  if (setjmp (buf))
    {
      printf ("made it ok; %d\n", arr[77]);
      exit (0);
    }

  foo = (char *) alloca (128);
  sub5 (buf);

  /* NOTREACHED */
  return 1;
}
