/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * This is derived from the Berkeley source:
 *	@(#)random.c	5.5 (Berkeley) 7/6/88
 * It was reworked for the GNU C Library by Roland McGrath.
 * Rewritten to be reentrent by Ulrich Drepper, 1995
 */

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>


/* An improved random number generation package.  In addition to the standard
   rand()/srand() like interface, this package also has a special state info
   interface.  The initstate() routine is called with a seed, an array of
   bytes, and a count of how many bytes are being passed in; this array is
   then initialized to contain information for random number generation with
   that much state information.  Good sizes for the amount of state
   information are 32, 64, 128, and 256 bytes.  The state can be switched by
   calling the setstate() function with the same array as was initiallized
   with initstate().  By default, the package runs with 128 bytes of state
   information and generates far better random numbers than a linear
   congruential generator.  If the amount of state information is less than
   32 bytes, a simple linear congruential R.N.G. is used.  Internally, the
   state information is treated as an array of longs; the zeroeth element of
   the array is the type of R.N.G. being used (small integer); the remainder
   of the array is the state information for the R.N.G.  Thus, 32 bytes of
   state information will give 7 longs worth of state information, which will
   allow a degree seven polynomial.  (Note: The zeroeth word of state
   information also has some other information stored in it; see setstate
   for details).  The random number generation technique is a linear feedback
   shift register approach, employing trinomials (since there are fewer terms
   to sum up that way).  In this approach, the least significant bit of all
   the numbers in the state table will act as a linear feedback shift register,
   and will have period 2^deg - 1 (where deg is the degree of the polynomial
   being used, assuming that the polynomial is irreducible and primitive).
   The higher order bits will have longer periods, since their values are
   also influenced by pseudo-random carries out of the lower bits.  The
   total period of the generator is approximately deg*(2**deg - 1); thus
   doubling the amount of state information has a vast influence on the
   period of the generator.  Note: The deg*(2**deg - 1) is an approximation
   only good for large deg, when the period of the shift register is the
   dominant factor.  With deg equal to seven, the period is actually much
   longer than the 7*(2**7 - 1) predicted by this formula.  */



/* For each of the currently supported random number generators, we have a
   break value on the amount of state information (you need at least thi
   bytes of state info to support this random number generator), a degree for
   the polynomial (actually a trinomial) that the R.N.G. is based on, and
   separation between the two lower order coefficients of the trinomial.  */

/* Linear congruential.  */
#define	TYPE_0		0
#define	BREAK_0		8
#define	DEG_0		0
#define	SEP_0		0

/* x**7 + x**3 + 1.  */
#define	TYPE_1		1
#define	BREAK_1		32
#define	DEG_1		7
#define	SEP_1		3

/* x**15 + x + 1.  */
#define	TYPE_2		2
#define	BREAK_2		64
#define	DEG_2		15
#define	SEP_2		1

/* x**31 + x**3 + 1.  */
#define	TYPE_3		3
#define	BREAK_3		128
#define	DEG_3		31
#define	SEP_3		3

/* x**63 + x + 1.  */
#define	TYPE_4		4
#define	BREAK_4		256
#define	DEG_4		63
#define	SEP_4		1


/* Array versions of the above information to make code run faster.
   Relies on fact that TYPE_i == i.  */

#define	MAX_TYPES	5	/* Max number of types above.  */

static const int degrees[MAX_TYPES] = { DEG_0, DEG_1, DEG_2, DEG_3, DEG_4 };
static const int seps[MAX_TYPES] = { SEP_0, SEP_1, SEP_2, SEP_3, SEP_4 };




/* Initialize the random number generator based on the given seed.  If the
   type is the trivial no-state-information type, just remember the seed.
   Otherwise, initializes state[] based on the given "seed" via a linear
   congruential generator.  Then, the pointers are set to known locations
   that are exactly rand_sep places apart.  Lastly, it cycles the state
   information a given number of times to get rid of any initial dependencies
   introduced by the L.C.R.N.G.  Note that the initialization of randtbl[]
   for default usage relies on values produced by this routine.  */
int
__srandom_r (x, buf)
     unsigned int x;
     struct random_data *buf;
{
  if (buf == NULL || buf->rand_type < TYPE_0 || buf->rand_type > TYPE_4)
    return -1;

  buf->state[0] = x;
  if (buf->rand_type != TYPE_0)
    {
      long int i;
      for (i = 1; i < buf->rand_deg; ++i)
	{
	  /* This does:
	       state[i] = (16807 * state[i - 1]) % 2147483647;
	     but avoids overflowing 31 bits.  */
	  long int hi = buf->state[i - 1] / 127773;
	  long int lo = buf->state[i - 1] % 127773;
	  long int test = 16807 * lo - 2836 * hi;
	  buf->state[i] = test + (test < 0 ? 2147483647 : 0);
	}
      buf->fptr = &buf->state[buf->rand_sep];
      buf->rptr = &buf->state[0];
      for (i = 0; i < 10 * buf->rand_deg; ++i)
	{
	  long int discard;
	  (void) __random_r (buf, &discard);
	}
    }

  return 0;
}

weak_alias (__srandom_r, srandom_r)
weak_alias (__srandom_r, srand_r)

/* Initialize the state information in the given array of N bytes for
   future random number generation.  Based on the number of bytes we
   are given, and the break values for the different R.N.G.'s, we choose
   the best (largest) one we can and set things up for it.  srandom is
   then called to initialize the state information.  Note that on return
   from srandom, we set state[-1] to be the type multiplexed with the current
   value of the rear pointer; this is so successive calls to initstate won't
   lose this information and will be able to restart with setstate.
   Note: The first thing we do is save the current state, if any, just like
   setstate so that it doesn't matter when initstate is called.
   Returns a pointer to the old state.  */
int
__initstate_r (seed, arg_state, n, buf)
     unsigned int seed;
     void *arg_state;
     size_t n;
     struct random_data *buf;
{
  if (buf == NULL)
    return -1;

  if (buf->rand_type == TYPE_0)
    buf->state[-1] = buf->rand_type;
  else
    buf->state[-1] = (MAX_TYPES * (buf->rptr - buf->state)) + buf->rand_type;
  if (n < BREAK_1)
    {
      if (n < BREAK_0)
	{
	  errno = EINVAL;
	  return -1;
	}
      buf->rand_type = TYPE_0;
      buf->rand_deg = DEG_0;
      buf->rand_sep = SEP_0;
    }
  else if (n < BREAK_2)
    {
      buf->rand_type = TYPE_1;
      buf->rand_deg = DEG_1;
      buf->rand_sep = SEP_1;
    }
  else if (n < BREAK_3)
    {
      buf->rand_type = TYPE_2;
      buf->rand_deg = DEG_2;
      buf->rand_sep = SEP_2;
    }
  else if (n < BREAK_4)
    {
      buf->rand_type = TYPE_3;
      buf->rand_deg = DEG_3;
      buf->rand_sep = SEP_3;
    }
  else
    {
      buf->rand_type = TYPE_4;
      buf->rand_deg = DEG_4;
      buf->rand_sep = SEP_4;
    }

  buf->state = &((long int *) arg_state)[1];	/* First location.  */
  /* Must set END_PTR before srandom.  */
  buf->end_ptr = &buf->state[buf->rand_deg];

  __srandom_r (seed, buf);

  if (buf->rand_type == TYPE_0)
    buf->state[-1] = buf->rand_type;
  else
    buf->state[-1] = (MAX_TYPES * (buf->rptr - buf->state)) + buf->rand_type;

  return 0;
}

weak_alias (__initstate_r, initstate_r)

/* Restore the state from the given state array.
   Note: It is important that we also remember the locations of the pointers
   in the current state information, and restore the locations of the pointers
   from the old state information.  This is done by multiplexing the pointer
   location into the zeroeth word of the state information. Note that due
   to the order in which things are done, it is OK to call setstate with the
   same state as the current state
   Returns a pointer to the old state information.  */
int
__setstate_r (arg_state, buf)
     void *arg_state;
     struct random_data *buf;
{
  long int *new_state = (long int *) arg_state;
  int type = new_state[0] % MAX_TYPES;
  int rear = new_state[0] / MAX_TYPES;

  if (buf == NULL)
    return -1;

  if (buf->rand_type == TYPE_0)
    buf->state[-1] = buf->rand_type;
  else
    buf->state[-1] = (MAX_TYPES * (buf->rptr - buf->state)) + buf->rand_type;

  switch (type)
    {
    case TYPE_0:
    case TYPE_1:
    case TYPE_2:
    case TYPE_3:
    case TYPE_4:
      buf->rand_type = type;
      buf->rand_deg = degrees[type];
      buf->rand_sep = seps[type];
      break;
    default:
      /* State info munged.  */
      errno = EINVAL;
      return -1;
    }

  buf->state = &new_state[1];
  if (buf->rand_type != TYPE_0)
    {
      buf->rptr = &buf->state[rear];
      buf->fptr = &buf->state[(rear + buf->rand_sep) % buf->rand_deg];
    }
  /* Set end_ptr too.  */
  buf->end_ptr = &buf->state[buf->rand_deg];

  return 0;
}

weak_alias (__setstate_r, setstate_r)

/* If we are using the trivial TYPE_0 R.N.G., just do the old linear
   congruential bit.  Otherwise, we do our fancy trinomial stuff, which is the
   same in all ther other cases due to all the global variables that have been
   set up.  The basic operation is to add the number at the rear pointer into
   the one at the front pointer.  Then both pointers are advanced to the next
   location cyclically in the table.  The value returned is the sum generated,
   reduced to 31 bits by throwing away the "least random" low bit.
   Note: The code takes advantage of the fact that both the front and
   rear pointers can't wrap on the same call by not testing the rear
   pointer if the front one has wrapped.  Returns a 31-bit random number.  */

int
__random_r (buf, result)
     struct random_data *buf;
     long int *result;
{
  if (buf == NULL || result == NULL)
    return -1;

  if (buf->rand_type == TYPE_0)
    {
      buf->state[0] = ((buf->state[0] * 1103515245) + 12345) & LONG_MAX;
      *result = buf->state[0];
    }
  else
    {
      *buf->fptr += *buf->rptr;
      /* Chucking least random bit.  */
      *result = (*buf->fptr >> 1) & LONG_MAX;
      ++buf->fptr;
      if (buf->fptr >= buf->end_ptr)
	{
	  buf->fptr = buf->state;
	  ++buf->rptr;
	}
      else
	{
	  ++buf->rptr;
	  if (buf->rptr >= buf->end_ptr)
	    buf->rptr = buf->state;
	}
    }
  return 0;
}

weak_alias (__random_r, random_r)
