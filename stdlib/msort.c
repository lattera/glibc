/* An alternative to qsort, with an identical interface.
   This file is part of the GNU C Library.
   Copyright (C) 1992, 1995-1997, 1999, 2000, 2001, 2002
   Free Software Foundation, Inc.
   Original Implementation by Mike Haertel, September 1988.
   Towers of Hanoi Mergesort by Roger Sayle, January 2002.

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

#include <alloca.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <memcopy.h>
#include <errno.h>


/* Check whether pointer P is aligned for access by type T. */
#define TYPE_ALIGNED(P,T)  (((char *) (P) - (char *) 0) % __alignof__ (T) == 0)


static int hanoi_sort (char *b, size_t n, size_t s,
                        __compar_fn_t cmp, char *t);
static int hanoi_sort_int (int *b, size_t n,
                           __compar_fn_t cmp, int *t);
#if INT_MAX != LONG_MAX
static int hanoi_sort_long (long int *b, size_t n,
                            __compar_fn_t cmp, long int *t);
#endif
static void msort_with_tmp (void *b, size_t n, size_t s,
			    __compar_fn_t cmp, void *t);


/* This routine implements "Towers of Hanoi Mergesort".  The algorithm
   sorts the n elements of size s pointed to by array b using comparison
   function cmp.  The argument t points to a suitable temporary buffer.
   If the return value is zero, the sorted array is returned in b, and
   for non-zero return values the sorted array is returned in t.  */
static int
hanoi_sort (char *b, size_t n, size_t s, __compar_fn_t cmp, char *t)
{
  size_t n1, n2;
  char *b1,*b2;
  char *t1,*t2;
  char *s1,*s2;
  size_t size;
  int result;
  char *ptr;

  if (n <= 1)
    return 0;

  if (n == 2)
    {
      b2 = b + s;
      if ((*cmp) (b, b2) <= 0)
	return 0;
      memcpy (__mempcpy (t, b2, s), b, s);
      return 1;
    }

  n1 = n/2;
  n2 = n - n1;
  /* n1 < n2!  */

  size = n1 * s;
  b1 = b;
  b2 = b + size;

  t1 = t;
  t2 = t + size;

  /* Recursively call hanoi_sort to sort the two halves of the array.
     Depending upon the return values, determine the values s1 and s2
     the locations of the two sorted subarrays, ptr, the location to
     contain the sorted array and result, the return value for this
     function.  Note that "ptr = result? t : b".  */
  if (hanoi_sort (b1, n1, s, cmp, t1))
    {
      if (hanoi_sort (b2, n2, s, cmp, t2))
	{
	  result = 0;
	  ptr = b;
	  s1 = t1;
	  s2 = t2;
	}
      else
	{
	  result = 0;
	  ptr = b;
	  s1 = t1;
	  s2 = b2;
	}
    }
  else
    {
      if (hanoi_sort (b2, n2, s, cmp, t2))
	{
	  result = 1;
	  ptr = t;
	  s1 = b1;
	  s2 = t2;
	}
      else
	{
	  result = 1;
	  ptr = t;
	  s1 = b1;
	  s2 = b2;
	}
    }

  /*  Merge the two sorted arrays s1 and s2 of n1 and n2 elements
      respectively, placing the result in ptr.  On entry, n1 > 0
      && n2 > 0, and with each iteration either n1 or n2 is decreased
      until either reaches zero, and the loop terminates via return.  */
  for (;;)
    {
      if ((*cmp) (s1, s2) <= 0)
	{
	  ptr = (char *) __mempcpy (ptr, s1, s);
	  s1 += s;
	  --n1;
	  if (n1 == 0)
            {
              if (ptr != s2)
                memcpy (ptr, s2, n2 * s);
              return result;
            }
	}
      else
	{
	  ptr = (char *) __mempcpy (ptr, s2, s);
	  s2 += s;
	  --n2;
	  if (n2 == 0)
	    {
	      memcpy (ptr, s1, n1 * s);
	      return result;
	    }
        }
    }
}


/* This routine is a variant of hanoi_sort that is optimized for the
   case where items to be sorted are the size of ints, and both b and
   t are suitably aligned.  The parameter s in not needed as it is
   known to be sizeof(int).  */
static int
hanoi_sort_int (int *b, size_t n, __compar_fn_t cmp, int *t)
{
  size_t n1, n2;
  int *b1,*b2;
  int *t1,*t2;
  int *s1,*s2;
  int result;
  int *ptr;

  if (n <= 1)
    return 0;

  if (n == 2)
    {
      if ((*cmp) (b, b + 1) <= 0)
	return 0;
      t[0] = b[1];
      t[1] = b[0];
      return 1;
    }

  n1 = n/2;
  n2 = n - n1;
  /* n1 < n2!  */

  b1 = b;
  b2 = b + n1;

  t1 = t;
  t2 = t + n1;

  /* Recursively call hanoi_sort_int to sort the two halves.  */
  if (hanoi_sort_int (b1, n1, cmp, t1))
    {
      if (hanoi_sort_int (b2, n2, cmp, t2))
	{
	  result = 0;
	  ptr = b;
	  s1 = t1;
	  s2 = t2;
	}
      else
	{
	  result = 0;
	  ptr = b;
	  s1 = t1;
	  s2 = b2;
	}
    }
  else
    {
      if (hanoi_sort_int (b2, n2, cmp, t2))
	{
	  result = 1;
	  ptr = t;
	  s1 = b1;
	  s2 = t2;
	}
      else
	{
	  result = 1;
	  ptr = t;
	  s1 = b1;
	  s2 = b2;
	}
    }

  /*  Merge n1 elements from s1 and n2 elements from s2 into ptr.  */
  for (;;)
    {
      if ((*cmp) (s1, s2) <= 0)
	{
	  *ptr++ = *s1++;
	  --n1;
	  if (n1 == 0)
            {
              if (ptr != s2)
                memcpy (ptr, s2, n2 * sizeof (int));
              return result;
            }
	}
      else
	{
	  *ptr++ = *s2++;
	  --n2;
	  if (n2 == 0)
	    {
	      memcpy (ptr, s1, n1 * sizeof (int));
	      return result;
	    }
	}
    }
}


#if INT_MAX != LONG_MAX
/* This routine is a variant of hanoi_sort that is optimized for the
   case where items to be sorted are the size of longs, and both b and
   t are suitably aligned.  The parameter s in not needed as it is
   known to be sizeof(long).  In case sizeof(int)== sizeof(long) we
   do not need this code since it would be the same as hanoi_sort_int.  */
static int
hanoi_sort_long (long int *b, size_t n, __compar_fn_t cmp, long int *t)
{
  size_t n1, n2;
  long int *b1,*b2;
  long int *t1,*t2;
  long int *s1,*s2;
  int result;
  long int *ptr;

  if (n <= 1)
    return 0;

  if (n == 2)
    {
      if ((*cmp) (b, b + 1) <= 0)
	return 0;
      t[0] = b[1];
      t[1] = b[0];
      return 1;
    }

  n1 = n/2;
  n2 = n - n1;
  /* n1 < n2!  */

  b1 = b;
  b2 = b + n1;

  t1 = t;
  t2 = t + n1;

  /* Recursively call hanoi_sort_long to sort the two halves.  */
  if (hanoi_sort_long (b1, n1, cmp, t1))
    {
      if (hanoi_sort_long (b2, n2, cmp, t2))
	{
	  result = 0;
	  ptr = b;
	  s1 = t1;
	  s2 = t2;
	}
      else
	{
	  result = 0;
	  ptr = b;
	  s1 = t1;
	  s2 = b2;
	}
    }
  else
    {
      if (hanoi_sort_long (b2, n2, cmp, t2))
	{
	  result = 1;
	  ptr = t;
	  s1 = b1;
	  s2 = t2;
	}
      else
	{
	  result = 1;
	  ptr = t;
	  s1 = b1;
	  s2 = b2;
	}
    }

  /*  Merge n1 elements from s1 and n2 elements from s2 into ptr.  */
  for (;;)
    {
      if ((*cmp) (s1, s2) <= 0)
	{
	  *ptr++ = *s1++;
	  --n1;
	  if (n1 == 0)
            {
              if (ptr != s2)
                memcpy (ptr, s2, n2 * sizeof (long));
              return result;
            }
	}
      else
	{
	  *ptr++ = *s2++;
	  --n2;
	  if (n2 == 0)
	    {
	      memcpy (ptr, s1, n1 * sizeof (long));
	      return result;
	    }
        }
    }
}
#endif


/* This routine preserves the original interface to msort_with_tmp and
   determines which variant of hanoi_sort to call, based upon item size
   and alignment.  */

static void
msort_with_tmp (void *b, size_t n, size_t s, __compar_fn_t cmp, void *t)
{
  const size_t size = n * s;

  if (s == sizeof (int) && TYPE_ALIGNED (b, int))
    {
      if (hanoi_sort_int (b, n, cmp, t))
        memcpy (b, t, size);
    }
#if INT_MAX != LONG_MAX
  else if (s == sizeof (long int) && TYPE_ALIGNED (b, long int))
    {
      if (hanoi_sort_long (b, n, cmp, t))
        memcpy (b, t, size);
    }
#endif
  else
    {
      /* Call the generic implementation.  */
      if (hanoi_sort (b, n, s, cmp, t))
        memcpy (b, t, size);
    }
}

void
qsort (void *b, size_t n, size_t s, __compar_fn_t cmp)
{
  const size_t size = n * s;

  if (size < 1024)
    {
      void *buf = __alloca (size);

      /* The temporary array is small, so put it on the stack.  */
      msort_with_tmp (b, n, s, cmp, buf);
    }
  else
    {
      /* We should avoid allocating too much memory since this might
	 have to be backed up by swap space.  */
      static long int phys_pages;
      static int pagesize;

      if (phys_pages == 0)
	{
	  phys_pages = __sysconf (_SC_PHYS_PAGES);

	  if (phys_pages == -1)
	    /* Error while determining the memory size.  So let's
	       assume there is enough memory.  Otherwise the
	       implementer should provide a complete implementation of
	       the `sysconf' function.  */
	    phys_pages = (long int) (~0ul >> 1);

	  /* The following determines that we will never use more than
	     a quarter of the physical memory.  */
	  phys_pages /= 4;

	  pagesize = __sysconf (_SC_PAGESIZE);
	}

      /* Just a comment here.  We cannot compute
	   phys_pages * pagesize
	   and compare the needed amount of memory against this value.
	   The problem is that some systems might have more physical
	   memory then can be represented with a `size_t' value (when
	   measured in bytes.  */

      /* If the memory requirements are too high don't allocate memory.  */
      if ((long int) (size / pagesize) > phys_pages)
	_quicksort (b, n, s, cmp);
      else
	{
	  /* It's somewhat large, so malloc it.  */
	  int save = errno;
	  char *tmp = malloc (size);
	  if (tmp == NULL)
	    {
	      /* Couldn't get space, so use the slower algorithm
		 that doesn't need a temporary array.  */
	      __set_errno (save);
	      _quicksort (b, n, s, cmp);
	    }
	  else
	    {
	      __set_errno (save);
	      msort_with_tmp (b, n, s, cmp, tmp);
	      free (tmp);
	    }
	}
    }
}
