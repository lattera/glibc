/* Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.org>, 2000.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <ctype.h>
#include <locale.h>
#include <stdio.h>
#include <wctype.h>
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
  int failures = 0;
  int status;

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
	  ++failures;
	}
    }

  printf ("\n%d failures in printf tests\n", failures);
  status = failures != 0;

  /* ctype tests.  This makes sure that the multibyte chracter digit
     representations are not handle in this table.  */
  for (cnt = 0; cnt < 256; ++cnt)
    if (cnt >= '0' && cnt <= '9')
      {
	if (! isdigit (cnt))
	  {
	    printf ("isdigit ('%c') == 0\n", cnt);
	    ++failures;
	  }
      }
    else
      {
	if (isdigit (cnt))
	  {
	    printf ("isdigit (%d) != 0\n", cnt);
	    ++failures;
	  }
      }

  printf ("\n%d failures in ctype tests\n", failures);
  status = failures != 0;

  /* wctype tests.  This makes sure the second set of digits is also
     recorded.  */
  for (cnt = 0; cnt < 256; ++cnt)
    if (cnt >= '0' && cnt <= '9')
      {
	if (! iswdigit (cnt))
	  {
	    printf ("iswdigit (L'%c') == 0\n", cnt);
	    ++failures;
	  }
      }
    else
      {
	if (iswdigit (cnt))
	  {
	    printf ("iswdigit (%d) != 0\n", cnt);
	    ++failures;
	  }
      }

  for (cnt = 0x2070; cnt < 0x2090; ++cnt)
    if (cnt >= 0x2080 && cnt <= 0x2089)
      {
	if (! iswdigit (cnt))
	  {
	    printf ("iswdigit (U%04X) == 0\n", cnt);
	    ++failures;
	  }
      }
    else
      {
	if (iswdigit (cnt))
	  {
	    printf ("iswdigit (U%04X) != 0\n", cnt);
	    ++failures;
	  }
      }

  printf ("\n%d failures in wctype tests\n", failures);
  status = failures != 0;

  return status;
}
