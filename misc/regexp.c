/* Define function and variables for the obsolete <regexp.h> interface.
   Copyright (C) 1996, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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

#define __DO_NOT_DEFINE_COMPILE
#include <regexp.h>

/* Define the variables used for the interface.  */
char *loc1;
char *loc2;

/* Although we do not support the use we define this variable as well.  */
char *locs;


/* Find the next match in STRING.  The compiled regular expression is
   found in the buffer starting at EXPBUF.  `loc1' will return the
   first character matched and `loc2' points to the next unmatched
   character.  */
extern int __step (const char *string, const char *expbuf);
int
__step (const char *string, const char *expbuf)
{
  regmatch_t match;	/* We only need info about the full match.  */

  expbuf += __alignof (regex_t *);
  expbuf -= (expbuf - ((const char *) 0)) % __alignof__ (regex_t *);

  if (__regexec ((const regex_t *) expbuf, string, 1, &match, REG_NOTEOL)
      == REG_NOMATCH)
    return 0;

  loc1 = (char *) string + match.rm_so;
  loc2 = (char *) string + match.rm_eo;
  return 1;
}
weak_alias (__step, step)


/* Match the beginning of STRING with the compiled regular expression
   in EXPBUF.  If the match is successful `loc2' will contain the
   position of the first unmatched character.  */
extern int __advance (const char *string, const char *expbuf);
int
__advance (const char *string, const char *expbuf)
{
  regmatch_t match;	/* We only need info about the full match.  */

  expbuf += __alignof__ (regex_t *);
  expbuf -= (expbuf - ((const char *) 0)) % __alignof__ (regex_t *);

  if (__regexec ((const regex_t *) expbuf, string, 1, &match, REG_NOTEOL)
      == REG_NOMATCH
      /* We have to check whether the check is at the beginning of the
	 buffer.  */
      || match.rm_so != 0)
    return 0;

  loc2 = (char *) string + match.rm_eo;
  return 1;
}
weak_alias (__advance, advance)
