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
 * Rewritten to be reentrant by Ulrich Drepper, 1995
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
   calling the setstate() function with the same array as was initialized
   with initstate().  By default, the package runs with 128 bytes of state
   information and generates far better random numbers than a linear
   congruential generator.  If the amount of state information is less than
   32 bytes, a simple linear congruential R.N.G. is used.  Internally, the
   state information is treated as an array of longs; the zeroth element of
   the array is the type of R.N.G. being used (small integer); the remainder
   of the array is the state information for the R.N.G.  Thus, 32 bytes of
   state information will give 7 longs worth of state information, which will
   allow a degree seven polynomial.  (Note: The zeroth word of state
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
   break value on the amount of state information (you need at least this many
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

struct random_poly_info
{
  char seps[MAX_TYPES - 1];
  char degrees[MAX_TYPES - 1];
};

static const struct random_poly_info random_poly_info =
{
  { SEP_1, SEP_2, SEP_3, SEP_4 },
  { DEG_1, DEG_2, DEG_3, DEG_4 },
};



/* Initialize the random number generator based on the given seed.  If the
   type is the trivial no-state-information type, just remember the seed.
   Otherwise, initializes state[] based on the given "seed" via a linear
   congruential generator.  Then, the pointers are set to known locations
   that are exactly rand_sep places apart.  Lastly, it cycles the state
   information a given number of times to get rid of any initial dependencies
   introduced by the L.C.R.N.G.  Note that the initialization of randtbl[]
   for default usage relies on values produced by this routine.  */
int
__srandom_r (seed, buf)
     unsigned int seed;
     struct random_data *buf;
{
  int type;
  int32_t *state;

  if (buf == NULL)
    goto fail;
  type = buf->rand_type;
  if ((unsigned)type >= MAX_TYPES)
    goto fail;

  /* We must make sure the seed is not 0.  Take arbitrarily 1 in this case.  */
  state = buf->state;
  if (seed == 0)
    seed = 1;
  state[0] = seed;
  if (type == TYPE_0)
    goto done;

    {
      int degree;
      long int word;
      int jc;
      int32_t *dst;
      int kc;
      int separation;

      degree = buf->rand_deg;
      jc = degree - 1;
      dst = state;
      word = seed;
      while (--jc >= 0)
	{
	  long int hi;
	  long int lo;

	  /* This does:
	       state[i] = (16807 * state[i - 1]) % 2147483647;
	     but avoids overflowing 31 bits.  */
	  ++dst;
	  hi = word / 127773;
	  lo = word % 127773;
	  word = 16807 * lo - 2836 * hi;
	  if (word < 0)
	    word += 2147483647;
	  *dst = word;
	}
      state = buf->state;
      degree = buf->rand_deg;
      separation = buf->rand_sep;
      buf->fptr = &state[separation];
      buf->rptr = &state[0];
      kc = 10 * degree;
      while (--kc >= 0)
	{
	  int32_t discard;
	  (void) __random_r (buf, &discard);
	}
    }
    goto done;

 fail:
  return -1;

 done:
  return 0;
}

weak_alias (__srandom_r, srandom_r)

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
  int type;
  int degree;
  int separation;
  int32_t *state;
  int mess;
  const struct random_poly_info *rpi;

  if (buf == NULL)
    goto fail;

  if (n >= BREAK_3)
    type = n < BREAK_4 ? TYPE_3 : TYPE_4;
  else if (n < BREAK_1)
    {
      if (n < BREAK_0)
	goto fail;
      type = TYPE_0;
    }
  else
    type = n < BREAK_2 ? TYPE_1 : TYPE_2;

  state = &((int32_t *) arg_state)[1];	/* First location.  */
  buf->state = state;
  mess = TYPE_0;
  if (type == TYPE_0)
    goto skip_to_here;

  rpi = &random_poly_info;
  degree = rpi->degrees[type - 1];
  separation = rpi->seps[type - 1];

  /* Must set END_PTR before srandom.  */
  buf->end_ptr = &state[degree];

  buf->rand_deg = degree;
  buf->rand_sep = separation;

  mess = (buf->rptr - state) * MAX_TYPES + type;

 skip_to_here:
  state[-1] = mess;

  __srandom_r (seed, buf);

  return 0;

 fail:
  return -1;
}

weak_alias (__initstate_r, initstate_r)

/* Restore the state from the given state array.
   Note: It is important that we also remember the locations of the pointers
   in the current state information, and restore the locations of the pointers
   from the old state information.  This is done by multiplexing the pointer
   location into the zeroth word of the state information. Note that due
   to the order in which things are done, it is OK to call setstate with the
   same state as the current state
   Returns a pointer to the old state information.  */
int
__setstate_r (arg_state, buf)
     void *arg_state;
     struct random_data *buf;
{
  int32_t *new_state;
  int type;
  int rear;
  int32_t *old_state;
  int32_t *ns1;
  int degree;
  int separation;
  int mess;
  int old_type;
  int new_mess;
  int *old_rptr;
  const struct random_poly_info *rpi;

  if (buf == NULL)
    return -1;

  old_type = buf->rand_type;
  old_state = buf->state;
  old_rptr = buf->rptr;
  mess = old_type;
  if (old_type != TYPE_0)
    mess += (old_rptr - old_state) * MAX_TYPES;
  old_state[-1] = mess;

  new_state = (int32_t *) arg_state;
  new_mess = new_state[0];
  type = new_mess % MAX_TYPES;
  rear = new_mess / MAX_TYPES;

  rpi = &random_poly_info;
  degree = rpi->degrees[type - 1];
  separation = rpi->seps[type - 1];
  if (rear >= degree)
    goto fail;

  ns1 = &new_state[1];
  if (type != TYPE_0)
    {
      int t;

      t = rear + separation;
      if (t >= degree)
	t -= degree;
      buf->rptr = &ns1[rear];
      buf->fptr = &ns1[t];
      buf->rand_deg = degree;
      buf->rand_sep = separation;
      buf->end_ptr = &ns1[degree];
    }

  return 0;

 fail:
  return -1;
}

weak_alias (__setstate_r, setstate_r)

/* If we are using the trivial TYPE_0 R.N.G., just do the old linear
   congruential bit.  Otherwise, we do our fancy trinomial stuff, which is the
   same in all the other cases due to all the global variables that have been
   set up.  The basic operation is to add the number at the rear pointer into
   the one at the front pointer.  Then both pointers are advanced to the next
   location cyclically in the table.  The value returned is the sum generated,
   reduced to 31 bits by throwing away the "least random" low bit.
   Returns a 31-bit random number.  */

int
__random_r (buf, result)
     struct random_data *buf;
     int32_t *result;
{
  int32_t *res_ptr;
  int rand_type;

  res_ptr = result;
  rand_type = buf->rand_type;
  if (buf == NULL || res_ptr == NULL)
    goto fail;

  if (rand_type == TYPE_0)
    goto old_style;

  {
    int32_t *fp0;
    int32_t *rp0;
    int32_t sum;
    int32_t fval;
    int32_t rval;
    int32_t rez;
    int32_t *fp1;
    int32_t *rp1;
    int32_t *end;
    int32_t *begin;

    /* 0 */
    fp0 = buf->fptr;
    rp0 = buf->rptr;

    /* 1 */
    fval = *fp0;
    rval = *rp0;
    fp1 = fp0 + 1;

    /* 2 */
    sum = fval + rval;
    rp1 = rp0 + 1;

    /* 3 */
    rez = (sum >> 1) & 0x7FFFFFFF;
    *fp0 = sum;
    end = buf->end_ptr;

    /* 4 */
    *res_ptr = rez;
    begin = buf->state;
    if (fp1 == end)
      fp1 = begin;
    if (rp1 == end)
      rp1 = begin;

    /* 5 */
    buf->fptr = fp1;
    buf->rptr = rp1;
  }
  goto done;

 old_style:
  {
    int32_t *state;
    int32_t rez;

    state = buf->state;
    rez = ((*state * 1103515245) + 12345) & 0x7FFFFFFF;
    *res_ptr = rez;
    *state = rez;
  }
  goto done;

 fail:
  return -1;

 done:
  return 0;
}

weak_alias (__random_r, random_r)
