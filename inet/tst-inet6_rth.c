#include <stdio.h>
#include <netinet/ip6.h>

static int
do_test (void)
{
  int res = 0;
  char buf[1000];
  void *p = inet6_rth_init (buf, 24, IPV6_RTHDR_TYPE_0, 0);
  if (p == NULL)
    {
      puts ("first inet6_rth_init failed");
      res = 1;
    }
  else if (inet6_rth_add (p, &in6addr_any) == 0)
    {
      puts ("first inet6_rth_add succeeded");
      res = 1;
    }

  p = inet6_rth_init (buf, 24, IPV6_RTHDR_TYPE_0, 1);
  if (p == NULL)
    {
      puts ("second inet6_rth_init failed");
      res = 1;
    }
  else if (inet6_rth_add (p, &in6addr_any) != 0)
    {
      puts ("second inet6_rth_add failed");
      res = 1;
    }
  return res;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
