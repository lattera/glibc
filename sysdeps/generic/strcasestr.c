/* Return the offset of one string within another.
   Copyright (C) 1994, 1996-2000, 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/*
 * My personal strstr() implementation that beats most other algorithms.
 * Until someone tells me otherwise, I assume that this is the
 * fastest implementation of strstr() in C.
 * I deliberately chose not to comment it.  You should have at least
 * as much fun trying to understand it, as I had to write it :-).
 *
 * Stephen R. van den Berg, berg@pool.informatik.rwth-aachen.de	*/

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <ctype.h>

#if defined _LIBC || defined HAVE_STRING_H
# include <string.h>
#endif

#ifdef _LIBC
# include <locale/localeinfo.h>
# define TOLOWER(c) __tolower_l ((unsigned char) c, loc)
#else
# define TOLOWER(c) _tolower (c)
#endif

typedef unsigned chartype;

#undef strcasestr
#undef __strcasestr

char *
__strcasestr (phaystack, pneedle)
     const char *phaystack;
     const char *pneedle;
{
  register const unsigned char *haystack, *needle;
  register chartype b, c;
#ifdef _LIBC
  __locale_t loc = _NL_CURRENT_LOCALE;
#endif

  haystack = (const unsigned char *) phaystack;
  needle = (const unsigned char *) pneedle;

  b = TOLOWER (*needle);
  if (b != '\0')
    {
      haystack--;				/* possible ANSI violation */
      do
	{
	  c = *++haystack;
	  if (c == '\0')
	    goto ret0;
	}
      while (TOLOWER (c) != (int) b);

      c = TOLOWER (*++needle);
      if (c == '\0')
	goto foundneedle;
      ++needle;
      goto jin;

      for (;;)
        {
          register chartype a;
	  register const unsigned char *rhaystack, *rneedle;

	  do
	    {
	      a = *++haystack;
	      if (a == '\0')
		goto ret0;
	      if (TOLOWER (a) == (int) b)
		break;
	      a = *++haystack;
	      if (a == '\0')
		goto ret0;
shloop:
	      ;
	    }
          while (TOLOWER (a) != (int) b);

jin:	  a = *++haystack;
	  if (a == '\0')
	    goto ret0;

	  if (TOLOWER (a) != (int) c)
	    goto shloop;

	  rhaystack = haystack-- + 1;
	  rneedle = needle;
	  a = TOLOWER (*rneedle);

	  if (TOLOWER (*rhaystack) == (int) a)
	    do
	      {
		if (a == '\0')
		  goto foundneedle;
		++rhaystack;
		a = TOLOWER (*++needle);
		if (TOLOWER (*rhaystack) != (int) a)
		  break;
		if (a == '\0')
		  goto foundneedle;
		++rhaystack;
		a = TOLOWER (*++needle);
	      }
	    while (TOLOWER (*rhaystack) == (int) a);

	  needle = rneedle;		/* took the register-poor approach */

	  if (a == '\0')
	    break;
        }
    }
foundneedle:
  return (char*) haystack;
ret0:
  return 0;
}

weak_alias (__strcasestr, strcasestr)
