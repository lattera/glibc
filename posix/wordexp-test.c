/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <wordexp.h>
#include <stdio.h>
#include <stdlib.h>

struct test_case_struct
{
  int retval;
  const char *env;
  const char *words;
  int flags;
  int wordc;
  const char *wordv[10];
} test_case[] =
  {
    { 0, NULL, "one", 0, 1, { "one", } },
    { 0, NULL, "one two", 0, 2, { "one", "two", } },
    { 0, NULL, "one two three", 0, 3, { "one", "two", "three", } },
    { 0, NULL, "~root", 0, 1, { "/root", } },
    { 0, "foo", "${var}", 0, 1, { "foo", } },
    { 0, "foo", "$var", 0, 1, { "foo", } },
    { 0, NULL, "\"quoted\"", 0, 1, { "quoted", } },
    { -1, NULL, NULL, 0, 0, { NULL, } },
  };

int
main (int argc, char * argv[])
{
  wordexp_t we;
  int test;
  int fail = 0;
  int retval;
  int i;

  setenv ("IFS", " \t\n", 1);
  for (test = 0; test_case[test].retval != -1; test++)
    {
      int bzzzt = 0;

      if (test_case[test].env)
	setenv ("var", test_case[test].env, 1);
      else
	unsetenv ("var");

      printf ("Test %d: ", test);
      retval = wordexp (test_case[test].words, &we, test_case[test].flags);

      if (retval != test_case[test].retval ||
	  we.we_wordc != test_case[test].wordc)
	bzzzt = 1;
      else
	for (i = 0; i < we.we_wordc; i++)
	  if (strcmp (test_case[test].wordv[i], we.we_wordv[i]) != 0)
	    {
	      bzzzt = 1;
	      break;
	    }

      if (bzzzt)
	{
	  ++fail;
	  printf ("FAILED\n");
	  printf ("Test words: <%s>, need retval %d, wordc %d\n",
		  test_case[test].words, test_case[test].retval,
		  test_case[test].wordc);
	  printf ("Got retval %d, wordc %d: ", retval, we.we_wordc);
	  for (i = 0; i < we.we_wordc; i++)
	    printf ("<%s> ", we.we_wordv[i]);
	  printf ("\n");
	}
      else
	printf ("OK\n");

      wordfree (&we);
    }

  return fail;
}
