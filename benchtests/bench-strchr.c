/* Measure STRCHR functions.
   Copyright (C) 2013-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#define TEST_MAIN
#ifndef WIDE
# ifdef USE_FOR_STRCHRNUL
#  define TEST_NAME "strchrnul"
# else
#  define TEST_NAME "strchr"
# endif /* !USE_FOR_STRCHRNUL */
#else
# ifdef USE_FOR_STRCHRNUL
#  define TEST_NAME "wcschrnul"
# else
#  define TEST_NAME "wcschr"
# endif /* !USE_FOR_STRCHRNUL */
#endif /* WIDE */
#include "bench-string.h"

#ifndef WIDE
# ifdef USE_FOR_STRCHRNUL
#  define STRCHR strchrnul
#  define stupid_STRCHR stupid_STRCHRNUL
#  define simple_STRCHR simple_STRCHRNUL
# else
#  define STRCHR strchr
# endif /* !USE_FOR_STRCHRNUL */
# define STRLEN strlen
# define CHAR char
# define BIG_CHAR CHAR_MAX
# define MIDDLE_CHAR 127
# define SMALL_CHAR 23
# define UCHAR unsigned char
#else
# include <wchar.h>
# ifdef USE_FOR_STRCHRNUL
#  define STRCHR wcschrnul
#  define stupid_STRCHR stupid_WCSCHRNUL
#  define simple_STRCHR simple_WCSCHRNUL
# else
#  define STRCHR wcschr
# endif /* !USE_FOR_STRCHRNUL */
# define STRLEN wcslen
# define CHAR wchar_t
# define BIG_CHAR WCHAR_MAX
# define MIDDLE_CHAR 1121
# define SMALL_CHAR 851
# define UCHAR wchar_t
#endif /* WIDE */

#ifdef USE_FOR_STRCHRNUL
# define NULLRET(endptr) endptr
#else
# define NULLRET(endptr) NULL
#endif /* !USE_FOR_STRCHRNUL */


typedef CHAR *(*proto_t) (const CHAR *, int);

CHAR *
simple_STRCHR (const CHAR *s, int c)
{
  for (; *s != (CHAR) c; ++s)
    if (*s == '\0')
      return NULLRET ((CHAR *) s);
  return (CHAR *) s;
}

CHAR *
stupid_STRCHR (const CHAR *s, int c)
{
  size_t n = STRLEN (s) + 1;

  while (n--)
    if (*s++ == (CHAR) c)
      return (CHAR *) s - 1;
  return NULLRET ((CHAR *) s - 1);
}

IMPL (stupid_STRCHR, 0)
IMPL (simple_STRCHR, 0)
IMPL (STRCHR, 1)

static void
do_one_test (impl_t *impl, const CHAR *s, int c, const CHAR *exp_res)
{
  size_t i, iters = INNER_LOOP_ITERS;
  timing_t start, stop, cur;

  TIMING_NOW (start);
  for (i = 0; i < iters; ++i)
    {
      CALL (impl, s, c);
    }
  TIMING_NOW (stop);

  TIMING_DIFF (cur, start, stop);

  TIMING_PRINT_MEAN ((double) cur, (double) iters);
}

static void
do_test (size_t align, size_t pos, size_t len, int seek_char, int max_char)
/* For wcschr: align here means align not in bytes,
   but in wchar_ts, in bytes it will equal to align * (sizeof (wchar_t))
   len for wcschr here isn't in bytes but it's number of wchar_t symbols.  */
{
  size_t i;
  CHAR *result;
  CHAR *buf = (CHAR *) buf1;
  align &= 15;
  if ((align + len) * sizeof (CHAR) >= page_size)
    return;

  for (i = 0; i < len; ++i)
    {
      buf[align + i] = 32 + 23 * i % max_char;
      if (buf[align + i] == seek_char)
	buf[align + i] = seek_char + 1;
      else if (buf[align + i] == 0)
	buf[align + i] = 1;
    }
  buf[align + len] = 0;

  if (pos < len)
    {
      buf[align + pos] = seek_char;
      result = buf + align + pos;
    }
  else if (seek_char == 0)
    result = buf + align + len;
  else
    result = NULLRET (buf + align + len);

  printf ("Length %4zd, alignment in bytes %2zd:",
	  pos, align * sizeof (CHAR));

  FOR_EACH_IMPL (impl, 0)
    do_one_test (impl, buf + align, seek_char, result);

  putchar ('\n');
}

int
test_main (void)
{
  size_t i;

  test_init ();

  printf ("%20s", "");
  FOR_EACH_IMPL (impl, 0)
    printf ("\t%s", impl->name);
  putchar ('\n');

  for (i = 1; i < 8; ++i)
    {
      do_test (0, 16 << i, 2048, SMALL_CHAR, MIDDLE_CHAR);
      do_test (i, 16 << i, 2048, SMALL_CHAR, MIDDLE_CHAR);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (i, 64, 256, SMALL_CHAR, MIDDLE_CHAR);
      do_test (i, 64, 256, SMALL_CHAR, BIG_CHAR);
    }

  for (i = 0; i < 32; ++i)
    {
      do_test (0, i, i + 1, SMALL_CHAR, MIDDLE_CHAR);
      do_test (0, i, i + 1, SMALL_CHAR, BIG_CHAR);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (0, 16 << i, 2048, 0, MIDDLE_CHAR);
      do_test (i, 16 << i, 2048, 0, MIDDLE_CHAR);
    }

  for (i = 1; i < 8; ++i)
    {
      do_test (i, 64, 256, 0, MIDDLE_CHAR);
      do_test (i, 64, 256, 0, BIG_CHAR);
    }

  for (i = 0; i < 32; ++i)
    {
      do_test (0, i, i + 1, 0, MIDDLE_CHAR);
      do_test (0, i, i + 1, 0, BIG_CHAR);
    }

  return ret;
}

#include <support/test-driver.c>
