/* Tests for __inet6_scopeid_pton.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

#include <arpa/inet.h>
#include <inttypes.h>
#include <net-internal.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>

/* An interface which is known to the system.  */
static const char *interface_name;
static uint32_t interface_index;

/* Initiale the variables above.  */
static void
setup_interface (void)
{
  struct if_nameindex *list = if_nameindex ();
  if (list != NULL && list[0].if_index != 0 && list[0].if_name[0] != '\0')
    {
      interface_name = list[0].if_name;
      interface_index = list[0].if_index;
    }
}

/* Convert ADDRESS to struct in6_addr.  */
static struct in6_addr
from_string (const char *address)
{
  struct in6_addr addr;
  if (inet_pton (AF_INET6, address, &addr) != 1)
    {
      printf ("error: inet_pton (\"%s\") failed\n", address);
      exit (1);
    }
  return addr;
}

/* Check a single address were we expected a failure.  */
static void
expect_failure (const char *address, const char *scope)
{
  struct in6_addr addr = from_string (address);
  uint32_t result = 1234;
  if (__inet6_scopeid_pton (&addr, scope, &result) == 0)
    {
      printf ("error: unexpected success for %s%%%s\n",
              address, scope);
      exit (1);
    }
  if (result != 1234)
    {
      printf ("error: unexpected result update for %s%%%s\n",
              address, scope);
      exit (1);
    }
}

/* Check a single address were we expected a success.  */
static void
expect_success (const char *address, const char *scope, uint32_t expected)
{
  struct in6_addr addr = from_string (address);
  uint32_t actual = expected + 1;
  if (__inet6_scopeid_pton (&addr, scope, &actual) != 0)
    {
      printf ("error: unexpected failure for %s%%%s\n",
              address, scope);
      exit (1);
    }
  if (actual != expected)
    {
      printf ("error: unexpected result for for %s%%%s\n",
              address, scope);
      printf ("  expected: %" PRIu32 "\n", expected);
      printf ("  actual:   %" PRIu32 "\n", actual);
      exit (1);
    }
}

static int
do_test (void)
{
  setup_interface ();

  static const char *test_addresses[]
    = { "::", "::1", "2001:db8::1", NULL };
  for (int i = 0; test_addresses[i] != NULL; ++i)
    {
      expect_success (test_addresses[i], "0", 0);
      expect_success (test_addresses[i], "5555", 5555);

      expect_failure (test_addresses[i], "");
      expect_failure (test_addresses[i], "-1");
      expect_failure (test_addresses[i], "-99");
      expect_failure (test_addresses[i], "037777777777");
      expect_failure (test_addresses[i], "0x");
      expect_failure (test_addresses[i], "0x1");
    }

  if (interface_name != NULL)
    {
      expect_success ("fe80::1", interface_name, interface_index);
      expect_success ("ff02::1", interface_name, interface_index);
      expect_failure ("::", interface_name);
      expect_failure ("::1", interface_name);
      expect_failure ("ff01::1", interface_name);
      expect_failure ("2001:db8::1", interface_name);
    }

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
