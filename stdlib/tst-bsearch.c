/* Copyright (C) 2000, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2000.

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

#include <stdio.h>
#include <stdlib.h>
#include <tst-stack-align.h>

struct entry
{
  int val;
  const char *str;
} arr[] =
{
  { 0, "zero" },
  { 1, "one" },
  { 2, "two" },
  { 3, "three" },
  { 4, "four" },
  { 5, "five" },
  { 6, "six" },
  { 7, "seven" },
  { 8, "eight" },
  { 9, "nine" },
  { 10, "ten" }
};
#define narr (sizeof (arr) / sizeof (arr[0]))

static int align_check;

static int
comp (const void *p1, const void *p2)
{
  struct entry *e1 = (struct entry *) p1;
  struct entry *e2 = (struct entry *) p2;

  if (!align_check)
    align_check = TEST_STACK_ALIGN () ? -1 : 1;

  return e1->val - e2->val;
}


int
main (void)
{
  size_t cnt;
  int result = 0;
  struct entry key;
  struct entry *res;

  for (cnt = 0; cnt < narr; ++cnt)
    {

      key.val = arr[cnt].val;

      res = (struct entry *) bsearch (&key, arr, narr, sizeof (arr[0]), comp);
      if (res == NULL)
	{
	  printf ("entry %zd not found\n", cnt);
	  result = 1;
	}
      else if (res != &arr[cnt])
	{
	  puts ("wrong entry returned");
	  result = 1;
	}
    }

  /* And some special tests that shouldn't find any entry.  */
  key.val = -1;
  res = (struct entry *) bsearch (&key, arr, narr, sizeof (arr[0]), comp);
  if (res != NULL)
    {
      puts ("found an entry that's not there");
      result = 1;
    }

  key.val = 11;
  res = (struct entry *) bsearch (&key, arr, narr, sizeof (arr[0]), comp);
  if (res != NULL)
    {
      puts ("found an entry that's not there");
      result = 1;
    }

  key.val = 11;
  res = (struct entry *) bsearch (&key, arr, 0, sizeof (arr[0]), comp);
  if (res != NULL)
    {
      puts ("found an entry that's not there");
      result = 1;
    }

  /* Now the array contains only one element - no entry should be found.  */
  for (cnt = 0; cnt < narr; ++cnt)
    {
      key.val = arr[cnt].val;

      res = (struct entry *) bsearch (&key, &arr[5], 1, sizeof (arr[0]), comp);
      if (cnt == 5)
	{
	  if (res == NULL)
	    {
	      printf ("entry %zd not found\n", cnt);
	      result = 1;
	    }
	  else if (res != &arr[cnt])
	    {
	      puts ("wrong entry returned");
	      result = 1;
	    }
	}
      else if (res != NULL)
	{
	  puts ("found an entry that's not there");
	  result = 1;
	}
    }

  if (align_check == 0)
    {
      puts ("alignment not checked");
      result = 1;
    }
  else if (align_check == -1)
    {
      puts ("stack not sufficiently aligned");
      result = 1;
    }

  if (result == 0)
    puts ("all OK");

  return result;
}
