#include <monetary.h>
#include <stdio.h>
#include <locale.h>

static int
check (const char *fmt, double n)
{
  int result;
  char buf[1000];

  result = strfmon (buf, sizeof buf, fmt, n) == -1;

  printf ("\"%s\"\n", buf);
  return result;
}

int
main (void)
{
  int result = 0;

  setlocale (LC_ALL, "");

  result |= check ("%n", 123.45);
  result |= check ("%n", -123.45);
  result |= check ("%n", 3456.781);

  result |= check ("%11n", 123.45);
  result |= check ("%11n", -123.45);
  result |= check ("%11n", 3456.781);

  result |= check ("%#5n", 123.45);
  result |= check ("%#5n", -123.45);
  result |= check ("%#5n", 3456.781);

  result |= check ("%=*#5n", 123.45);
  result |= check ("%=*#5n", -123.45);
  result |= check ("%=*#5n", 3456.781);

  result |= check ("%=0#5n", 123.45);
  result |= check ("%=0#5n", -123.45);
  result |= check ("%=0#5n", 3456.781);

  result |= check ("%^#5n", 123.45);
  result |= check ("%^#5n", -123.45);
  result |= check ("%^#5n", 3456.781);

  result |= check ("%^#5.0n", 123.45);
  result |= check ("%^#5.0n", -123.45);
  result |= check ("%^#5.0n", 3456.781);

  result |= check ("%^#5.4n", 123.45);
  result |= check ("%^#5.4n", -123.45);
  result |= check ("%^#5.4n", 3456.781);

  result |= check ("%(#5n", 123.45);
  result |= check ("%(#5n", -123.45);
  result |= check ("%(#5n", 3456.781);

  result |= check ("%!(#5n", 123.45);
  result |= check ("%!(#5n", -123.45);
  result |= check ("%!(#5n", 3456.781);

  return result;
}
