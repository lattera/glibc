/* Basic functionality tests for inet6 option processing.
   Copyright (C) 2016-2017 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <netdb.h>
#include <resolv.h>
#include <string.h>
#include <support/check_nss.h>
#include <support/resolv_test.h>
#include <support/xthread.h>

static void
response (const struct resolv_response_context *ctx,
          struct resolv_response_builder *b,
          const char *qname, uint16_t qclass, uint16_t qtype)
{
  bool include_both =  strcmp (qname, "both.example") == 0;
  bool include_a = qtype == T_A || include_both;
  bool include_aaaa = qtype == T_AAAA || include_both;

  resolv_response_init (b, (struct resolv_response_flags) {});
  resolv_response_add_question (b, qname, qclass, qtype);
  resolv_response_section (b, ns_s_an);
  if (include_a)
    {
      char ipv4[4] = {192, 0, 2, 17};
      resolv_response_open_record (b, qname, qclass, T_A, 0);
      resolv_response_add_data (b, &ipv4, sizeof (ipv4));
      resolv_response_close_record (b);
    }
  if (include_aaaa)
    {
        char ipv6[16]
          = {0x20, 0x01, 0xd, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
        resolv_response_open_record (b, qname, qclass, T_AAAA, 0);
        resolv_response_add_data (b, &ipv6, sizeof (ipv6));
        resolv_response_close_record (b);
    }
}

/* Test that getaddrinfo is not influenced by RES_USE_INET6.  */
static void
test_gai (void)
{
  {
    struct addrinfo hints =
      {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP,
      };
    struct addrinfo *ai;
    int ret = getaddrinfo ("www1.example", "80", &hints, &ai);
    check_addrinfo ("getaddrinfo AF_UNSPEC www1.example", ai, ret,
                    "address: STREAM/TCP 192.0.2.17 80\n"
                    "address: STREAM/TCP 2001:db8::1 80\n");
    if (ret == 0)
      freeaddrinfo (ai);
    ret = getaddrinfo ("both.example", "80", &hints, &ai);
    /* Combined A/AAAA responses currently result in address
       duplication.  */
    check_addrinfo ("getaddrinfo AF_UNSPEC both.example", ai, ret,
                    "address: STREAM/TCP 192.0.2.17 80\n"
                    "address: STREAM/TCP 192.0.2.17 80\n"
                    "address: STREAM/TCP 2001:db8::1 80\n"
                    "address: STREAM/TCP 2001:db8::1 80\n");
    if (ret == 0)
      freeaddrinfo (ai);
  }
  {
    struct addrinfo hints =
      {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP,
      };
    struct addrinfo *ai;
    int ret = getaddrinfo ("www1.example", "80", &hints, &ai);
    check_addrinfo ("getaddrinfo AF_INET www1.example", ai, ret,
                    "address: STREAM/TCP 192.0.2.17 80\n");
    if (ret == 0)
      freeaddrinfo (ai);
    ret = getaddrinfo ("both.example", "80", &hints, &ai);
    check_addrinfo ("getaddrinfo AF_INET both.example", ai, ret,
                    "address: STREAM/TCP 192.0.2.17 80\n");
    if (ret == 0)
      freeaddrinfo (ai);
  }
  {
    struct addrinfo hints =
      {
        .ai_family = AF_INET6,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = IPPROTO_TCP,
      };
    struct addrinfo *ai;
    int ret = getaddrinfo ("www1.example", "80", &hints, &ai);
    check_addrinfo ("getaddrinfo (AF_INET6)", ai, ret,
                    "address: STREAM/TCP 2001:db8::1 80\n");
    if (ret == 0)
      freeaddrinfo (ai);
    ret = getaddrinfo ("both.example", "80", &hints, &ai);
    check_addrinfo ("getaddrinfo AF_INET6 both.example", ai, ret,
                    "address: STREAM/TCP 2001:db8::1 80\n");
    if (ret == 0)
      freeaddrinfo (ai);
  }
}

/* Test that gethostbyname2 is not influenced by RES_USE_INET6.  */
static void
test_get2 (void)
{
  check_hostent ("gethostbyname2 AF_INET www1.example",
                 gethostbyname2 ("www1.example", AF_INET),
                 "name: www1.example\n"
                 "address: 192.0.2.17\n");
  check_hostent ("gethostbyname2 AF_INET both.example",
                 gethostbyname2 ("both.example", AF_INET),
                 "name: both.example\n"
                 "address: 192.0.2.17\n");

  check_hostent ("gethostbyname2 AF_INET6 www1.example",
                 gethostbyname2 ("www1.example", AF_INET6),
                 "name: www1.example\n"
                 "address: 2001:db8::1\n");
  check_hostent ("gethostbyname2 AF_INET6 both.example",
                 gethostbyname2 ("both.example", AF_INET6),
                 "name: both.example\n"
                 "address: 2001:db8::1\n");
}

static void *
threadfunc (void *ignored)
{
  struct resolv_test *obj = resolv_test_start
    ((struct resolv_redirect_config)
     {
       .response_callback = response
     });

  check_hostent ("gethostbyname (\"www1.example\")",
                 gethostbyname ("www1.example"),
                 "name: www1.example\n"
                 "address: 192.0.2.17\n");
  check_hostent ("gethostbyname (\"both.example\")",
                 gethostbyname ("both.example"),
                 "name: both.example\n"
                 "address: 192.0.2.17\n");
  test_get2 ();
  test_gai ();

  _res.options |= RES_USE_INET6;
  check_hostent ("gethostbyname (\"www1.example\")",
                 gethostbyname ("www1.example"),
                 "name: www1.example\n"
                 "address: 2001:db8::1\n");
  check_hostent ("gethostbyname (\"both.example\")",
                 gethostbyname ("both.example"),
                 "name: both.example\n"
                 "address: 2001:db8::1\n");
  test_get2 ();
  test_gai ();

  resolv_test_end (obj);

  return NULL;
}

static int
do_test (void)
{
  resolv_test_init ();

  /* Attempt to run on a non-main thread first.  */
  {
    pthread_t thr = xpthread_create (NULL, threadfunc, NULL);
    xpthread_join (thr);
  }

  /* Try the main thread next.  */
  threadfunc (NULL);

  return 0;
}

#include <support/test-driver.c>
