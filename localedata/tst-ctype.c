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


static const char lower[] = "abcdefghijklmnopqrstuvwxyz";
static const char upper[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char digits[] = "0123456789";


#define FAIL(str, args...) \
  {									      \
    printf ("      " str "\n", ##args);					      \
    ++errors;								      \
  }


int
main (void)
{
  const char *cp;
  const char *cp2;
  int errors = 0;

  setlocale (LC_ALL, "");

  printf ("Testing the ctype data of the `%s' locale\n",
	  setlocale (LC_CTYPE, NULL));

#if 0
  /* Just for debugging.  */
  {
    /* Contents of the class array.  */
    int n = 0;

    printf ("upper = %04x  lower = %04x  alpha = %04x  digit = %04x  xdigit = %04x\n"
	    "space = %04x  print = %04x  graph = %04x  blank = %04x  cntrl  = %04x\n"
	    "punct = %04x  alnum = %04x\n",
	    _ISupper, _ISlower, _ISalpha, _ISdigit, _ISxdigit,
	    _ISspace, _ISprint, _ISgraph, _ISblank, _IScntrl,
	    _ISpunct, _ISalnum);

    while (n < 256)
      {
	if (n % 8 == 0)
	  printf ("%02x: ", n);
	printf ("%04x%s", __ctype_b[n], (n + 1) % 8 == 0 ? "\n" : " ");
	++n;
      }
  }
#endif

  puts ("  Test of ASCII character range\n    special NUL byte handling");
  if (isupper ('\0'))
    FAIL ("isupper ('\\0') is true");
  if (islower ('\0'))
    FAIL ("islower ('\\0') is true");
  if (isalpha ('\0'))
    FAIL ("isalpha ('\\0') is true");
  if (isdigit ('\0'))
    FAIL ("isdigit ('\\0') is true");
  if (isxdigit ('\0'))
    FAIL ("isxdigit ('\\0') is true");
  if (isspace ('\0'))
    FAIL ("isspace ('\\0') is true");
  if (isprint ('\0'))
    FAIL ("isprint ('\\0') is true");
  if (isgraph ('\0'))
    FAIL ("isgraph ('\\0') is true");
  if (isblank ('\0'))
    FAIL ("isblank ('\\0') is true");
  if (iscntrl ('\0'))
    FAIL ("iscntrl ('\\0') is true");
  if (ispunct ('\0'))
    FAIL ("ispunct ('\\0') is true");
  if (isalnum ('\0'))
    FAIL ("isalnum ('\\0') is true");

  puts ("    islower()");
  for (cp = lower; *cp != '\0'; ++cp)
    if (! islower (*cp))
      FAIL ("islower ('%c') not true", *cp);
  for (cp = upper; *cp != '\0'; ++cp)
    if (islower (*cp))
      FAIL ("islower ('%c') is true", *cp);
  for (cp = digits; *cp != '\0'; ++cp)
    if (islower (*cp))
      FAIL ("islower ('%c') is true", *cp);

  puts ("    isupper()");
  for (cp = lower; *cp != '\0'; ++cp)
    if (isupper (*cp))
      FAIL ("isupper ('%c') is true", *cp);
  for (cp = upper; *cp != '\0'; ++cp)
    if (! isupper (*cp))
      FAIL ("isupper ('%c') not true", *cp);
  for (cp = digits; *cp != '\0'; ++cp)
    if (isupper (*cp))
      FAIL ("isupper ('%c') is true", *cp);

  puts ("    isalpha()");
  for (cp = lower; *cp != '\0'; ++cp)
    if (! isalpha (*cp))
      FAIL ("isalpha ('%c') not true", *cp);
  for (cp = upper; *cp != '\0'; ++cp)
    if (! isalpha (*cp))
      FAIL ("isalpha ('%c') not true", *cp);
  for (cp = digits; *cp != '\0'; ++cp)
    if (isalpha (*cp))
      FAIL ("isalpha ('%c') is true", *cp);

  puts ("    isdigit()");
  for (cp = lower; *cp != '\0'; ++cp)
    if (isdigit (*cp))
      FAIL ("isdigit ('%c') is true", *cp);
  for (cp = upper; *cp != '\0'; ++cp)
    if (isdigit (*cp))
      FAIL ("isdigit ('%c') is true", *cp);
  for (cp = digits; *cp != '\0'; ++cp)
    if (! isdigit (*cp))
      FAIL ("isdigit ('%c') not true", *cp);

  puts ("    isxdigit()");
  for (cp = lower; *cp != '\0'; ++cp)
    if ((! isxdigit (*cp) && cp - lower < 6)
	|| (isxdigit (*cp) && cp - lower >= 6))
      FAIL ("isxdigit ('%c') %s true", *cp, cp - upper < 6 ? "not" : "is");
  for (cp = upper; *cp != '\0'; ++cp)
    if ((! isxdigit (*cp) && cp - upper < 6)
	|| (isxdigit (*cp) && cp - upper >= 6))
      FAIL ("isxdigit ('%c') %s true", *cp, cp - upper < 6 ? "not" : "is");
  for (cp = digits; *cp != '\0'; ++cp)
    if (! isxdigit (*cp))
      FAIL ("isxdigit ('%c') not true", *cp);

  puts ("    isspace()");
  for (cp = lower; *cp != '\0'; ++cp)
    if (isspace (*cp))
      FAIL ("isspace ('%c') is true", *cp);
  for (cp = upper; *cp != '\0'; ++cp)
    if (isspace (*cp))
      FAIL ("isspace ('%c') is true", *cp);
  for (cp = digits; *cp != '\0'; ++cp)
    if (isspace (*cp))
      FAIL ("isspace ('%c') is true", *cp);

  puts ("    isprint()");
  for (cp = lower; *cp != '\0'; ++cp)
    if (! isprint (*cp))
      FAIL ("isprint ('%c') not true", *cp);
  for (cp = upper; *cp != '\0'; ++cp)
    if (! isprint (*cp))
      FAIL ("isprint ('%c') not true", *cp);
  for (cp = digits; *cp != '\0'; ++cp)
    if (! isprint (*cp))
      FAIL ("isprint ('%c') not true", *cp);

  puts ("    isgraph()");
  for (cp = lower; *cp != '\0'; ++cp)
    if (! isgraph (*cp))
      FAIL ("isgraph ('%c') not true", *cp);
  for (cp = upper; *cp != '\0'; ++cp)
    if (! isgraph (*cp))
      FAIL ("isgraph ('%c') not true", *cp);
  for (cp = digits; *cp != '\0'; ++cp)
    if (! isgraph (*cp))
      FAIL ("isgraph ('%c') not true", *cp);

  puts ("    isblank()");
  for (cp = lower; *cp != '\0'; ++cp)
    if (isblank (*cp))
      FAIL ("isblank ('%c') is true", *cp);
  for (cp = upper; *cp != '\0'; ++cp)
    if (isblank (*cp))
      FAIL ("isblank ('%c') is true", *cp);
  for (cp = digits; *cp != '\0'; ++cp)
    if (isblank (*cp))
      FAIL ("isblank ('%c') is true", *cp);

  puts ("    iscntrl()");
  for (cp = lower; *cp != '\0'; ++cp)
    if (iscntrl (*cp))
      FAIL ("iscntrl ('%c') is true", *cp);
  for (cp = upper; *cp != '\0'; ++cp)
    if (iscntrl (*cp))
      FAIL ("iscntrl ('%c') is true", *cp);
  for (cp = digits; *cp != '\0'; ++cp)
    if (iscntrl (*cp))
      FAIL ("iscntrl ('%c') is true", *cp);

  puts ("    ispunct()");
  for (cp = lower; *cp != '\0'; ++cp)
    if (ispunct (*cp))
      FAIL ("ispunct ('%c') is true", *cp);
  for (cp = upper; *cp != '\0'; ++cp)
    if (ispunct (*cp))
      FAIL ("ispunct ('%c') is true", *cp);
  for (cp = digits; *cp != '\0'; ++cp)
    if (ispunct (*cp))
      FAIL ("ispunct ('%c') is true", *cp);

  puts ("    isalnum()");
  for (cp = lower; *cp != '\0'; ++cp)
    if (! isalnum (*cp))
      FAIL ("isalnum ('%c') not true", *cp);
  for (cp = upper; *cp != '\0'; ++cp)
    if (! isalnum (*cp))
      FAIL ("isalnum ('%c') not true", *cp);
  for (cp = digits; *cp != '\0'; ++cp)
    if (! isalnum (*cp))
      FAIL ("isalnum ('%c') not true", *cp);


  puts ("    tolower()");
  for (cp = lower; *cp != '\0'; ++cp)
    if (tolower (*cp) != *cp)
      FAIL ("tolower ('%c') != '%c'", *cp, *cp);
  for (cp = upper, cp2 = lower; *cp != '\0'; ++cp, ++cp2)
    if (tolower (*cp) != *cp2)
      FAIL ("tolower ('%c') != '%c'", *cp, *cp2);
  for (cp = digits; *cp != '\0'; ++cp)
    if (tolower (*cp) != *cp)
      FAIL ("tolower ('%c') != '%c'", *cp, *cp);

  puts ("    toupper()");
  for (cp = lower, cp2 = upper; *cp != '\0'; ++cp, ++cp2)
    if (toupper (*cp) != *cp2)
      FAIL ("toupper ('%c') != '%c'", *cp, *cp2);
  for (cp = upper; *cp != '\0'; ++cp)
    if (toupper (*cp) != *cp)
      FAIL ("toupper ('%c') != '%c'", *cp, *cp);
  for (cp = digits; *cp != '\0'; ++cp)
    if (toupper (*cp) != *cp)
      FAIL ("toupper ('%c') != '%c'", *cp, *cp);


  if (errors != 0)
    {
      printf ("  %d errors for `%s' locale\n\n\n", errors,
	      setlocale (LC_ALL, NULL));
      return 1;
    }

  printf ("  No errors for `%s' locale\n\n\n", setlocale (LC_ALL, NULL));
  return 0;
}
