#include <error.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>

int
main (void)
{
  wctype_t wct;
  wchar_t buf[1000];
  int result = 1;

  setlocale (LC_ALL, "");
  wprintf (L"locale = %s\n", setlocale (LC_CTYPE, NULL));

  wct = wctype ("jhira");
  if (wct == 0)
    error (EXIT_FAILURE, 0, "jhira: no such character class");

  if (fgetws (buf, sizeof (buf) / sizeof (buf[0]), stdin) != NULL)
    {
      int n;

      wprintf (L"buf[] = \"%ls\"\n", buf);

      result = 0;

      for (n = 0; buf[n] != L'\0'; ++n)
	{
	  wprintf (L"jhira(U%04lx = %lc) = %d\n", (long) buf[n], buf[n],
		   iswctype (buf[n], wct));
	  result |= ((buf[n] < 0xff && iswctype (buf[n], wct))
		     || (buf[n] > 0xff && !iswctype (buf[n], wct)));
	}
    }

  return result;
}
