#include <locale.h>
#include <stdio.h>
#include <sys/types.h>


#define ZERO  "\xe2\x82\x80"
#define ONE   "\xe2\x82\x81"
#define TWO   "\xe2\x82\x82"
#define THREE "\xe2\x82\x83"
#define FOUR  "\xe2\x82\x84"
#define FIVE  "\xe2\x82\x85"
#define SIX   "\xe2\x82\x86"
#define SEVEN "\xe2\x82\x87"
#define EIGHT "\xe2\x82\x88"
#define NINE  "\xe2\x82\x89"

static struct printf_int_test
{
  int n;
  const char *format;
  const char *expected;
} printf_int_tests[] =
{
  {       0, "%I'10d", "       " ZERO },
  {       1, "%I'10d", "       " ONE },
  {       2, "%I'10d", "       " TWO },
  {       3, "%I'10d", "       " THREE },
  {       4, "%I'10d", "       " FOUR },
  {       5, "%I'10d", "       " FIVE },
  {       6, "%I'10d", "       " SIX },
  {       7, "%I'10d", "       " SEVEN },
  {       8, "%I'10d", "       " EIGHT },
  {       9, "%I'10d", "       " NINE },
  {      11, "%I'10d", "    " ONE ONE },
  {      12, "%I'10d", "    " ONE TWO },
  {     123, "%I10d",  " " ONE TWO THREE },
  {     123, "%I'10d", " " ONE TWO THREE },
  {    1234, "%I10d",  ONE TWO THREE FOUR },
  {    1234, "%I'10d", ONE "," TWO THREE FOUR },
  {   12345, "%I'10d", ONE TWO "," THREE FOUR FIVE },
  {  123456, "%I'10d", ONE TWO THREE "," FOUR FIVE SIX },
  { 1234567, "%I'10d", ONE "," TWO THREE FOUR "," FIVE SIX SEVEN }
};



int
main (void)
{
  int cnt;
  int printf_failures = 0;

  if (setlocale (LC_ALL, "test7") == NULL)
    {
      puts ("cannot set locale `test7'");
      exit (1);
    }

  /* First: printf tests.  */
  for (cnt = 0; cnt < sizeof (printf_int_tests) / sizeof (printf_int_tests[0]);
       ++cnt)
    {
      char buf[100];
      ssize_t n;

      n = snprintf (buf, sizeof buf, printf_int_tests[cnt].format,
		    printf_int_tests[cnt].n);

      if (n != strlen (printf_int_tests[cnt].expected)
	  || strcmp (buf, printf_int_tests[cnt].expected) != 0)
	{
	  printf ("%3d: got \"%s\", expected \"%s\"\n",
		  cnt, buf, printf_int_tests[cnt].expected);
	  ++printf_failures;
	}
    }

  printf ("\n%d failures in printf tests\n", printf_failures);

  return printf_failures != 0;
}
