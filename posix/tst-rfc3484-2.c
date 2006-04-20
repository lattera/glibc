#include <stdbool.h>
#include <stdio.h>

/* Internal definitions used in the libc code.  */
#define __getservbyname_r getservbyname_r
#define __socket socket
#define __getsockname getsockname
#define __inet_aton inet_aton
#define __gethostbyaddr_r gethostbyaddr_r
#define __gethostbyname2_r gethostbyname2_r

void
attribute_hidden
__check_pf (bool *p1, bool *p2, struct in6addrinfo **in6ai, size_t *in6ailen)
{
  *p1 = *p2 = true;
  *in6ai = NULL;
  *in6ailen = 0;
}
int
__idna_to_ascii_lz (const char *input, char **output, int flags)
{
  return 0;
}
int
__idna_to_unicode_lzlz (const char *input, char **output, int flags)
{
  return 0;
}

#include "../sysdeps/posix/getaddrinfo.c"

service_user *__nss_hosts_database attribute_hidden;


/* This is the beginning of the real test code.  The above defines
   (among other things) the function rfc3484_sort.  */


#if __BYTE_ORDER == __BIG_ENDIAN
# define h(n) n
#else
# define h(n) __bswap_constant_32 (n)
#endif


static int
do_test (void)
{
  struct sockaddr_in so1;
  so1.sin_family = AF_INET;
  so1.sin_addr.s_addr = h (0xc0a85f19);

  struct sockaddr_in sa1;
  sa1.sin_family = AF_INET;
  sa1.sin_addr.s_addr = h (0xe0a85f19);

  struct addrinfo ai1;
  ai1.ai_family = AF_INET;
  ai1.ai_addr = (struct sockaddr *) &sa1;

  struct sockaddr_in6 so2;
  so2.sin6_family = AF_INET6;
  so2.sin6_addr.s6_addr32[0] = h (0xfec01234);
  so2.sin6_addr.s6_addr32[1] = 1;
  so2.sin6_addr.s6_addr32[2] = 1;
  so2.sin6_addr.s6_addr32[3] = 1;

  struct sockaddr_in6 sa2;
  sa2.sin6_family = AF_INET6;
  sa2.sin6_addr.s6_addr32[0] = h (0x07d10001);
  sa2.sin6_addr.s6_addr32[1] = 1;
  sa2.sin6_addr.s6_addr32[2] = 1;
  sa2.sin6_addr.s6_addr32[3] = 1;

  struct addrinfo ai2;
  ai2.ai_family = AF_INET6;
  ai2.ai_addr = (struct sockaddr *) &sa2;


  struct sort_result results[2];

  results[0].dest_addr = &ai1;
  results[0].got_source_addr = true;
  results[0].source_addr_len = sizeof (so1);
  memcpy (&results[0].source_addr, &so1, sizeof (so1));

  results[1].dest_addr = &ai2;
  results[1].got_source_addr = true;
  results[1].source_addr_len = sizeof (so2);
  memcpy (&results[1].source_addr, &so2, sizeof (so2));


  qsort (results, 2, sizeof (results[0]), rfc3484_sort);

  int result = 0;
  if (results[0].dest_addr->ai_family == AF_INET6)
    {
      puts ("wrong order in first test");
      result |= 1;
    }


  /* And again, this time with the reverse starting order.  */
  results[1].dest_addr = &ai1;
  results[1].got_source_addr = true;
  results[1].source_addr_len = sizeof (so1);
  memcpy (&results[1].source_addr, &so1, sizeof (so1));

  results[0].dest_addr = &ai2;
  results[0].got_source_addr = true;
  results[0].source_addr_len = sizeof (so2);
  memcpy (&results[0].source_addr, &so2, sizeof (so2));


  qsort (results, 2, sizeof (results[0]), rfc3484_sort);

  if (results[0].dest_addr->ai_family == AF_INET6)
    {
      puts ("wrong order in second test");
      result |= 1;
    }

  return result;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
