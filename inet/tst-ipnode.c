#include <netdb.h>
#include <stdio.h>

/* The unspecified IPv6 address.  */
struct in6_addr anyv6 = IN6ADDR_ANY_INIT;

int
main (void)
{
  int errors = 0;
  int errval;

  /* Test the unspecifed IPv6 address.  */
  errval = 0x3453456;
  if (getipnodebyaddr (&anyv6, sizeof (anyv6), AF_INET6, &errval) != NULL
      || errval != HOST_NOT_FOUND)
    {
      puts ("getipnodenyaddr(in6addr_any,...) != NULL");
      ++errors;
    }

  return errors != NULL;
}
