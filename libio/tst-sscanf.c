#include <stdio.h>
#include <wchar.h>

#define WCS_LENGTH 256

int
main (void)
{
  const char cnv[] ="%l[abc]";
  const char str[] = "abbcXab";
  wchar_t wcs[WCS_LENGTH];
  int result = 0;

  sscanf (str, cnv, wcs);
  printf ("wcs = \"%ls\"\n", wcs);
  fflush (stdout);
  result = wcscmp (wcs, L"abbc") != 0;

  return result;
}
