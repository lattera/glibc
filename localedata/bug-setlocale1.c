// BZ 12788
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


static int
do_test (int argc, char *argv[])
{
  if (argc > 1)
    {
      char *newargv[5];
      int i;
      if (argc != 2 && argc != 5)
	{
	  printf ("wrong number of arguments (%d)\n", argc);
	  return 1;
	}

      for (i = 0; i < (argc == 5 ? 4 : 1); i++)
	newargv[i] = argv[i + 1];
      newargv[i] = NULL;

      char *env[3];
      env[0] = (char *) "LC_CTYPE=de_DE.UTF-8";
      char *loc = getenv ("LOCPATH");
      if (loc == NULL || loc[0] == '\0')
	{
	  puts ("LOCPATH not set");
	  return 1;
	}
      asprintf (&env[1], "LOCPATH=%s", loc);
      if (env[1] == NULL)
	{
	  puts ("asprintf failed");
	  return 1;
	}
      env[2] = NULL;

      execve (newargv[0], newargv, env);

      puts ("execve returned");
      return 1;
    }

  int result = 0;

  char *a = setlocale (LC_ALL, "");
  printf ("setlocale(LC_ALL, \"\") = %s\n", a);
  if (a == NULL)
    return 1;
  a = strdupa (a);

  char *b = setlocale (LC_CTYPE, "");
  printf ("setlocale(LC_CTYPE, \"\") = %s\n", b);
  if (b == NULL)
    return 1;

  char *c = setlocale (LC_ALL, NULL);
  printf ("setlocale(LC_ALL, NULL) = %s\n", c);
  if (c == NULL)
    return 1;
  c = strdupa (c);

  if (strcmp (a, c) != 0)
    {
      puts ("*** first and third result do not match");
      result = 1;
    }

  char *d = setlocale (LC_NUMERIC, "");
  printf ("setlocale(LC_NUMERIC, \"\") = %s\n", d);
  if (d == NULL)
    return 1;

  if (strcmp (d, "C") != 0)
    {
      puts ("*** LC_NUMERIC not C");
      result = 1;
    }

  char *e = setlocale (LC_ALL, NULL);
  printf ("setlocale(LC_ALL, NULL) = %s\n", e);
  if (e == NULL)
    return 1;

  if (strcmp (a, e) != 0)
    {
      puts ("*** first and fifth result do not match");
      result = 1;
    }

  char *f = setlocale (LC_ALL, "C");
  printf ("setlocale(LC_ALL, \"C\") = %s\n", f);
  if (f == NULL)
    return 1;

  if (strcmp (f, "C") != 0)
    {
      puts ("*** LC_ALL not C");
      result = 1;
    }

  char *g = setlocale (LC_ALL, NULL);
  printf ("setlocale(LC_ALL, NULL) = %s\n", g);
  if (g == NULL)
    return 1;

  if (strcmp (g, "C") != 0)
    {
      puts ("*** LC_ALL not C");
      result = 1;
    }

  char *h = setlocale (LC_CTYPE, NULL);
  printf ("setlocale(LC_CTYPE, NULL) = %s\n", h);
  if (h == NULL)
    return 1;

  if (strcmp (h, "C") != 0)
    {
      puts ("*** LC_CTYPE not C");
      result = 1;
    }

  return result;
}

#define TEST_FUNCTION do_test (argc, argv)
#include "../test-skeleton.c"
