/*@group*/
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
/*@end group*/

/*@group*/
int
input_timeout (int filedes, unsigned int seconds)
{
  fd_set set;
  struct timeval timeout;
/*@end group*/

  /* Initialize the file descriptor set. */
  FD_ZERO (&set);
  FD_SET (filedes, &set);

  /* Initialize the timeout data structure. */
  timeout.tv_sec = seconds;
  timeout.tv_usec = 0;

/*@group*/
  /* @code{select} returns 0 if timeout, 1 if input available, -1 if error. */
  return TEMP_FAILURE_RETRY (select (FD_SETSIZE,
				     &set, NULL, NULL,
				     &timeout));
}
/*@end group*/

/*@group*/
int
main (void)
{
  fprintf (stderr, "select returned %d.\n",
	   input_timeout (STDIN_FILENO, 5));
  return 0;
}
/*@end group*/
