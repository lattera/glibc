/* Test program for tsearch et al.
   Copyright (C) 1997, 2000, 2001, 2003 Free Software Foundation, Inc.
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

#ifndef _GNU_SOURCE
# define _GNU_SOURCE	1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include <tst-stack-align.h>

#define SEED 0
#define BALANCED 1
#define PASSES 100

#if BALANCED
#include <math.h>
#define SIZE 1000
#else
#define SIZE 100
#endif

enum order
{
  ascending,
  descending,
  randomorder
};

enum action
{
  build,
  build_and_del,
  delete,
  find
};

/* Set to 1 if a test is flunked.  */
static int error = 0;

/* The keys we add to the tree.  */
static int x[SIZE];

/* Pointers into the key array, possibly permutated, to define an order
   for insertion/removal.  */
static int y[SIZE];

/* Flags set for each element visited during a tree walk.  */
static int z[SIZE];

/* Depths for all the elements, to check that the depth is constant for
   all three visits.  */
static int depths[SIZE];

/* Maximum depth during a tree walk.  */
static int max_depth;

static int stack_align_check[2];

/* Compare two keys.  */
static int
cmp_fn (const void *a, const void *b)
{
  if (!stack_align_check[0])
    stack_align_check[0] = TEST_STACK_ALIGN () ? -1 : 1;
  return *(const int *) a - *(const int *) b;
}

/* Permute an array of integers.  */
static void
memfry (int *string)
{
  int i;

  for (i = 0; i < SIZE; ++i)
    {
      int32_t j;
      int c;

      j = random () % SIZE;

      c = string[i];
      string[i] = string[j];
      string[j] = c;
    }
}

static void
walk_action (const void *nodep, const VISIT which, const int depth)
{
  int key = **(int **) nodep;

  if (!stack_align_check[1])
    stack_align_check[1] = TEST_STACK_ALIGN () ? -1 : 1;

  if (depth > max_depth)
    max_depth = depth;
  if (which == leaf || which == preorder)
    {
      ++z[key];
      depths[key] = depth;
    }
  else
    {
      if (depths[key] != depth)
	{
	  fputs ("Depth for one element is not constant during tree walk.\n",
		 stdout);
	}
    }
}

static void
walk_tree (void *root, int expected_count)
{
  int i;

  memset (z, 0, sizeof z);
  max_depth = 0;

  twalk (root, walk_action);
  for (i = 0; i < expected_count; ++i)
    if (z[i] != 1)
      {
	fputs ("Node was not visited.\n", stdout);
	error = 1;
      }

#if BALANCED
  if (max_depth > log (expected_count) * 2 + 2)
#else
  if (max_depth > expected_count)
#endif
    {
      fputs ("Depth too large during tree walk.\n", stdout);
      error = 1;
    }
}

/* Perform an operation on a tree.  */
static void
mangle_tree (enum order how, enum action what, void **root, int lag)
{
  int i;

  if (how == randomorder)
    {
      for (i = 0; i < SIZE; ++i)
	y[i] = i;
      memfry (y);
    }

  for (i = 0; i < SIZE + lag; ++i)
    {
      void *elem;
      int j, k;

      switch (how)
	{
	case randomorder:
	  if (i >= lag)
	    k = y[i - lag];
	  else
	    /* Ensure that the array index is within bounds.  */
	    k = y[(SIZE - i - 1 + lag) % SIZE];
	  j = y[i % SIZE];
	  break;

	case ascending:
	  k = i - lag;
	  j = i;
	  break;

	case descending:
	  k = SIZE - i - 1 + lag;
	  j = SIZE - i - 1;
	  break;

	default:
	  /* This never should happen, but gcc isn't smart enough to
	     recognize it.  */
	  abort ();
	}

      switch (what)
	{
	case build_and_del:
	case build:
	  if (i < SIZE)
	    {
	      if (tfind (x + j, (void *const *) root, cmp_fn) != NULL)
		{
		  fputs ("Found element which is not in tree yet.\n", stdout);
		  error = 1;
		}
	      elem = tsearch (x + j, root, cmp_fn);
	      if (elem == 0
		  || tfind (x + j, (void *const *) root, cmp_fn) == NULL)
		{
		  fputs ("Couldn't find element after it was added.\n",
			 stdout);
		  error = 1;
		}
	    }

	  if (what == build || i < lag)
	    break;

	  j = k;
	  /* fall through */

	case delete:
	  elem = tfind (x + j, (void *const *) root, cmp_fn);
	  if (elem == NULL || tdelete (x + j, root, cmp_fn) == NULL)
	    {
	      fputs ("Error deleting element.\n", stdout);
	      error = 1;
	    }
	  break;

	case find:
	  if (tfind (x + j, (void *const *) root, cmp_fn) == NULL)
	    {
	      fputs ("Couldn't find element after it was added.\n", stdout);
	      error = 1;
	    }
	  break;

	}
    }
}


int
main (int argc, char **argv)
{
  int total_error = 0;
  static char state[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
  void *root = NULL;
  int i, j;

  initstate (SEED, state, 8);

  for (i = 0; i < SIZE; ++i)
    x[i] = i;

  /* Do this loop several times to get different permutations for the
     random case.  */
  fputs ("Series I\n", stdout);
  for (i = 0; i < PASSES; ++i)
    {
      fprintf (stdout, "Pass %d... ", i + 1);
      fflush (stdout);
      error = 0;

      mangle_tree (ascending, build, &root, 0);
      mangle_tree (ascending, find, &root, 0);
      mangle_tree (descending, find, &root, 0);
      mangle_tree (randomorder, find, &root, 0);
      walk_tree (root, SIZE);
      mangle_tree (ascending, delete, &root, 0);

      mangle_tree (ascending, build, &root, 0);
      walk_tree (root, SIZE);
      mangle_tree (descending, delete, &root, 0);

      mangle_tree (ascending, build, &root, 0);
      walk_tree (root, SIZE);
      mangle_tree (randomorder, delete, &root, 0);

      mangle_tree (descending, build, &root, 0);
      mangle_tree (ascending, find, &root, 0);
      mangle_tree (descending, find, &root, 0);
      mangle_tree (randomorder, find, &root, 0);
      walk_tree (root, SIZE);
      mangle_tree (descending, delete, &root, 0);

      mangle_tree (descending, build, &root, 0);
      walk_tree (root, SIZE);
      mangle_tree (descending, delete, &root, 0);

      mangle_tree (descending, build, &root, 0);
      walk_tree (root, SIZE);
      mangle_tree (randomorder, delete, &root, 0);

      mangle_tree (randomorder, build, &root, 0);
      mangle_tree (ascending, find, &root, 0);
      mangle_tree (descending, find, &root, 0);
      mangle_tree (randomorder, find, &root, 0);
      walk_tree (root, SIZE);
      mangle_tree (randomorder, delete, &root, 0);

      for (j = 1; j < SIZE; j *= 2)
	{
	  mangle_tree (randomorder, build_and_del, &root, j);
	}

      fputs (error ? " failed!\n" : " ok.\n", stdout);
      total_error |= error;
    }

  fputs ("Series II\n", stdout);
  for (i = 1; i < SIZE; i *= 2)
    {
      fprintf (stdout, "For size %d... ", i);
      fflush (stdout);
      error = 0;

      mangle_tree (ascending, build_and_del, &root, i);
      mangle_tree (descending, build_and_del, &root, i);
      mangle_tree (ascending, build_and_del, &root, i);
      mangle_tree (descending, build_and_del, &root, i);
      mangle_tree (ascending, build_and_del, &root, i);
      mangle_tree (descending, build_and_del, &root, i);
      mangle_tree (ascending, build_and_del, &root, i);
      mangle_tree (descending, build_and_del, &root, i);

      fputs (error ? " failed!\n" : " ok.\n", stdout);
      total_error |= error;
    }

  for (i = 0; i < 2; ++i)
    if (stack_align_check[i] == 0)
      {
        printf ("stack alignment check %d not run\n", i);
        total_error |= 1;
      }
    else if (stack_align_check[i] != 1)
      {
        printf ("stack insufficiently aligned in check %d\n", i);
        total_error |= 1;
      }

  return total_error;
}
