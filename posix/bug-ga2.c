/* Test case by Sam Varshavchik <mrsam@courier-mta.com>.  */
#include <mcheck.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

int
main (void)
{
  struct addrinfo hints, *res;
  int i, ret;

  mtrace ();
  for (i = 0; i < 100; i++)
    {
      memset (&hints, 0, sizeof (hints));
      hints.ai_family = PF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;

      ret = getaddrinfo ("www.gnu.org", "http", &hints, &res);

      if (ret)
	{
	  printf ("%s\n", gai_strerror (ret));
	  return 1;
	}
      freeaddrinfo (res);
    }
  return 0;
}
