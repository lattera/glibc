#include <stdio.h>
#include <locale.h>

int
main (void)
{
  char buf[10];
  setlocale (LC_ALL, "vi_VN.TCVN-5712");
  return sprintf (buf, "%.*s", 2, "vi") != 2;
}
