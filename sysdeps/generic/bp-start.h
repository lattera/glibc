/* Bounded-pointer checking macros for C.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Greg McGary <greg@mcgary.org>

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


#if __BOUNDED_POINTERS__

  /* The command-line arg vector and environment vector come to us from
     the OS as an unbounded pointer to an array of unbounded strings.
     The user's main expects argv and __environ to be bounded pointers
     to arrays of bounded strings.  */
# define INIT_ARGV_and_ENVIRON \
  do {									      \
    int envc;								      \
    for (envc = 0; *ubp_ev; ubp_ev++, envc++)				      \
      ;									      \
    ubp_ev -= envc;							      \
									      \
    /* GKM FIXME: we could save some space by allocating only enough for      \
       the additional low & high words, and destructively rewriting	      \
       argv in place.  */						      \
    __ptrvalue (argv) = __ptrlow (argv)					      \
      = alloca ((argc + envc + 2) * sizeof (*argv));			      \
    __ptrhigh (argv) = __ptrvalue (argv) + argc + 1;			      \
    __ptrvalue (__environ) = __ptrlow (__environ) = __ptrhigh (argv);	      \
    __ptrhigh (__environ) = __ptrvalue (__environ) + envc + 1;		      \
    boundify_vector (__environ, ubp_ev);				      \
    boundify_vector (argv, ubp_av);					      \
  } while (0)


/* Copy an unbounded vector of unbounded strings into a bounded
   counterpart.  */

static void
boundify_vector (char **dest, char *__unbounded *__unbounded src)
{
  char *__unbounded s;
  for (; *src; src++, dest++)
    {
      __ptrvalue (*dest) = __ptrlow (*dest) = *src;
      __ptrhigh (*dest) = src[1];
    }
  *dest = 0;
  /* The OS lays out strings contiguously in vector order,
     so  */
  for (s = __ptrvalue (dest[-1]); *s; s++)
    ;
  __ptrhigh (dest[-1]) = ++s;
}

#else

# define INIT_ARGV_and_ENVIRON __environ = ubp_ev

#endif
