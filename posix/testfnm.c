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
  { "a/b", "*", FNM_FILE_NAME, FNM_NOMATCH },
  { "a/b", "*[/]b", FNM_FILE_NAME, FNM_NOMATCH },
  { "a/b", "*[b]", FNM_FILE_NAME, FNM_NOMATCH },
  { "a/b", "[*]/b", 0, FNM_NOMATCH },
  { "*/b", "[*]/b", 0, 0 },
  { "a/b", "[?]/b", 0, FNM_NOMATCH },
  { "?/b", "[?]/b", 0, 0 },
  { "a/b", "[[a]/b", 0, 0 },
  { "[/b", "[[a]/b", 0, 0 },
  { "a/b", "\\*/b", 0, FNM_NOMATCH },
  { "*/b", "\\*/b", 0, 0 },
  { "a/b", "\\?/b", 0, FNM_NOMATCH },
  { "?/b", "\\?/b", 0, 0 },
  { "[/b", "[/b", 0, FNM_NOMATCH },
  { "[/b", "\\[/b", 0, 0 },
  { "aa/b", "??/b", 0, 0 },
  { "aa/b", "???b", 0, 0 },
  { "aa/b", "???b", FNM_PATHNAME, FNM_NOMATCH },
  { ".a/b", "?a/b", FNM_PATHNAME|FNM_PERIOD, FNM_NOMATCH },
  { "a/.b", "a/?b", FNM_PATHNAME|FNM_PERIOD, FNM_NOMATCH },
  { ".a/b", "*a/b", FNM_PATHNAME|FNM_PERIOD, FNM_NOMATCH },
  { "a/.b", "a/*b", FNM_PATHNAME|FNM_PERIOD, FNM_NOMATCH },
  { ".a/b", "[.]a/b", FNM_PATHNAME|FNM_PERIOD, FNM_NOMATCH },
  { "a/.b", "a/[.]b", FNM_PATHNAME|FNM_PERIOD, FNM_NOMATCH },
  { "a/b", "*/?", FNM_PATHNAME|FNM_PERIOD, 0 },
  { "a/b", "?/*", FNM_PATHNAME|FNM_PERIOD, 0 },
  { ".a/b", ".*/?", FNM_PATHNAME|FNM_PERIOD, 0 },
  { "a/.b", "*/.?", FNM_PATHNAME|FNM_PERIOD, 0 },
  { "a/.b", "*/*", FNM_PATHNAME|FNM_PERIOD, FNM_NOMATCH },
  { "a/.b", "*?*/*", FNM_PERIOD, 0 },
  { "a./b", "*[.]/b", FNM_PATHNAME|FNM_PERIOD, 0 },
  { "a/b", "*[[:alpha:]]/*[[:alnum:]]", FNM_PATHNAME, 0 },
  { "a/b", "*[![:digit:]]*/[![:d-d]", FNM_PATHNAME, 0 },
  { "a/[", "*[![:digit:]]*/[[:d-d]", FNM_PATHNAME, 0 },
  { "a/[", "*[![:digit:]]*/[![:d-d]", FNM_PATHNAME, FNM_NOMATCH },
  { "a.b", "a?b", FNM_PATHNAME|FNM_PERIOD, 0 },
  { "a.b", "a*b", FNM_PATHNAME|FNM_PERIOD, 0 },
  { "a.b", "a[.]b", FNM_PATHNAME|FNM_PERIOD, 0 },
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
