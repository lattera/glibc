#include <termios.h>
#include <stdio.h>

ssize_t
my_getpass (char **lineptr, size_t *n, FILE *stream)
{
  struct termios old, new;
  int nread;

  /* Turn echoing off and fail if we can't.  */
  if (tcgetattr (fileno (stream), &old) != 0)
    return -1;
  new = old;
  new.c_lflag &= ~ECHO;
  if (tcsetattr (fileno (stream), TCSAFLUSH, &new) != 0)
    return -1;
  
  /* Read the password.  */
  nread = getline (lineptr, n, stream);

  /* Restore terminal.  */
  (void) tcsetattr (fileno (stream), TCSAFLUSH, &old);
  
  return nread;
}
