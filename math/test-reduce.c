/* Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Geoffrey Keating <Geoff.Keating@anu.edu.au>, 1997.

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

/* This is a generic program for comparing two precisions of a one-input
   mathematical function.  It is amazingly good at detecting when GCC
   folds constants improperly.  */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <math.h>
#include <ieee754.h>
#include <fenv.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TSMALL float
#define RSMALL(rfun) ({ unsigned rnum = (rfun); *(float *) &rnum; })
#define TBIG double
#define XDIFF (24)
#define REDUCE(x) \
   ({ union ieee754_float u = { x }; u.ieee.exponent = 0x80; x = u.f; })
#define ABS(x) fabs(x)

#define string_0(x) #x
#define string_1(x) string_0(x)
#define TBIG_NAME string_1(TBIG)
#define TSMALL_NAME string_1(TSMALL)

#define R_NEAREST 1
#define R_ZERO 2
#define R_UP 4
#define R_DOWN 8
#define R_ALL (R_NEAREST|R_ZERO|R_UP|R_DOWN)
static fenv_t rmodes[4];
static const char * const rmnames[4] =
{ "near","zero","+Inf","-Inf" };

static int quiet = 0;

#ifdef FE_ALL_INVALID
static const int invalid_exceptions = (FE_ALL_INVALID
				       | FE_INVALID | FE_DIVBYZERO);
#else
static const int invalid_exceptions = (FE_INVALID | FE_DIVBYZERO);
#endif

static int
checkit (char *fname,
	 TSMALL (*fsmall) (TSMALL), TBIG (*fbig) (TBIG),
	 unsigned smalltries, unsigned largetries)
{
  unsigned int i, nerrors = 0, nwarn;

  int tryone (TSMALL fval)
    {
      int rmode;
      int excepts, exceptsb;
      TSMALL fres;
      TBIG fvalb, fresb, diff;
      char warn;

      fvalb = (TBIG) fval;

      for (rmode = 0; rmode < 4; ++rmode)
	{
	  fesetenv (rmodes + rmode);
	  fres = fsmall (fval);
	  excepts = fetestexcept (invalid_exceptions);
	  fesetenv (rmodes + rmode);
	  fresb = fbig (fvalb);
	  exceptsb = fetestexcept (invalid_exceptions);

	  if (excepts != exceptsb)
	    {
	      unsigned char *fvp = (unsigned char *) &fval;
	      size_t j;

	      printf ("%s(", fname);
	      for (j = 0; j < sizeof (TSMALL); j++)
		printf ("%02x", fvp[j]);
	      printf ("),%s: exceptions %s: %08x, %s: %08x\n",
		      rmnames[rmode],
		      TBIG_NAME, exceptsb, TSMALL_NAME, excepts);
	      if (++nerrors > 10)
		return 1;
	    }

	  diff = ABS (fres - (TSMALL) fresb);
	  if (fres == (TSMALL) fresb
	      || isnan (fres) && isnan (fresb)
	      || logb (fresb) - logb (diff) < XDIFF - 1)
	    continue;

	  if (logb (fresb) - logb (diff) < XDIFF)
	    {
	      if (++nwarn > 10 || quiet)
		continue;
	      warn = 'w';
	    }
	  else
	    {
	      if (++nerrors > 10)
		return 1;
	      warn = 'e';
	    }

	  {
	    TSMALL fresbs = (TSMALL) fresb;
	    unsigned char *fvp = (unsigned char *) &fval;
	    unsigned char *frp = (unsigned char *) &fres;
	    unsigned char *frpb = (unsigned char *) &fresb;
	    unsigned char *frpbs = (unsigned char *) &fresbs;
	    size_t j;

	    printf ("%s(",fname);
	    for (j = 0; j < sizeof (TSMALL); ++j)
	      printf ("%02x", fvp[j]);
	    printf ("),%s: %s ", rmnames[rmode], TBIG_NAME);
	    for (j = 0; j < sizeof (TBIG); ++j)
	      printf ("%02x", frpb[j]);
	    printf (" (");
	    for (j = 0; j < sizeof (TSMALL); ++j)
	      printf ("%02x", frpbs[j]);
	    printf ("), %s ", TSMALL_NAME);
	    for (j = 0; j < sizeof (TSMALL); ++j)
	      printf ("%02x", frp[j]);
	    printf (" %c\n", warn);
	  }
	}
      return 0;
    }

  nwarn = 0;
  for (i = 0; i < smalltries; i++)
    {
      TSMALL fval;

      fval = RSMALL (rand () ^ rand () << 16);
      REDUCE (fval);
      if (tryone (fval))
	break;
    }

  printf ("%s-small: %d errors, %d (%0.2f%%) inaccuracies\n",
	  fname, nerrors, nwarn,
	  nwarn * 0.25 / ((double) smalltries));

  nwarn = 0;
  for (i = 0; i < largetries; ++i)
    {
      TSMALL fval;

      fval = RSMALL (rand () ^ rand () << 16);
      if (tryone (fval))
	break;
    }

  printf ("%s-large: %d errors, %d (%0.2f%%) inaccuracies\n",
	  fname, nerrors, nwarn,
	  nwarn * 0.25 / ((double) largetries));
  return nerrors == 0;
}

int
main (void)
{
  int r;

  _LIB_VERSION = _IEEE_;

  /* Set up environments for rounding modes.  */
  fesetenv (FE_DFL_ENV);
  fesetround (FE_TONEAREST);
  fegetenv (rmodes + 0);
  fesetround (FE_TOWARDSZERO);
  fegetenv (rmodes + 1);
  fesetround (FE_UPWARD);
  fegetenv (rmodes + 2);
  fesetround (FE_DOWNWARD);
  fegetenv (rmodes + 3);

  /* Seed the RNG.  */
  srand (time (0));

  /* Do it.  */
  r  = checkit ("exp2", exp2f, exp2, 1 << 20, 1 << 15);
  r &= checkit ("exp",  expf,  exp,  1 << 20, 1 << 15);
  return r ? 0 : 1;
}
