/* Copyright (C) 1997, 1998, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Geoffrey Keating <Geoff.Keating@anu.edu.au>, 1997.

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
#include <math.h>
#include <gmp.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>

#define PRINT_ERRORS 0

#define TOL 80
#define N2 18
#define FRAC (32*4)

#define mpbpl (CHAR_BIT * sizeof (mp_limb_t))
#define SZ (FRAC / mpbpl + 1)
typedef mp_limb_t mp1[SZ], mp2[SZ * 2];

/* This string has 101 hex digits.  */
static const char exp1[102] = "2" /* point */
"b7e151628aed2a6abf7158809cf4f3c762e7160f38b4da56a7"
"84d9045190cfef324e7738926cfbe5f4bf8d8d8c31d763da07";
static const char exp_m1[102] = "0" /* point */
"5e2d58d8b3bcdf1abadec7829054f90dda9805aab56c773330"
"24b9d0a507daedb16400bf472b4215b8245b669d90d27a5aea";

static const char hexdig[] = "0123456789abcdef";

static void
print_mpn_fp (const mp_limb_t *x, unsigned int dp, unsigned int base)
{
   unsigned int i;
   mp1 tx;

   memcpy (tx, x, sizeof (mp1));
   if (base == 16)
     fputs ("0x", stdout);
   assert (x[SZ-1] < base);
   fputc (hexdig[x[SZ - 1]], stdout);
   fputc ('.', stdout);
   for (i = 0; i < dp; i++)
     {
       tx[SZ - 1] = 0;
       mpn_mul_1 (tx, tx, SZ, base);
       assert (tx[SZ - 1] < base);
       fputc (hexdig[tx[SZ - 1]], stdout);
     }
}

static void
read_mpn_hex(mp_limb_t *x, const char *str)
{
  int i;

  memset (x, 0, sizeof (mp1));
  for (i = -1; i < 100 && i < FRAC / 4; ++i)
    x[(FRAC - i * 4 - 4) / mpbpl] |= ((strchr (hexdig, str[i + 1]) - hexdig)
				      << (FRAC - i * 4 - 4) % mpbpl);
}

static mp_limb_t *get_log2(void) __attribute__((const));
static mp_limb_t *
get_log2(void)
{
  static mp1 log2_m;
  static int log2_m_inited = 0;
  static const char log2[102] = "0" /* point */
    "b17217f7d1cf79abc9e3b39803f2f6af40f343267298b62d8a"
    "0d175b8baafa2be7b876206debac98559552fb4afa1b10ed2e";

  if (!log2_m_inited)
    {
      read_mpn_hex (log2_m, log2);
      log2_m_inited = 1;
    }
  return log2_m;
}

/* Compute e^x.  */
static void
exp_mpn (mp1 ex, mp1 x)
{
   unsigned int n;
   mp1 xp;
   mp2 tmp;
   mp_limb_t chk, round;
   mp1 tol;

   memset (xp, 0, sizeof (mp1));
   memset (ex, 0, sizeof (mp1));
   xp[FRAC / mpbpl] = (mp_limb_t)1 << FRAC % mpbpl;
   memset (tol, 0, sizeof (mp1));
   tol[(FRAC - TOL) / mpbpl] = (mp_limb_t)1 << (FRAC - TOL) % mpbpl;

   n = 0;

   do
     {
       /* Calculate sum(x^n/n!) until the next term is sufficiently small.  */

       mpn_mul_n (tmp, xp, x, SZ);
       assert(tmp[SZ * 2 - 1] == 0);
       if (n > 0)
	 round = mpn_divmod_1 (xp, tmp + FRAC / mpbpl, SZ, n);
       chk = mpn_add_n (ex, ex, xp, SZ);
       assert (chk == 0);
       ++n;
       assert (n < 80); /* Catch too-high TOL.  */
     }
   while (n < 10 || mpn_cmp (xp, tol, SZ) >= 0);
}

/* Calculate 2^x.  */
static void
exp2_mpn (mp1 ex, mp1 x)
{
  mp2 tmp;
  mpn_mul_n (tmp, x, get_log2 (), SZ);
  assert(tmp[SZ * 2 - 1] == 0);
  exp_mpn (ex, tmp + FRAC / mpbpl);
}


static int
mpn_bitsize(const mp_limb_t *SRC_PTR, mp_size_t SIZE)
{
  int i, j;
  for (i = SIZE - 1; i > 0; --i)
    if (SRC_PTR[i] != 0)
      break;
  for (j = mpbpl - 1; j >= 0; --j)
    if ((SRC_PTR[i] & (mp_limb_t)1 << j) != 0)
      break;

  return i * mpbpl + j;
}

int
main (void)
{
  mp1 ex, x, xt, e2, e3;
  int i;
  int errors = 0;
  int failures = 0;
  mp1 maxerror;
  int maxerror_s = 0;
  const double sf = pow (2, mpbpl);

  /* assert(mpbpl == mp_bits_per_limb); */
  assert(FRAC / mpbpl * mpbpl == FRAC);

  memset (maxerror, 0, sizeof (mp1));
  memset (xt, 0, sizeof (mp1));
  xt[(FRAC - N2) / mpbpl] = (mp_limb_t)1 << (FRAC - N2) % mpbpl;

  for (i = 0; i < (1 << N2); ++i)
    {
      int e2s, e3s, j;
      double de2;

      mpn_mul_1 (x, xt, SZ, i);
      exp2_mpn (ex, x);
      de2 = exp2 (i / (double) (1 << N2));
      for (j = SZ - 1; j >= 0; --j)
	{
	  e2[j] = (mp_limb_t) de2;
	  de2 = (de2 - e2[j]) * sf;
	}
      if (mpn_cmp (ex, e2, SZ) >= 0)
	mpn_sub_n (e3, ex, e2, SZ);
      else
	mpn_sub_n (e3, e2, ex, SZ);

      e2s = mpn_bitsize (e2, SZ);
      e3s = mpn_bitsize (e3, SZ);
      if (e3s >= 0 && e2s - e3s < 54)
	{
#if PRINT_ERRORS
	  printf ("%06x ", i * (0x100000 / (1 << N2)));
	  print_mpn_fp (ex, (FRAC / 4) + 1, 16);
	  putchar ('\n');
	  fputs ("       ",stdout);
	  print_mpn_fp (e2, (FRAC / 4) + 1, 16);
	  putchar ('\n');
	  printf (" %c     ",
		  e2s - e3s < 54 ? e2s - e3s == 53 ? 'e' : 'F' : 'P');
	  print_mpn_fp (e3, (FRAC / 4) + 1, 16);
	  putchar ('\n');
#endif
	  errors += (e2s - e3s == 53);
	  failures += (e2s - e3s < 53);
	}
      if (e3s >= maxerror_s
	  && mpn_cmp (e3, maxerror, SZ) > 0)
	{
	  memcpy (maxerror, e3, sizeof (mp1));
	  maxerror_s = e3s;
	}
    }

  /* Check exp_mpn against precomputed value of exp(1).  */
  memset (x, 0, sizeof (mp1));
  x[FRAC / mpbpl] = (mp_limb_t)1 << FRAC % mpbpl;
  exp_mpn (ex, x);
  read_mpn_hex (e2, exp1);
  if (mpn_cmp (ex, e2, SZ) >= 0)
    mpn_sub_n (e3, ex, e2, SZ);
  else
    mpn_sub_n (e3, e2, ex, SZ);

  printf ("%d failures; %d errors; error rate %0.2f%%\n", failures, errors,
	  errors * 100.0 / (double) (1 << N2));
  fputs ("maximum error:   ", stdout);
  print_mpn_fp (maxerror, (FRAC / 4) + 1, 16);
  putchar ('\n');
  fputs ("error in exp(1): ", stdout);
  print_mpn_fp (e3, (FRAC / 4) + 1, 16);
  putchar ('\n');

  return failures == 0 ? 0 : 1;
}
