#include <nss.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int
do_test (void)
{
  int retval = 0;

  __nss_configure_lookup ("passwd", "test1");

  static const unsigned int pwdids[] = { 100, 30, 200, 60, 20000 };
#define npwdids (sizeof (pwdids) / sizeof (pwdids[0]))
  setpwent ();

  const unsigned int *np = pwdids;
  for (struct passwd *p = getpwent (); p != NULL; ++np, p = getpwent ())
    if (p->pw_uid != *np || strncmp (p->pw_name, "name", 4) != 0
	|| atol (p->pw_name + 4) != *np)
      {
	printf ("passwd entry %td wrong (%s, %u)\n",
		np - pwdids, p->pw_name, p->pw_uid);
	retval = 1;
	break;
      }

  endpwent ();

  for (int i = npwdids - 1; i >= 0; --i)
    {
      char buf[30];
      snprintf (buf, sizeof (buf), "name%u", pwdids[i]);

      struct passwd *p = getpwnam (buf);
      if (p == NULL || p->pw_uid != pwdids[i] || strcmp (buf, p->pw_name) != 0)
	{
	  printf ("passwd entry \"%s\" wrong\n", buf);
	  retval = 1;
	}

      p = getpwuid (pwdids[i]);
      if (p == NULL || p->pw_uid != pwdids[i] || strcmp (buf, p->pw_name) != 0)
	{
	  printf ("passwd entry %u wrong\n", pwdids[i]);
	  retval = 1;
	}

      snprintf (buf, sizeof (buf), "name%u", pwdids[i] + 1);

      p = getpwnam (buf);
      if (p != NULL)
	{
	  printf ("passwd entry \"%s\" wrong\n", buf);
	  retval = 1;
	}

      p = getpwuid (pwdids[i] + 1);
      if (p != NULL)
	{
	  printf ("passwd entry %u wrong\n", pwdids[i] + 1);
	  retval = 1;
	}
    }

  return retval;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
