#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

enum { buf_size = 16 };
static char buf[buf_size] = "323543357756889";

int
main (int argc, char *argv[])
{
  struct in_addr addr;
  int result = 0;

  if (inet_aton (buf, &addr) != 0)
    {
      printf ("%s is seen as a valid IP address\n", buf);
      result = 1;
    }

  return result;
}
