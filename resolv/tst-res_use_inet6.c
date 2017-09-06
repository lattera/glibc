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
#include <support/check.h>
#include <support/check_nss.h>
#include <support/resolv_test.h>
#include <support/xthread.h>

/* Produce a response based on QNAME: Certain characters in the first
   label of QNAME trigger the inclusion of resource records:

   'a'   A record (IPv4 address)
   'q'   AAAA record (quad A record, IPv6 address)
   'm'   record type must match QTYPE (no additional records)

   QTYPE is ignored for record type selection if 'm' is not
   specified.  */
static void
response (const struct resolv_response_context *ctx,
          struct resolv_response_builder *b,
          const char *qname, uint16_t qclass, uint16_t qtype)
{
  bool include_a = false;
  bool include_aaaa = false;
  bool include_match = false;
  for (const char *p = qname; *p != '.' && *p != '\0'; ++p)
    {
      if (*p == 'a')
        include_a = true;
      else if (*p == 'q')
        include_aaaa = true;
      else if (*p == 'm')
        include_match = true;
    }
  if (include_match)
    {
      if (qtype == T_A)
        include_aaaa = false;
      else if (qtype == T_AAAA)
        include_a = false;
    }

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
    int ret = getaddrinfo ("qam.example", "80", &hints, &ai);
    check_addrinfo ("getaddrinfo AF_UNSPEC qam.example", ai, ret,
                    "address: STREAM/TCP 192.0.2.17 80\n"
                    "address: STREAM/TCP 2001:db8::1 80\n");
    if (ret == 0)
      freeaddrinfo (ai);
    ret = getaddrinfo ("am.example", "80", &hints, &ai);
    check_addrinfo ("getaddrinfo AF_UNSPEC am.example", ai, ret,
                    "address: STREAM/TCP 192.0.2.17 80\n");
    if (ret == 0)
      freeaddrinfo (ai);
    ret = getaddrinfo ("qa.example", "80", &hints, &ai);
    /* Combined A/AAAA responses currently result in address
       duplication.  */
    check_addrinfo ("getaddrinfo AF_UNSPEC qa.example", ai, ret,
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
    int ret = getaddrinfo ("qam.example", "80", &hints, &ai);
    check_addrinfo ("getaddrinfo AF_INET qam.example", ai, ret,
                    "address: STREAM/TCP 192.0.2.17 80\n");
    if (ret == 0)
      freeaddrinfo (ai);
    ret = getaddrinfo ("am.example", "80", &hints, &ai);
    check_addrinfo ("getaddrinfo AF_INET am.example", ai, ret,
                    "address: STREAM/TCP 192.0.2.17 80\n");
    if (ret == 0)
      freeaddrinfo (ai);
    ret = getaddrinfo ("qa.example", "80", &hints, &ai);
    check_addrinfo ("getaddrinfo AF_INET qa.example", ai, ret,
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
    int ret = getaddrinfo ("qa.example", "80", &hints, &ai);
    check_addrinfo ("getaddrinfo (AF_INET6)", ai, ret,
                    "address: STREAM/TCP 2001:db8::1 80\n");
    if (ret == 0)
      freeaddrinfo (ai);
    ret = getaddrinfo ("am.example", "80", &hints, &ai);
    check_addrinfo ("getaddrinfo AF_INET6 am.example", ai, ret,
                    "error: No address associated with hostname\n");
    if (ret == 0)
      freeaddrinfo (ai);
    ret = getaddrinfo ("qam.example", "80", &hints, &ai);
    check_addrinfo ("getaddrinfo AF_INET6 qam.example", ai, ret,
                    "address: STREAM/TCP 2001:db8::1 80\n");
    if (ret == 0)
      freeaddrinfo (ai);
  }
}

/* Test that gethostbyname2 is mostly not influenced by
   RES_USE_INET6.  */
static void
test_get2_any (void)
{
  check_hostent ("gethostbyname2 AF_INET am.example",
                 gethostbyname2 ("am.example", AF_INET),
                 "name: am.example\n"
                 "address: 192.0.2.17\n");
  check_hostent ("gethostbyname2 AF_INET a.example",
                 gethostbyname2 ("a.example", AF_INET),
                 "name: a.example\n"
                 "address: 192.0.2.17\n");
  check_hostent ("gethostbyname2 AF_INET qm.example",
                 gethostbyname2 ("qm.example", AF_INET),
                 "error: NO_ADDRESS\n");
  check_hostent ("gethostbyname2 AF_INET q.example",
                 gethostbyname2 ("q.example", AF_INET),
                 "error: NO_RECOVERY\n");
  check_hostent ("gethostbyname2 AF_INET qam.example",
                 gethostbyname2 ("qam.example", AF_INET),
                 "name: qam.example\n"
                 "address: 192.0.2.17\n");
  check_hostent ("gethostbyname2 AF_INET qa.example",
                 gethostbyname2 ("qa.example", AF_INET),
                 "name: qa.example\n"
                 "address: 192.0.2.17\n");

  check_hostent ("gethostbyname2 AF_INET6 qm.example",
                 gethostbyname2 ("qm.example", AF_INET6),
                 "name: qm.example\n"
                 "address: 2001:db8::1\n");
  check_hostent ("gethostbyname2 AF_INET6 q.example",
                 gethostbyname2 ("q.example", AF_INET6),
                 "name: q.example\n"
                 "address: 2001:db8::1\n");
  check_hostent ("gethostbyname2 AF_INET6 qam.example",
                 gethostbyname2 ("qam.example", AF_INET6),
                 "name: qam.example\n"
                 "address: 2001:db8::1\n");
  check_hostent ("gethostbyname2 AF_INET6 qa.example",
                 gethostbyname2 ("qa.example", AF_INET6),
                 "name: qa.example\n"
                 "address: 2001:db8::1\n");
  /* Additional AF_INET6 tests depend on RES_USE_INET6; see below.  */
}

/* gethostbyname2 tests with RES_USE_INET6 disabled.  */
static void
test_get2_no_inet6 (void)
{
  test_get2_any ();

  check_hostent ("gethostbyname2 AF_INET6 am.example",
                 gethostbyname2 ("am.example", AF_INET6),
                 "error: NO_ADDRESS\n");
  check_hostent ("gethostbyname2 AF_INET6 a.example",
                 gethostbyname2 ("a.example", AF_INET6),
                 "error: NO_RECOVERY\n");
}

/* gethostbyname2 tests with RES_USE_INET6 enabled.  */
static void
test_get2_inet6 (void)
{
  test_get2_any ();

  check_hostent ("gethostbyname2 AF_INET6 am.example",
                 gethostbyname2 ("am.example", AF_INET6),
                 "name: am.example\n"
                 "address: ::ffff:192.0.2.17\n");
  check_hostent ("gethostbyname2 AF_INET6 a.example",
                 gethostbyname2 ("a.example", AF_INET6),
                 "error: NO_RECOVERY\n");
}

/* Collection of tests which assume no RES_USE_INET6 flag.  */
static void
test_no_inet6 (void)
{
  check_hostent ("gethostbyname (\"a.example\")",
                 gethostbyname ("a.example"),
                 "name: a.example\n"
                 "address: 192.0.2.17\n");
  check_hostent ("gethostbyname (\"qa.example\")",
                 gethostbyname ("qa.example"),
                 "name: qa.example\n"
                 "address: 192.0.2.17\n");
  check_hostent ("gethostbyname (\"am.example\")",
                 gethostbyname ("am.example"),
                 "name: am.example\n"
                 "address: 192.0.2.17\n");
  check_hostent ("gethostbyname (\"qam.example\")",
                 gethostbyname ("qam.example"),
                 "name: qam.example\n"
                 "address: 192.0.2.17\n");
  check_hostent ("gethostbyname (\"q.example\")",
                 gethostbyname ("q.example"),
                 "error: NO_RECOVERY\n");
  check_hostent ("gethostbyname (\"qm.example\")",
                 gethostbyname ("qm.example"),
                 "error: NO_ADDRESS\n");
  test_get2_no_inet6 ();
  test_get2_no_inet6 ();
  test_gai ();
  test_get2_no_inet6 ();
  test_get2_no_inet6 ();
}

static void *
threadfunc (void *ignored)
{
  struct resolv_test *obj = resolv_test_start
    ((struct resolv_redirect_config)
     {
       .response_callback = response
     });

  TEST_VERIFY ((_res.options & RES_USE_INET6) == 0);
  test_no_inet6 ();

  _res.options |= RES_USE_INET6;
  check_hostent ("gethostbyname (\"a.inet6.example\")",
                 gethostbyname ("a.inet6.example"),
                 "error: NO_RECOVERY\n");
  check_hostent ("gethostbyname (\"am.inet6.example\")",
                 gethostbyname ("am.inet6.example"),
                 "name: am.inet6.example\n"
                 "address: ::ffff:192.0.2.17\n");
  check_hostent ("gethostbyname (\"qa.inet6.example\")",
                 gethostbyname ("qa.inet6.example"),
                 "name: qa.inet6.example\n"
                 "address: 2001:db8::1\n");
  check_hostent ("gethostbyname (\"qam.inet6.example\")",
                 gethostbyname ("qam.inet6.example"),
                 "name: qam.inet6.example\n"
                 "address: 2001:db8::1\n");
  check_hostent ("gethostbyname (\"q.inet6.example\")",
                 gethostbyname ("q.inet6.example"),
                 "name: q.inet6.example\n"
                 "address: 2001:db8::1\n");
  check_hostent ("gethostbyname (\"qm.inet6.example\")",
                 gethostbyname ("qm.inet6.example"),
                 "name: qm.inet6.example\n"
                 "address: 2001:db8::1\n");
  test_get2_inet6 ();
  test_get2_inet6 ();
  test_gai ();
  test_get2_inet6 ();
  test_get2_inet6 ();

  TEST_VERIFY (_res.options & RES_USE_INET6);
  _res.options &= ~RES_USE_INET6;
  test_no_inet6 ();

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
