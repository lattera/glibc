#include <stdio.h>
#include <unistd.h>
#include <error.h>
#include <errno.h>
#include <sys/wait.h>

void noop (void);

int
main (void)
{
  int pid;

  printf ("Before vfork\n");
  fflush (stdout);
  pid = vfork ();
  if (pid == 0)
    {
      /* This will clobber the return pc from vfork in the parent on
	 machines where it is stored on the stack, if vfork wasn't
	 implemented correctly, */
      noop ();
      _exit (2);
    }
  else if (pid < 0)
    error (1, errno, "vfork");
  printf ("After vfork (parent)\n");
  wait (0);
  exit (0);
}

void
noop ()
{
}
