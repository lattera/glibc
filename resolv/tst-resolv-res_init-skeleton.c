/* Test parsing of /etc/resolv.conf.  Genric version.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

/* Before including this file, TEST_THREAD has to be defined to 0 or
   1, depending on whether the threading tests should be compiled
   in.  */

#include <arpa/inet.h>
#include <gnu/lib-names.h>
#include <netdb.h>
#include <resolv/resolv-internal.h> /* For DEPRECATED_RES_USE_INET6.  */
#include <stdio.h>
#include <stdlib.h>
#include <support/capture_subprocess.h>
#include <support/check.h>
#include <support/namespace.h>
#include <support/run_diff.h>
#include <support/support.h>
#include <support/temp_file.h>
#include <support/test-driver.h>
#include <support/xstdio.h>
#include <support/xunistd.h>

#if TEST_THREAD
# include <support/xthread.h>
#endif

/* This is the host name used to ensure predictable behavior of
   res_init.  */
static const char *const test_hostname = "www.example.com";

/* Path to the test root directory.  */
static char *path_chroot;

/* Path to resolv.conf under path_chroot (outside the chroot).  */
static char *path_resolv_conf;

static void
prepare (int argc, char **argv)
{
  path_chroot = xasprintf ("%s/tst-resolv-res_init-XXXXXX", test_dir);
  if (mkdtemp (path_chroot) == NULL)
    FAIL_EXIT1 ("mkdtemp (\"%s\"): %m", path_chroot);
  add_temp_file (path_chroot);

  /* Create the /etc directory in the chroot environment.  */
  char *path_etc = xasprintf ("%s/etc", path_chroot);
  xmkdir (path_etc, 0777);
  add_temp_file (path_etc);

  /* Create an empty resolv.conf file.  */
  path_resolv_conf = xasprintf ("%s/resolv.conf", path_etc);
  add_temp_file (path_resolv_conf);
  support_write_file_string (path_resolv_conf, "");

  free (path_etc);

  /* valgrind needs a temporary directory in the chroot.  */
  {
    char *path_tmp = xasprintf ("%s/tmp", path_chroot);
    xmkdir (path_tmp, 0777);
    add_temp_file (path_tmp);
    free (path_tmp);
  }
}

/* Verify that the chroot environment has been set up.  */
static void
check_chroot_working (void *closure)
{
  xchroot (path_chroot);
  FILE *fp = xfopen (_PATH_RESCONF, "r");
  xfclose (fp);

  TEST_VERIFY_EXIT (res_init () == 0);
  TEST_VERIFY (_res.options & RES_INIT);

  char buf[100];
  if (gethostname (buf, sizeof (buf)) < 0)
    FAIL_EXIT1 ("gethostname: %m");
  if (strcmp (buf, test_hostname) != 0)
    FAIL_EXIT1 ("unexpected host name: %s", buf);
}

/* If FLAG is set in *OPTIONS, write NAME to FP, and clear it in
   *OPTIONS.  */
static void
print_option_flag (FILE *fp, int *options, int flag, const char *name)
{
  if (*options & flag)
    {
      fprintf (fp, " %s", name);
      *options &= ~flag;
    }
}

/* Write a decoded version of the resolver configuration *RESP to the
   stream FP.  */
static void
print_resp (FILE *fp, res_state resp)
{
  /* The options directive.  */
  {
    /* RES_INIT is used internally for tracking initialization.  */
    TEST_VERIFY (resp->options & RES_INIT);
    /* Also mask out other default flags which cannot be set through
       the options directive.  */
    int options
      = resp->options & ~(RES_INIT | RES_RECURSE | RES_DEFNAMES | RES_DNSRCH);
    if (options != 0
        || resp->ndots != 1
        || resp->retrans != RES_TIMEOUT
        || resp->retry != RES_DFLRETRY)
      {
        fputs ("options", fp);
        if (resp->ndots != 1)
          fprintf (fp, " ndots:%d", resp->ndots);
        if (resp->retrans != RES_TIMEOUT)
          fprintf (fp, " timeout:%d", resp->retrans);
        if (resp->retry != RES_DFLRETRY)
          fprintf (fp, " attempts:%d", resp->retry);
        print_option_flag (fp, &options, RES_USEVC, "use-vc");
        print_option_flag (fp, &options, DEPRECATED_RES_USE_INET6, "inet6");
        print_option_flag (fp, &options, RES_ROTATE, "rotate");
        print_option_flag (fp, &options, RES_USE_EDNS0, "edns0");
        print_option_flag (fp, &options, RES_SNGLKUP,
                           "single-request");
        print_option_flag (fp, &options, RES_SNGLKUPREOP,
                           "single-request-reopen");
        print_option_flag (fp, &options, RES_NOTLDQUERY, "no-tld-query");
        fputc ('\n', fp);
        if (options != 0)
          fprintf (fp, "; error: unresolved option bits: 0x%x\n", options);
      }
  }

  /* The search and domain directives.  */
  if (resp->dnsrch[0] != NULL)
    {
      fputs ("search", fp);
      for (int i = 0; i < MAXDNSRCH && resp->dnsrch[i] != NULL; ++i)
        {
          fputc (' ', fp);
          fputs (resp->dnsrch[i], fp);
        }
      fputc ('\n', fp);
    }
  else if (resp->defdname[0] != '\0')
    fprintf (fp, "domain %s\n", resp->defdname);

  /* The sortlist directive.  */
  if (resp->nsort > 0)
    {
      fputs ("sortlist", fp);
      for (int i = 0; i < resp->nsort && i < MAXRESOLVSORT; ++i)
        {
          char net[20];
          if (inet_ntop (AF_INET, &resp->sort_list[i].addr,
                         net, sizeof (net)) == NULL)
            FAIL_EXIT1 ("inet_ntop: %m\n");
          char mask[20];
          if (inet_ntop (AF_INET, &resp->sort_list[i].mask,
                         mask, sizeof (mask)) == NULL)
            FAIL_EXIT1 ("inet_ntop: %m\n");
          fprintf (fp, " %s/%s", net, mask);
        }
      fputc ('\n', fp);
    }

  /* The nameserver directives.  */
  for (size_t i = 0; i < resp->nscount; ++i)
    {
      char host[NI_MAXHOST];
      char service[NI_MAXSERV];

      /* See get_nsaddr in res_send.c.  */
      void *addr;
      size_t addrlen;
      if (resp->nsaddr_list[i].sin_family == 0
          && resp->_u._ext.nsaddrs[i] != NULL)
        {
          addr = resp->_u._ext.nsaddrs[i];
          addrlen = sizeof (*resp->_u._ext.nsaddrs[i]);
        }
      else
        {
          addr = &resp->nsaddr_list[i];
          addrlen = sizeof (resp->nsaddr_list[i]);
        }

      int ret = getnameinfo (addr, addrlen,
                             host, sizeof (host), service, sizeof (service),
                             NI_NUMERICHOST | NI_NUMERICSERV);
      if (ret != 0)
        {
          if (ret == EAI_SYSTEM)
            fprintf (fp, "; error: getnameinfo: %m\n");
          else
            fprintf (fp, "; error: getnameinfo: %s\n", gai_strerror (ret));
        }
      else
        {
          fprintf (fp, "nameserver %s\n", host);
          if (strcmp (service, "53") != 0)
            fprintf (fp, "; unrepresentable port number %s\n\n", service);
        }
    }

  TEST_VERIFY (!ferror (fp));
}

/* Parameters of one test case.  */
struct test_case
{
  /* A short, descriptive name of the test.  */
  const char *name;

  /* The contents of the /etc/resolv.conf file.  */
  const char *conf;

  /* The expected output from print_resp.  */
  const char *expected;

  /* Setting for the LOCALDOMAIN environment variable.  NULL if the
     variable is not to be set.  */
  const char *localdomain;

  /* Setting for the RES_OPTIONS environment variable.  NULL if the
     variable is not to be set.  */
  const char *res_options;
};

enum test_init
{
  test_init,
  test_ninit,
  test_mkquery,
  test_gethostbyname,
  test_getaddrinfo,
  test_init_method_last = test_getaddrinfo
};

/* Closure argument for run_res_init.  */
struct test_context
{
  enum test_init init;
  const struct test_case *t;
};

static void
setup_nss_dns_and_chroot (void)
{
  /* Load nss_dns outside of the chroot.  */
  if (dlopen (LIBNSS_DNS_SO, RTLD_LAZY) == NULL)
    FAIL_EXIT1 ("could not load " LIBNSS_DNS_SO ": %s", dlerror ());
  xchroot (path_chroot);
  /* Force the use of nss_dns.  */
  __nss_configure_lookup ("hosts", "dns");
}

/* Run res_ninit or res_init in a subprocess and dump the parsed
   resolver state to standard output.  */
static void
run_res_init (void *closure)
{
  struct test_context *ctx = closure;
  TEST_VERIFY (getenv ("LOCALDOMAIN") == NULL);
  TEST_VERIFY (getenv ("RES_OPTIONS") == NULL);
  if (ctx->t->localdomain != NULL)
    setenv ("LOCALDOMAIN", ctx->t->localdomain, 1);
  if (ctx->t->res_options != NULL)
    setenv ("RES_OPTIONS", ctx->t->res_options, 1);

  switch (ctx->init)
    {
    case test_init:
      xchroot (path_chroot);
      TEST_VERIFY (res_init () == 0);
      print_resp (stdout, &_res);
      return;

    case test_ninit:
      xchroot (path_chroot);
      res_state resp = xmalloc (sizeof (*resp));
      memset (resp, 0, sizeof (*resp));
      TEST_VERIFY (res_ninit (resp) == 0);
      print_resp (stdout, resp);
      res_nclose (resp);
      free (resp);
      return;

    case test_mkquery:
      xchroot (path_chroot);
      unsigned char buf[512];
      TEST_VERIFY (res_mkquery (QUERY, "www.example",
                                C_IN, ns_t_a, NULL, 0,
                                NULL, buf, sizeof (buf)) > 0);
      print_resp (stdout, &_res);
      return;

    case test_gethostbyname:
      setup_nss_dns_and_chroot ();
      /* Trigger implicit initialization of the _res structure.  The
         actual lookup result is immaterial.  */
      (void )gethostbyname ("www.example");
      print_resp (stdout, &_res);
      return;

    case test_getaddrinfo:
      setup_nss_dns_and_chroot ();
      /* Trigger implicit initialization of the _res structure.  The
         actual lookup result is immaterial.  */
      struct addrinfo *ai;
      (void) getaddrinfo ("www.example", NULL, NULL, &ai);
      print_resp (stdout, &_res);
      return;
    }

  FAIL_EXIT1 ("invalid init method %d", ctx->init);
}

#if TEST_THREAD
/* Helper function which calls run_res_init from a thread.  */
static void *
run_res_init_thread_func (void *closure)
{
  run_res_init (closure);
  return NULL;
}

/* Variant of res_run_init which runs the function on a non-main
   thread.  */
static void
run_res_init_on_thread (void *closure)
{
  xpthread_join (xpthread_create (NULL, run_res_init_thread_func, closure));
}
#endif /* TEST_THREAD */

struct test_case test_cases[] =
  {
    {.name = "empty file",
     .conf = "",
     .expected = "search example.com\n"
     "nameserver 127.0.0.1\n"
    },
    {.name = "empty file with LOCALDOMAIN",
     .conf = "",
     .expected = "search example.net\n"
     "nameserver 127.0.0.1\n",
     .localdomain = "example.net",
    },
    {.name = "empty file with RES_OPTIONS",
     .conf = "",
     .expected = "options attempts:5 edns0\n"
     "search example.com\n"
     "nameserver 127.0.0.1\n",
     .res_options = "edns0 attempts:5",
    },
    {.name = "empty file with RES_OPTIONS and LOCALDOMAIN",
     .conf = "",
     .expected = "options attempts:5 edns0\n"
     "search example.org\n"
     "nameserver 127.0.0.1\n",
     .localdomain = "example.org",
     .res_options = "edns0 attempts:5",
    },
    {.name = "basic",
     .conf = "domain example.net\n"
     "search corp.example.com example.com\n"
     "nameserver 192.0.2.1\n",
     .expected = "search corp.example.com example.com\n"
     "nameserver 192.0.2.1\n"
    },
    {.name = "whitespace",
     .conf = "# This test covers comment and whitespace processing "
     " (trailing whitespace,\n"
     "# missing newline at end of file).\n"
     "\n"
     "domain  example.net\n"
     ";search commented out\n"
     "search corp.example.com\texample.com\n"
     "#nameserver 192.0.2.3\n"
     "nameserver 192.0.2.1 \n"
     "nameserver 192.0.2.2",    /* No \n at end of file.  */
     .expected = "search corp.example.com example.com\n"
     "nameserver 192.0.2.1\n"
     "nameserver 192.0.2.2\n"
    },
    {.name = "option values, multiple servers",
     .conf = "options\tinet6\tndots:3 edns0\tattempts:5\ttimeout:19\n"
     "domain  example.net\n"
     ";domain comment\n"
     "search corp.example.com\texample.com\n"
     "nameserver 192.0.2.1\n"
     "nameserver ::1\n"
     "nameserver 192.0.2.2\n",
     .expected = "options ndots:3 timeout:19 attempts:5 inet6 edns0\n"
     "search corp.example.com example.com\n"
     "nameserver 192.0.2.1\n"
     "nameserver ::1\n"
     "nameserver 192.0.2.2\n"
    },
    {.name = "out-of-range option vales",
     .conf = "options use-vc timeout:999 attempts:999 ndots:99\n"
     "search example.com\n",
     .expected = "options ndots:15 timeout:30 attempts:5 use-vc\n"
     "search example.com\n"
     "nameserver 127.0.0.1\n"
    },
    {.name = "repeated directives",
     .conf = "options ndots:3 use-vc\n"
     "options edns0 ndots:2\n"
     "domain corp.example\n"
     "search example.net corp.example.com example.com\n"
     "search example.org\n"
     "search\n",
     .expected = "options ndots:2 use-vc edns0\n"
     "search example.org\n"
     "nameserver 127.0.0.1\n"
    },
    {.name = "many name servers, sortlist",
     .conf = "options single-request\n"
     "search example.org example.com example.net corp.example.com\n"
     "sortlist 192.0.2.0/255.255.255.0\n"
     "nameserver 192.0.2.1\n"
     "nameserver 192.0.2.2\n"
     "nameserver 192.0.2.3\n"
     "nameserver 192.0.2.4\n"
     "nameserver 192.0.2.5\n"
     "nameserver 192.0.2.6\n"
     "nameserver 192.0.2.7\n"
     "nameserver 192.0.2.8\n",
     .expected = "options single-request\n"
     "search example.org example.com example.net corp.example.com\n"
     "sortlist 192.0.2.0/255.255.255.0\n"
     "nameserver 192.0.2.1\n"
     "nameserver 192.0.2.2\n"
     "nameserver 192.0.2.3\n"
    },
    {.name = "IPv4 and IPv6 nameservers",
     .conf = "options single-request\n"
     "search example.org example.com example.net corp.example.com"
     " legacy.example.com\n"
     "sortlist 192.0.2.0\n"
     "nameserver 192.0.2.1\n"
     "nameserver 2001:db8::2\n"
     "nameserver 192.0.2.3\n"
     "nameserver 2001:db8::4\n"
     "nameserver 192.0.2.5\n"
     "nameserver 2001:db8::6\n"
     "nameserver 192.0.2.7\n"
     "nameserver 2001:db8::8\n",
     .expected = "options single-request\n"
     "search example.org example.com example.net corp.example.com"
     " legacy.example.com\n"
     "sortlist 192.0.2.0/255.255.255.0\n"
     "nameserver 192.0.2.1\n"
     "nameserver 2001:db8::2\n"
     "nameserver 192.0.2.3\n"
    },
    {.name = "garbage after nameserver",
     .conf = "nameserver 192.0.2.1 garbage\n"
     "nameserver 192.0.2.2:5353\n"
     "nameserver 192.0.2.3 5353\n",
     .expected = "search example.com\n"
     "nameserver 192.0.2.1\n"
     "nameserver 192.0.2.3\n"
    },
    {.name = "RES_OPTIONS is cummulative",
     .conf = "options timeout:7 ndots:2 use-vc\n"
     "nameserver 192.0.2.1\n",
     .expected = "options ndots:3 timeout:7 attempts:5 use-vc edns0\n"
     "search example.com\n"
     "nameserver 192.0.2.1\n",
     .res_options = "attempts:5 ndots:3 edns0 ",
    },
    { NULL }
  };

/* Run the indicated test case.  This function assumes that the chroot
   contents has already been set up.  */
static void
test_file_contents (const struct test_case *t)
{
#if TEST_THREAD
  for (int do_thread = 0; do_thread < 2; ++do_thread)
#endif
    for (int init_method = 0; init_method <= test_init_method_last;
         ++init_method)
      {
        if (test_verbose > 0)
          printf ("info:  testing init method %d\n", init_method);
        struct test_context ctx = { .init = init_method, .t = t };
        void (*func) (void *) = run_res_init;
#if TEST_THREAD
        if (do_thread)
          func = run_res_init_on_thread;
#endif
        struct support_capture_subprocess proc
          = support_capture_subprocess (func, &ctx);
        if (strcmp (proc.out.buffer, t->expected) != 0)
          {
            support_record_failure ();
            printf ("error: output mismatch for %s\n", t->name);
            support_run_diff ("expected", t->expected,
                              "actual", proc.out.buffer);
          }
        support_capture_subprocess_check (&proc, t->name, 0,
                                          sc_allow_stdout);
        support_capture_subprocess_free (&proc);
      }
}

static int
do_test (void)
{
  support_become_root ();
  support_enter_network_namespace ();
  if (!support_in_uts_namespace () || !support_can_chroot ())
    return EXIT_UNSUPPORTED;

  /* We are in an UTS namespace, so we can set the host name without
     altering the state of the entire system.  */
  if (sethostname (test_hostname, strlen (test_hostname)) != 0)
    FAIL_EXIT1 ("sethostname: %m");

  /* These environment variables affect resolv.conf parsing.  */
  unsetenv ("LOCALDOMAIN");
  unsetenv ("RES_OPTIONS");

  /* Ensure that the chroot setup worked.  */
  {
    struct support_capture_subprocess proc
      = support_capture_subprocess (check_chroot_working, NULL);
    support_capture_subprocess_check (&proc, "chroot", 0, sc_allow_none);
    support_capture_subprocess_free (&proc);
  }

  for (size_t i = 0; test_cases[i].name != NULL; ++i)
    {
      if (test_verbose > 0)
        printf ("info: running test: %s\n", test_cases[i].name);
      TEST_VERIFY (test_cases[i].conf != NULL);
      TEST_VERIFY (test_cases[i].expected != NULL);

      support_write_file_string (path_resolv_conf, test_cases[i].conf);

      test_file_contents (&test_cases[i]);

      /* The expected output from the empty file test is used for
         further tests.  */
      if (test_cases[i].conf[0] == '\0')
        {
          if (test_verbose > 0)
            printf ("info:  special test: missing file\n");
          TEST_VERIFY (unlink (path_resolv_conf) == 0);
          test_file_contents (&test_cases[i]);

          if (test_verbose > 0)
            printf ("info:  special test: dangling symbolic link\n");
          TEST_VERIFY (symlink ("does-not-exist", path_resolv_conf) == 0);
          test_file_contents (&test_cases[i]);
          TEST_VERIFY (unlink (path_resolv_conf) == 0);

          if (test_verbose > 0)
            printf ("info:  special test: unreadable file\n");
          support_write_file_string (path_resolv_conf, "");
          TEST_VERIFY (chmod (path_resolv_conf, 0) == 0);
          test_file_contents (&test_cases[i]);

          /* Restore the empty file.  */
          TEST_VERIFY (unlink (path_resolv_conf) == 0);
          support_write_file_string (path_resolv_conf, "");
        }
    }

  free (path_chroot);
  path_chroot = NULL;
  free (path_resolv_conf);
  path_resolv_conf = NULL;
  return 0;
}

#define PREPARE prepare
#include <support/test-driver.c>
