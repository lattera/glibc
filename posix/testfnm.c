#include <stdlib.h>
#include <stdio.h>
#include "fnmatch.h"

struct {
  const char *name;
  const char *pattern;
  int flags;
  int expected;
} tests[] = {
  { "lib", "*LIB*", FNM_PERIOD, FNM_NOMATCH },
  { "lib", "*LIB*", FNM_CASEFOLD|FNM_PERIOD, 0 },
  { "a/b", "a[/]b", 0, 0 },
  { "a/b", "a[/]b", FNM_PATHNAME, FNM_NOMATCH },
  { "a/b", "[a-z]/[a-z]", 0, 0 },
};

int
main (void)
{
  size_t i;
  int errors = 0;

  for (i = 0; i < sizeof (tests) / sizeof (*tests); i++)
    {
      int match;

      match = fnmatch (tests[i].pattern, tests[i].name, tests[i].flags);
      if (match != tests[i].expected)
	{
	  printf ("%s %s %s\n", tests[i].pattern,
		  match == 0 ? "matches" : "does not match",
		  tests[i].name);
	  errors++;
	}
    }

  exit (errors != 0);
}
