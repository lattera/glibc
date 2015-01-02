/* Generate table of tests in tst-strtod-round.c from
   tst-strtod-round-data.
   Copyright (C) 2012-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#define _GNU_SOURCE
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpfr.h>

/* Work around incorrect ternary value from mpfr_strtofr
   <https://sympa.inria.fr/sympa/arc/mpfr/2012-08/msg00005.html>.  */
#define WORKAROUND

static int
string_to_fp (mpfr_t f, const char *s, mpfr_rnd_t rnd)
{
#ifdef WORKAROUND
  mpfr_t f2;
  mpfr_init2 (f2, 100000);
  int r0 = mpfr_strtofr (f2, s, NULL, 0, rnd);
  int r = mpfr_set (f, f2, rnd);
  mpfr_subnormalize (f, r, rnd);
  mpfr_clear (f2);
  return r0 | r;
#else
  int r = mpfr_strtofr (f, s, NULL, 0, rnd);
  mpfr_subnormalize (f, r, rnd);
  return r;
#endif
}

static void
print_fp (mpfr_t f, const char *suffix, const char *suffix2)
{
  if (mpfr_inf_p (f))
    mpfr_printf ("\t%sINFINITY%s", mpfr_signbit (f) ? "-" : "", suffix2);
  else
    mpfr_printf ("\t%Ra%s%s", f, suffix, suffix2);
}

static void
round_str (const char *s, const char *suffix,
	   int prec, int emin, int emax, bool ibm_ld)
{
  mpfr_t f;
  mpfr_set_default_prec (prec);
  mpfr_set_emin (emin);
  mpfr_set_emax (emax);
  mpfr_init (f);
  int r = string_to_fp (f, s, MPFR_RNDD);
  if (ibm_ld)
    {
      assert (prec == 106 && emin == -1073 && emax == 1024);
      /* The maximum value in IBM long double has discontiguous
	 mantissa bits.  */
      mpfr_t max_value;
      mpfr_init2 (max_value, 107);
      mpfr_set_str (max_value, "0x1.fffffffffffff7ffffffffffffcp+1023", 0,
		    MPFR_RNDN);
      if (mpfr_cmpabs (f, max_value) > 0)
	r = 1;
      mpfr_clear (max_value);
    }
  mpfr_printf ("\t%s,\n", r ? "false" : "true");
  print_fp (f, suffix, ",\n");
  string_to_fp (f, s, MPFR_RNDN);
  print_fp (f, suffix, ",\n");
  string_to_fp (f, s, MPFR_RNDZ);
  print_fp (f, suffix, ",\n");
  string_to_fp (f, s, MPFR_RNDU);
  print_fp (f, suffix, "");
  mpfr_clear (f);
}

static void
round_for_all (const char *s)
{
  static const struct fmt {
    const char *suffix;
    int prec;
    int emin;
    int emax;
    bool ibm_ld;
  } formats[7] = {
    { "f", 24, -148, 128, false },
    { "", 53, -1073, 1024, false },
    { "L", 53, -1073, 1024, false },
    /* This is the Intel extended float format.  */
    { "L", 64, -16444, 16384, false },
    /* This is the Motorola extended float format.  */
    { "L", 64, -16445, 16384, false },
    { "L", 106, -1073, 1024, true },
    { "L", 113, -16493, 16384, false },
  };
  mpfr_printf ("  TEST (\"");
  const char *p;
  for (p = s; *p; p++)
    {
      putchar (*p);
      if ((p - s) % 60 == 59 && p[1])
	mpfr_printf ("\"\n\t\"");
    }
  mpfr_printf ("\",\n");
  int i;
  for (i = 0; i < 7; i++)
    {
      round_str (s, formats[i].suffix, formats[i].prec,
		 formats[i].emin, formats[i].emax, formats[i].ibm_ld);
      if (i < 6)
	mpfr_printf (",\n");
    }
  mpfr_printf ("),\n");
}

int
main (void)
{
  char *p = NULL;
  size_t len;
  ssize_t nbytes;
  while ((nbytes = getline (&p, &len, stdin)) != -1)
    {
      if (p[nbytes - 1] == '\n')
	p[nbytes - 1] = 0;
      round_for_all (p);
      free (p);
      p = NULL;
    }
  return 0;
}
