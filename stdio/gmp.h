/* gmp.h -- Definitions for GNU multiple precision functions.

Copyright (C) 1991, 1993, 1994 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
License for more details.

You should have received a copy of the GNU Library General Public License
along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

#ifndef __GMP_H__

#ifndef __GNU_MP__
#define __need_size_t
#include <stddef.h>

#ifdef __STDC__
#define __gmp_const const
#else
#define __gmp_const
#endif

#ifdef __GNUC__
#define __gmp_inline inline
#else
#define __gmp_inline
#endif

#ifdef _SHORT_LIMB
typedef unsigned int		mp_limb;
typedef int			mp_limb_signed;
#else
typedef unsigned long int	mp_limb;
typedef long int		mp_limb_signed;
#endif

typedef mp_limb *		mp_ptr;
typedef __gmp_const mp_limb *	mp_srcptr;
typedef int			mp_size_t;
typedef long int		mp_exp_t;

#ifndef __MP_SMALL__
typedef struct
{
  long int alloc;		/* Number of *limbs* allocated and pointed
				   to by the D field.  */
  long int size;		/* abs(SIZE) is the number of limbs
				   the last field points to.  If SIZE
				   is negative this is a negative
				   number.  */
  mp_limb *d;			/* Pointer to the limbs.  */
} __mpz_struct;
#else
typedef struct
{
  short int alloc;		/* Number of *limbs* allocated and pointed
				   to by the D field.  */
  short int size;		/* abs(SIZE) is the number of limbs
				   the last field points to.  If SIZE
				   is negative this is a negative
				   number.  */
  mp_limb *d;			/* Pointer to the limbs.  */
} __mpz_struct;
#endif
#endif /* __GNU_MP__ */

/* User-visible types.  */
typedef __mpz_struct MP_INT;
typedef __mpz_struct mpz_t[1];

/* Structure for rational numbers.  Zero is represented as 0/any, i.e.
   the denominator is ignored.  Negative numbers have the sign in
   the numerator.  */
typedef struct
{
  __mpz_struct num;
  __mpz_struct den;
#if 0
  long int num_alloc;		/* Number of limbs allocated
				   for the numerator.  */
  long int num_size;		/* The absolute value of this field is the
				   length of the numerator; the sign is the
				   sign of the entire rational number.  */
  mp_ptr num;			/* Pointer to the numerator limbs.  */
  long int den_alloc;		/* Number of limbs allocated
				   for the denominator.  */
  long int den_size;		/* Length of the denominator.  (This field
				   should always be positive.) */
  mp_ptr den;			/* Pointer to the denominator limbs.  */
#endif
} __mpq_struct;

typedef __mpq_struct MP_RAT;
typedef __mpq_struct mpq_t[1];

typedef struct
{
  mp_size_t alloc;		/* Number of *limbs* allocated and pointed
				   to by the D field.  */
  mp_size_t prec;		/* Max precision, in number of `mp_limb's.
				   Set by mpf_init and modified by
				   mpf_set_prec.  */
  mp_size_t size;		/* abs(SIZE) is the number of limbs
				   the last field points to.  If SIZE
				   is negative this is a negative
				   number.  */
  mp_exp_t exp;			/* Exponent, in the base of `mp_limb'.  */
  mp_limb *d;			/* Pointer to the limbs.  */
} __mpf_struct;

/* typedef __mpf_struct MP_FLOAT; */
typedef __mpf_struct mpf_t[1];

/* Types for function declarations in gmp files.  */
/* ??? Should not pollute user name space ??? */
typedef __gmp_const __mpz_struct *mpz_srcptr;
typedef __mpz_struct *mpz_ptr;
typedef __gmp_const __mpf_struct *mpf_srcptr;
typedef __mpf_struct *mpf_ptr;
typedef __gmp_const __mpq_struct *mpq_srcptr;
typedef __mpq_struct *mpq_ptr;

#ifdef __STDC__
#define _PROTO(x) x
#else
#define _PROTO(x) ()
#endif

void mp_set_memory_functions _PROTO((void *(*) (size_t),
				     void *(*) (void *, size_t, size_t),
				     void (*) (void *, size_t)));

/**************** Integer (i.e. Z) routines.  ****************/

void *_mpz_realloc _PROTO ((mpz_ptr, mp_size_t));

void mpz_abs _PROTO ((mpz_ptr, mpz_srcptr));
void mpz_add _PROTO ((mpz_ptr, mpz_srcptr, mpz_srcptr));
void mpz_add_ui _PROTO ((mpz_ptr, mpz_srcptr, unsigned long int));
void mpz_and _PROTO ((mpz_ptr, mpz_srcptr, mpz_srcptr));
void mpz_clear _PROTO ((mpz_ptr));
void mpz_clrbit _PROTO ((mpz_ptr, unsigned long int));
int mpz_cmp _PROTO ((mpz_srcptr, mpz_srcptr));
int mpz_cmp_si _PROTO ((mpz_srcptr, signed long int));
int mpz_cmp_ui _PROTO ((mpz_srcptr, unsigned long int));
void mpz_com _PROTO ((mpz_ptr, mpz_srcptr));
void mpz_div_2exp _PROTO ((mpz_ptr, mpz_srcptr, unsigned long int));
void mpz_fac_ui _PROTO ((mpz_ptr, unsigned long int));
void mpz_gcd _PROTO ((mpz_ptr, mpz_srcptr, mpz_srcptr));
unsigned long int mpz_gcd_ui _PROTO ((mpz_ptr, mpz_srcptr, unsigned long int));
void mpz_gcdext _PROTO ((mpz_ptr, mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr));
/* signed */ long int mpz_get_si _PROTO ((mpz_srcptr));
char *mpz_get_str _PROTO ((char *, int, mpz_srcptr));
unsigned long int mpz_get_ui _PROTO ((mpz_srcptr));
mp_limb mpz_getlimbn _PROTO ((mpz_srcptr, mp_size_t));
mp_size_t mpz_hamdist _PROTO ((mpz_srcptr, mpz_srcptr));
void mpz_init _PROTO ((mpz_ptr));
#ifdef FILE
void mpz_inp_raw _PROTO ((mpz_ptr, FILE *));
int mpz_inp_str _PROTO ((mpz_ptr, FILE *, int));
#endif
void mpz_ior _PROTO ((mpz_ptr, mpz_srcptr, mpz_srcptr));
void mpz_init_set _PROTO ((mpz_ptr, mpz_srcptr));
void mpz_init_set_si _PROTO ((mpz_ptr, signed long int));
int mpz_init_set_str _PROTO ((mpz_ptr, const char *, int));
void mpz_init_set_ui _PROTO ((mpz_ptr, unsigned long int));
void mpz_lcm _PROTO ((mpz_ptr, mpz_srcptr, mpz_srcptr));
void mpz_mod_2exp _PROTO ((mpz_ptr, mpz_srcptr, unsigned long int));
void mpz_mul _PROTO ((mpz_ptr, mpz_srcptr, mpz_srcptr));
void mpz_mul_2exp _PROTO ((mpz_ptr, mpz_srcptr, unsigned long int));
void mpz_mul_ui _PROTO ((mpz_ptr, mpz_srcptr, unsigned long int));
void mpz_neg _PROTO ((mpz_ptr, mpz_srcptr));
#ifdef FILE
void mpz_out_raw _PROTO ((FILE *, mpz_srcptr));
void mpz_out_str _PROTO ((FILE *, int, mpz_srcptr));
#endif
int mpz_perfect_square_p _PROTO ((mpz_srcptr));
mp_size_t mpz_popcount _PROTO ((mpz_srcptr));
void mpz_pow_ui _PROTO ((mpz_ptr, mpz_srcptr, unsigned long int));
void mpz_powm _PROTO ((mpz_ptr, mpz_srcptr, mpz_srcptr, mpz_srcptr));
void mpz_powm_ui _PROTO ((mpz_ptr, mpz_srcptr, unsigned long int, mpz_srcptr));
int mpz_probab_prime_p _PROTO ((mpz_srcptr, int));
void mpz_random _PROTO ((mpz_ptr, mp_size_t));
void mpz_random2 _PROTO ((mpz_ptr, mp_size_t));
void mpz_set _PROTO ((mpz_ptr, mpz_srcptr));
void mpz_set_si _PROTO ((mpz_ptr, signed long int));
int mpz_set_str _PROTO ((mpz_ptr, const char *, int));
void mpz_set_ui _PROTO ((mpz_ptr, unsigned long int));
size_t mpz_size _PROTO ((mpz_srcptr));
size_t mpz_sizeinbase _PROTO ((mpz_srcptr, int));
void mpz_sqrt _PROTO ((mpz_ptr, mpz_srcptr));
void mpz_sqrtrem _PROTO ((mpz_ptr, mpz_ptr, mpz_srcptr));
void mpz_sub _PROTO ((mpz_ptr, mpz_srcptr, mpz_srcptr));
void mpz_sub_ui _PROTO ((mpz_ptr, mpz_srcptr, unsigned long int));
void mpz_ui_pow_ui _PROTO ((mpz_ptr, unsigned long int, unsigned long int));

void mpz_fdiv_q _PROTO((mpz_ptr, mpz_srcptr, mpz_srcptr));
unsigned long int mpz_fdiv_q_ui _PROTO((mpz_ptr, mpz_srcptr, unsigned long int));
void mpz_fdiv_qr _PROTO((mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr));
unsigned long int mpz_fdiv_qr_ui _PROTO((mpz_ptr, mpz_ptr, mpz_srcptr, unsigned long int));
void mpz_fdiv_r _PROTO((mpz_ptr, mpz_srcptr, mpz_srcptr));
unsigned long int mpz_fdiv_r_ui _PROTO((mpz_ptr, mpz_srcptr, unsigned long int));
unsigned long int mpz_fdiv_ui _PROTO((mpz_srcptr, unsigned long int));
void mpz_tdiv_q _PROTO((mpz_ptr, mpz_srcptr, mpz_srcptr));
void mpz_tdiv_q_ui _PROTO((mpz_ptr, mpz_srcptr, unsigned long int));
void mpz_tdiv_qr _PROTO((mpz_ptr, mpz_ptr, mpz_srcptr, mpz_srcptr));
void mpz_tdiv_qr_ui _PROTO((mpz_ptr, mpz_ptr, mpz_srcptr, unsigned long int));
void mpz_tdiv_r _PROTO((mpz_ptr, mpz_srcptr, mpz_srcptr));
void mpz_tdiv_r_ui _PROTO((mpz_ptr, mpz_srcptr, unsigned long int));

/**************** Rational (i.e. Q) routines.  ****************/

void mpq_init _PROTO ((mpq_ptr));
void mpq_clear _PROTO ((mpq_ptr));
void mpq_set _PROTO ((mpq_ptr, mpq_srcptr));
void mpq_set_ui _PROTO ((mpq_ptr, unsigned long int, unsigned long int));
void mpq_set_si _PROTO ((mpq_ptr, signed long int, unsigned long int));
void mpq_add _PROTO ((mpq_ptr, mpq_srcptr, mpq_srcptr));
void mpq_sub _PROTO ((mpq_ptr, mpq_srcptr, mpq_srcptr));
void mpq_mul _PROTO ((mpq_ptr, mpq_srcptr, mpq_srcptr));
void mpq_div _PROTO ((mpq_ptr, mpq_srcptr, mpq_srcptr));
void mpq_neg _PROTO ((mpq_ptr, mpq_srcptr));
int mpq_cmp _PROTO ((mpq_srcptr, mpq_srcptr));
void mpq_inv _PROTO ((mpq_ptr, mpq_srcptr));
void mpq_set_num _PROTO ((mpq_ptr, mpz_srcptr));
void mpq_set_den _PROTO ((mpq_ptr, mpz_srcptr));
void mpq_get_num _PROTO ((mpz_ptr, mpq_srcptr));
void mpq_get_den _PROTO ((mpz_ptr, mpq_srcptr));

/**************** Float (i.e. F) routines.  ****************/

void mpf_abs _PROTO ((mpf_ptr, mpf_srcptr));
void mpf_add _PROTO ((mpf_ptr, mpf_srcptr, mpf_srcptr));
void mpf_add_ui _PROTO ((mpf_ptr, mpf_srcptr, unsigned long int));
void mpf_clear _PROTO ((mpf_ptr));
int mpf_cmp _PROTO ((mpf_srcptr, mpf_srcptr));
int mpf_cmp_si _PROTO ((mpf_srcptr, long int));
int mpf_cmp_ui _PROTO ((mpf_srcptr, unsigned long int));
void mpf_div _PROTO ((mpf_ptr, mpf_srcptr, mpf_srcptr));
void mpf_div_2exp _PROTO ((mpf_ptr, mpf_srcptr, unsigned long int));
void mpf_div_ui _PROTO ((mpf_ptr, mpf_srcptr, unsigned long int));
void mpf_dump _PROTO ((mpf_srcptr));
char *mpf_get_str _PROTO ((char *, mp_exp_t *, int, size_t, mpf_srcptr));
void mpf_init _PROTO ((mpf_ptr));
void mpf_init2 _PROTO ((mpf_ptr, mp_size_t));
#ifdef FILE
void mpf_inp_str _PROTO ((mpf_ptr, FILE *, int));
#endif
void mpf_init_set _PROTO ((mpf_ptr, mpf_srcptr));
void mpf_init_set_d _PROTO ((mpf_ptr, double));
void mpf_init_set_si _PROTO ((mpf_ptr, long int));
int mpf_init_set_str _PROTO ((mpf_ptr, char *, int));
void mpf_init_set_ui _PROTO ((mpf_ptr, unsigned long int));
void mpf_mul _PROTO ((mpf_ptr, mpf_srcptr, mpf_srcptr));
void mpf_mul_2exp _PROTO ((mpf_ptr, mpf_srcptr, unsigned long int));
void mpf_mul_ui _PROTO ((mpf_ptr, mpf_srcptr, unsigned long int));
void mpf_neg _PROTO ((mpf_ptr, mpf_srcptr));
#ifdef FILE
void mpf_out_str _PROTO ((mpf_ptr, int, size_t, FILE *));
#endif
void mpf_set _PROTO ((mpf_ptr, mpf_srcptr));
void mpf_set_d _PROTO ((mpf_ptr, double));
mp_size_t mpf_set_default_prec _PROTO ((mp_size_t));
void mpf_set_si _PROTO ((mpf_ptr, long int));
int mpf_set_str _PROTO ((mpf_ptr, const char *, int));
void mpf_set_ui _PROTO ((mpf_ptr, unsigned long int));
size_t mpf_size _PROTO ((mpf_srcptr));
void mpf_sqrt _PROTO ((mpf_ptr, mpf_srcptr));
void mpf_sqrt_ui _PROTO ((mpf_ptr, unsigned long int));
void mpf_sub _PROTO ((mpf_ptr, mpf_srcptr, mpf_srcptr));
void mpf_sub_ui _PROTO ((mpf_ptr, mpf_srcptr, unsigned long int));
void mpf_ui_div _PROTO ((mpf_ptr, unsigned long int, mpf_srcptr));

/************ Low level positive-integer (i.e. N) routines.  ************/

/* This is ugly, but we need to make usr calls reach the prefixed function.  */
#define mpn_add_n	__mpn_add_n
#define mpn_sub_n	__mpn_sub_n
#define mpn_mul_1	__mpn_mul_1
#define mpn_addmul_1	__mpn_addmul_1
#define mpn_submul_1	__mpn_submul_1
#define mpn_lshift	__mpn_lshift
#define mpn_rshift	__mpn_rshift
#define mpn_sub		__mpn_sub
#define mpn_add		__mpn_add
#define mpn_normal_size	__mpn_normal_size
#define mpn_cmp		__mpn_cmp
#define mpn_add_1	__mpn_add_1
#define mpn_sub_1	__mpn_sub_1
#define mpn_mul_n	__mpn_mul_n
#define mpn_mul		__mpn_mul
#define mpn_divmod	__mpn_divmod
#define mpn_divmod_1	__mpn_divmod_1
#define mpn_mod_1	__mpn_mod_1
#define mpn_sqrt	__mpn_sqrt
#define mpn_next_bit_set __mpn_next_bit_set
#define mpn_popcount	__mpn_popcount
#define mpn_hamdist	__mpn_hamdist
#define mpn_random2	__mpn_random2
#define mpn_set_str	__mpn_set_str
#define mpn_get_str	__mpn_get_str
#define mpn_gcd_1	__mpn_gcd_1

mp_limb __mpn_add_n _PROTO ((mp_ptr, mp_srcptr, mp_srcptr, mp_size_t));
mp_limb __mpn_sub_n _PROTO ((mp_ptr, mp_srcptr, mp_srcptr, mp_size_t));
mp_limb __mpn_mul _PROTO ((mp_ptr, mp_srcptr, mp_size_t, mp_srcptr, mp_size_t));
void __mpn_mul_n _PROTO ((mp_ptr, mp_srcptr, mp_srcptr, mp_size_t));
mp_limb __mpn_mul_1 _PROTO ((mp_ptr, mp_srcptr, mp_size_t, mp_limb));
mp_limb __mpn_addmul_1 _PROTO ((mp_ptr, mp_srcptr, mp_size_t, mp_limb));
mp_limb __mpn_submul_1 _PROTO ((mp_ptr, mp_srcptr, mp_size_t, mp_limb));
mp_limb __mpn_divmod _PROTO ((mp_ptr, mp_ptr, mp_size_t, mp_srcptr, mp_size_t));
mp_limb __mpn_divmod_1 _PROTO ((mp_ptr, mp_srcptr, mp_size_t, mp_limb));
mp_limb __mpn_mod_1 _PROTO ((mp_srcptr, mp_size_t, mp_limb));
mp_limb __mpn_lshift _PROTO ((mp_ptr, mp_srcptr, mp_size_t, unsigned int));
mp_limb __mpn_rshift _PROTO ((mp_ptr, mp_srcptr, mp_size_t, unsigned int));
mp_size_t __mpn_sqrt _PROTO ((mp_ptr, mp_ptr, mp_srcptr, mp_size_t));
int __mpn_cmp _PROTO ((mp_srcptr, mp_srcptr, mp_size_t));
mp_size_t __mpn_next_bit_set _PROTO ((mp_srcptr, mp_size_t));
mp_size_t __mpn_popcount _PROTO ((mp_srcptr, mp_size_t));
mp_size_t __mpn_hamdist _PROTO ((mp_srcptr, mp_srcptr, mp_size_t));
void __mpn_random2 _PROTO ((mp_ptr, mp_size_t));
mp_size_t __mpn_set_str _PROTO ((mp_ptr, const unsigned char *, size_t, int));
size_t __mpn_get_str _PROTO ((unsigned char *, int, mp_ptr, mp_size_t));
mp_limb __mpn_gcd_1 _PROTO ((mp_srcptr, mp_size_t, mp_limb));


static __gmp_inline mp_limb
#if __STDC__
__mpn_add_1 (register mp_ptr res_ptr,
	     register mp_srcptr s1_ptr,
	     register mp_size_t s1_size,
	     register mp_limb s2_limb)
#else
__mpn_add_1 (res_ptr, s1_ptr, s1_size, s2_limb)
     register mp_ptr res_ptr;
     register mp_srcptr s1_ptr;
     register mp_size_t s1_size;
     register mp_limb s2_limb;
#endif
{
  register mp_limb x;

  x = *s1_ptr++;
  s2_limb = x + s2_limb;
  *res_ptr++ = s2_limb;
  if (s2_limb < x)
    {
      while (--s1_size != 0)
	{
	  x = *s1_ptr++ + 1;
	  *res_ptr++ = x;
	  if (x != 0)
	    goto fin;
	}

      return 1;
    }

 fin:
  if (res_ptr != s1_ptr)
    {
      mp_size_t i;
      for (i = 0; i < s1_size - 1; i++)
	res_ptr[i] = s1_ptr[i];
    }
  return 0;
}

static __gmp_inline mp_limb
#if __STDC__
__mpn_add (register mp_ptr res_ptr,
	   register mp_srcptr s1_ptr,
	   register mp_size_t s1_size,
	   register mp_srcptr s2_ptr,
	   register mp_size_t s2_size)
#else
__mpn_add (res_ptr, s1_ptr, s1_size, s2_ptr, s2_size)
     register mp_ptr res_ptr;
     register mp_srcptr s1_ptr;
     register mp_size_t s1_size;
     register mp_srcptr s2_ptr;
     register mp_size_t s2_size;
#endif
{
  mp_limb cy_limb = 0;

  if (s2_size != 0)
    cy_limb = __mpn_add_n (res_ptr, s1_ptr, s2_ptr, s2_size);

  if (s1_size - s2_size != 0)
    cy_limb =  __mpn_add_1 (res_ptr + s2_size,
			    s1_ptr + s2_size,
			    s1_size - s2_size,
			    cy_limb);
  return cy_limb;
}

static __gmp_inline mp_limb
#if __STDC__
__mpn_sub_1 (register mp_ptr res_ptr,
	     register mp_srcptr s1_ptr,
	     register mp_size_t s1_size,
	     register mp_limb s2_limb)
#else
__mpn_sub_1 (res_ptr, s1_ptr, s1_size, s2_limb)
     register mp_ptr res_ptr;
     register mp_srcptr s1_ptr;
     register mp_size_t s1_size;
     register mp_limb s2_limb;
#endif
{
  register mp_limb x;

  x = *s1_ptr++;
  s2_limb = x - s2_limb;
  *res_ptr++ = s2_limb;
  if (s2_limb > x)
    {
      while (--s1_size != 0)
	{
	  x = *s1_ptr++;
	  *res_ptr++ = x - 1;
	  if (x != 0)
	    goto fin;
	}

      return 1;
    }

 fin:
  if (res_ptr != s1_ptr)
    {
      mp_size_t i;
      for (i = 0; i < s1_size - 1; i++)
	res_ptr[i] = s1_ptr[i];
    }
  return 0;
}

static __gmp_inline mp_limb
#if __STDC__
__mpn_sub (register mp_ptr res_ptr,
	   register mp_srcptr s1_ptr,
	   register mp_size_t s1_size,
	   register mp_srcptr s2_ptr,
	   register mp_size_t s2_size)
#else
__mpn_sub (res_ptr, s1_ptr, s1_size, s2_ptr, s2_size)
     register mp_ptr res_ptr;
     register mp_srcptr s1_ptr;
     register mp_size_t s1_size;
     register mp_srcptr s2_ptr;
     register mp_size_t s2_size;
#endif
{
  mp_limb cy_limb = 0;

  if (s2_size != 0)
    cy_limb = __mpn_sub_n (res_ptr, s1_ptr, s2_ptr, s2_size);

  if (s1_size - s2_size != 0)
    cy_limb =  __mpn_sub_1 (res_ptr + s2_size,
			    s1_ptr + s2_size,
			    s1_size - s2_size,
			    cy_limb);
  return cy_limb;
}

static __gmp_inline mp_size_t
#if __STDC__
__mpn_normal_size (mp_srcptr ptr, mp_size_t size)
#else
__mpn_normal_size (ptr, size)
     mp_srcptr ptr;
     mp_size_t size;
#endif
{
  while (size)
    {
      size--;
      if (ptr[size] != 0)
	return size + 1;
    }
  return 0;
}

/* Compatibility with GMP 1.  */

#define mpz_mdiv	mpz_fdiv_q
#define mpz_mdivmod	mpz_fdiv_qr
#define mpz_mmod	mpz_fdiv_r
#define mpz_mdiv_ui	mpz_fdiv_q_ui
#define mpz_mdivmod_ui(q,r,n,d) \
  ((r == 0) ? mpz_fdiv_q_ui (q,n,d) : mpz_fdiv_qr_ui (q,r,n,d))
#define mpz_mmod_ui(r,n,d) \
  ((r == 0) ? mpz_fdiv_ui (n,d) : mpz_fdiv_r_ui (r,n,d))
/* ??? Before release...
#define mpz_div_2exp	mpz_fdiv_q_2exp
#define mpz_mod_2exp	mpz_fdiv_r_2exp
*/

/* Useful synonyms, but not quite compatible with GMP 1.  */
#define mpz_div		mpz_fdiv_q
#define mpz_divmod	mpz_fdiv_qr
#define mpz_mod		mpz_fdiv_r
#define mpz_div_ui	mpz_fdiv_q_ui
#define mpz_divmod_ui	mpz_fdiv_qr_ui
#define mpz_mod_ui	mpz_fdiv_r_ui


#define __GNU_MP__ 2
#define __GNU_MP_VERSION 2
#define __GNU_MP_VERSION_MINOR -900 /* ??? */
#define __GMP_H__
#endif /* __GMP_H__ */
