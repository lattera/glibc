/*@group*/
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
/*@end group*/

/* When a @code{SIGUSR1} signal arrives, set this variable.  */
volatile sig_atomic_t usr_interrupt = 0;

void 
synch_signal (int sig)
{
  usr_interrupt = 1;
}

/* The child process executes this function. */
void 
child_function (void)
{
  /* Perform initialization. */
  printf ("I'm here!!!  My pid is %d.\n", (int) getpid ());

  /* Let parent know you're done. */
  kill (getppid (), SIGUSR1);

  /* Continue with execution. */
  puts ("Bye, now....");
  exit (0);
}

int
main (void)
{
  struct sigaction usr_action;
  sigset_t block_mask;
  pid_t child_id;

  /* Establish the signal handler. */
  sigfillset (&block_mask);
  usr_action.sa_handler = synch_signal;
  usr_action.sa_mask = block_mask;
  usr_action.sa_flags = 0;
  sigaction (SIGUSR1, &usr_action, NULL);

  /* Create the child process. */
  child_id = fork ();
  if (child_id == 0)
    child_function ();		/* Does not return.  */

/*@group*/
  /* Busy wait for the child to send a signal. */
  while (!usr_interrupt)
    ;
/*@end group*/

  /* Now continue execution. */
  puts ("That's all, folks!");

  return 0;
}
