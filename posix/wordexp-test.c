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

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <wordexp.h>

#define IFS ", \n\t"

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
    /* Simple word-splitting */
    { 0, NULL, "one", 0, 1, { "one", } },
    { 0, NULL, "one two", 0, 2, { "one", "two", } },
    { 0, NULL, "one two three", 0, 3, { "one", "two", "three", } },

    /* Simple parameter expansion */
    { 0, "foo", "${var}", 0, 1, { "foo", } },
    { 0, "foo", "$var", 0, 1, { "foo", } },

    /* Simple quote removal */
    { 0, NULL, "\"quoted\"", 0, 1, { "quoted", } },
    { 0, "foo", "\"$var\"\"$var\"", 0, 1, { "foofoo", } },
    { 0, NULL, "'singly-quoted'", 0, 1, { "singly-quoted", } },

    /* Simple command substitution */
    { 0, NULL, "$(echo hello)", 0, 1, { "hello", } },
    { 0, NULL, "$( (echo hello) )", 0, 1, { "hello", } },

    /* Simple arithmetic expansion */
    { 0, NULL, "$((1 + 1))", 0, 1, { "2", } },
    { 0, NULL, "$((2-3))", 0, 1, { "-1", } },
    { 0, NULL, "$((-1))", 0, 1, { "-1", } },

    /* Field splitting */
    { 0, NULL, " \tfoo\t\tbar ", 0, 2, { "foo", "bar", } },
    { 0, NULL, "  red  , white blue", 0, 3, { "red", "white", "blue", } },

    /* Advanced parameter expansion */
    { 0, NULL, "${var:-bar}", 0, 1, { "bar", } },
    { 0, NULL, "${var-bar}", 0, 1, { "bar", } },
    { 0, "", "${var:-bar}", 0, 1, { "bar", } },
    { 0, "foo", "${var:-bar}", 0, 1, { "foo", } },
    { 0, "", "${var-bar}", 0, 0, { NULL, } },
    { 0, NULL, "${var:=bar}", 0, 1, { "bar", } },
    { 0, NULL, "${var=bar}", 0, 1, { "bar", } },
    { 0, "", "${var:=bar}", 0, 1, { "bar", } },
    { 0, "foo", "${var:=bar}", 0, 1, { "foo", } },
    { 0, "", "${var=bar}", 0, 0, { NULL, } },
    { 0, "foo", "${var:?bar}", 0, 1, { "foo", } },
    { 0, NULL, "${var:+bar}", 0, 0, { NULL, } },
    { 0, NULL, "${var+bar}", 0, 0, { NULL, } },
    { 0, "", "${var:+bar}", 0, 0, { NULL, } },
    { 0, "foo", "${var:+bar}", 0, 1, { "bar", } },
    { 0, "", "${var+bar}", 0, 1, { "bar", } },
    { 0, "12345", "${#var}", 0, 1, { "5", } },

    { 0, "banana", "${var%na*}", 0, 1, { "bana", } },
    { 0, "banana", "${var%%na*}", 0, 1, { "ba", } },
    { 0, "borabora-island", "${var#*bora}", 0, 1, { "bora-island", } },
    { 0, "borabora-island", "${var##*bora}", 0, 1, {"-island", } },

    { -1, NULL, NULL, 0, 0, { NULL, } },
  };

static int testit (struct test_case_struct *tc);

void
command_line_test (const char *words)
{
  wordexp_t we;
  int i;
  int retval = wordexp (words, &we, 0);
  printf ("wordexp returned %d\n", retval);
  for (i = 0; i < we.we_wordc; i++)
    printf ("we_wordv[%d] = \"%s\"\n", i, we.we_wordv[i]);
}

int
main (int argc, char *argv[])
{
  struct passwd *pw;
  int test;
  int fail = 0;

  setenv ("IFS", IFS, 1);
  for (test = 0; test_case[test].retval != -1; test++)
    if (testit (&test_case[test]))
      ++fail;

  if (argc > 1)
    {
      command_line_test (argv[1]);
      return 0;
    }

  pw = getpwnam ("root");
  if (pw != NULL)
    {
      struct test_case_struct ts;

      ts.retval = 0;
      ts.env = NULL;
      ts.words = "~root";
      ts.flags = 0;
      ts.wordc = 1;
      ts.wordv[0] = pw->pw_dir;

      if (testit (&ts))
	++fail;
    }

  return fail != 0;
}


static int
testit (struct test_case_struct *tc)
{
  static int test;
  int retval;
  wordexp_t we;
  int bzzzt = 0;
  int i;

  if (tc->env)
    setenv ("var", tc->env, 1);
  else
    unsetenv ("var");

  printf ("Test %d: ", ++test);
  retval = wordexp (tc->words, &we, tc->flags);

  if (retval != tc->retval || we.we_wordc != tc->wordc)
    bzzzt = 1;
  else
    for (i = 0; i < we.we_wordc; ++i)
      if (strcmp (tc->wordv[i], we.we_wordv[i]) != 0)
	{
	  bzzzt = 1;
	  break;
	}

  if (bzzzt)
    {
      printf ("FAILED\n");
      printf ("Test words: <%s>, need retval %d, wordc %d\n",
	      tc->words, tc->retval, tc->wordc);
      printf ("Got retval %d, wordc %d: ", retval, we.we_wordc);
      for (i = 0; i < we.we_wordc; ++i)
	printf ("<%s> ", we.we_wordv[i]);
      printf ("\n");
    }
  else
    printf ("OK\n");

  wordfree (&we);

  return bzzzt;
}
