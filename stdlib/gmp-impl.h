/* Include file for internal GNU MP types and definitions.

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

#if ! defined (alloca)
#if defined (__GNUC__) || defined (__sparc__) || defined (sparc)
#define alloca __builtin_alloca
#endif
#endif

#ifndef NULL
#define NULL 0L
#endif

#if ! defined (__GNUC__)
#define inline			/* Empty */
void *alloca();
#endif

#define ABS(x) (x >= 0 ? x : -x)
#define MIN(l,o) ((l) < (o) ? (l) : (o))
#define MAX(h,i) ((h) > (i) ? (h) : (i))

#include "gmp-mparam.h"
/* #include "longlong.h" */

#ifdef __STDC__
void *malloc (size_t);
void *realloc (void *, size_t);
void free (void *);

extern void *	(*_mp_allocate_func) (size_t);
extern void *	(*_mp_reallocate_func) (void *, size_t, size_t);
extern void	(*_mp_free_func) (void *, size_t);

void *_mp_default_allocate (size_t);
void *_mp_default_reallocate (void *, size_t, size_t);
void _mp_default_free (void *, size_t);

#else

#define const			/* Empty */
#define signed			/* Empty */

void *malloc ();
void *realloc ();
void free ();

extern void *	(*_mp_allocate_func) ();
extern void *	(*_mp_reallocate_func) ();
extern void	(*_mp_free_func) ();

void *_mp_default_allocate ();
void *_mp_default_reallocate ();
void _mp_default_free ();
#endif

/* Copy NLIMBS *limbs* from SRC to DST.  */
#define MPN_COPY_INCR(DST, SRC, NLIMBS) \
  do {									\
    mp_size_t __i;							\
    for (__i = 0; __i < (NLIMBS); __i++)				\
      (DST)[__i] = (SRC)[__i];						\
  } while (0)
#define MPN_COPY_DECR(DST, SRC, NLIMBS) \
  do {									\
    mp_size_t __i;							\
    for (__i = (NLIMBS) - 1; __i >= 0; __i--)				\
      (DST)[__i] = (SRC)[__i];						\
  } while (0)
#define MPN_COPY MPN_COPY_INCR

/* Zero NLIMBS *limbs* AT DST.  */
#define MPN_ZERO(DST, NLIMBS) \
  do {									\
    mp_size_t __i;							\
    for (__i = 0; __i < (NLIMBS); __i++)				\
      (DST)[__i] = 0;							\
  } while (0)

#define MPN_NORMALIZE(DST, NLIMBS) \
  do {									\
    while (NLIMBS > 0)							\
      {									\
	if ((DST)[(NLIMBS) - 1] != 0)					\
	  break;							\
	NLIMBS--;							\
      }									\
  } while (0)
#define MPN_NORMALIZE_NOT_ZERO(DST, NLIMBS) \
  do {									\
    while (1)								\
      {									\
	if ((DST)[(NLIMBS) - 1] != 0)					\
	  break;							\
	NLIMBS--;							\
      }									\
  } while (0)

/*  Swap (mp_ptr, mp_size_t) (U, UL) with (V, VL)  */
#define MPN_SWAP(u, l, v, m) \
  do {									\
    { mp_ptr _; _ = (u), (u) = (v), (v) = _;}				\
    { mp_size_t _; _ = (l), (l) = (m), (m) = _;}			\
  } while (0)

/*  Return true iff the limb X has less bits than the limb Y.  */
#define MPN_LESS_BITS_LIMB(x,y) ((x) < (y) && (x) < ((x) ^ (y)))

/*  Return true iff (mp_ptr, mp_size_t) (U, UL) has less bits than (V, VL).  */
#define MPN_LESS_BITS(u, l, v, m) \
  ((l) < (m)								\
   || ((l) == (m) && (l) != 0 && MPN_LESS_BITS_LIMB ((u)[(l - 1)], (v)[(l) - 1])))

/*  Return true iff (mp_ptr, mp_size_t) (U, UL) has more bits than (V, VL).  */
#define MPN_MORE_BITS(u, l, v, m) MPN_LESS_BITS (v, m, u, l)

/*  Perform twos complement on (mp_ptr, mp_size_t) (U, UL), 
    putting result at (v, VL).  Precondition: U[0] != 0.  */
#define MPN_COMPL_INCR(u, v, l)	\
  do {									\
    mp_size_t _ = 0;							\
    (u)[0] = -(v)[_];							\
    while (_++ < (l)) 							\
      (u)[_] = ~(v)[_];							\
  } while (0)
#define MPN_COMPL MPN_COMPL_INCR

/* Initialize the MP_INT X with space for NLIMBS limbs.
   X should be a temporary variable, and it will be automatically
   cleared out when the running function returns.
   We use __x here to make it possible to accept both mpz_ptr and mpz_t
   arguments.  */
#define MPZ_TMP_INIT(X, NLIMBS) \
  do {									\
    mpz_ptr __x = (X);							\
    __x->alloc = (NLIMBS);						\
    __x->d = (mp_ptr) alloca ((NLIMBS) * BYTES_PER_MP_LIMB);		\
  } while (0)

#define MPN_MUL_N_RECURSE(prodp, up, vp, size, tspace) \
  do {									\
    if ((size) < KARATSUBA_THRESHOLD)					\
      ____mpn_mul_n_basecase (prodp, up, vp, size);			\
    else								\
      ____mpn_mul_n (prodp, up, vp, size, tspace);			\
  } while (0);
#define MPN_SQR_N_RECURSE(prodp, up, size, tspace) \
  do {									\
    if ((size) < KARATSUBA_THRESHOLD)					\
      ____mpn_sqr_n_basecase (prodp, up, size);				\
    else								\
      ____mpn_sqr_n (prodp, up, size, tspace);				\
  } while (0);

/* Structure for conversion between internal binary format and
   strings in base 2..36.  */
struct bases
{
  /* Number of digits in the conversion base that always fits in
     an mp_limb.  For example, for base 10 this is 10, since
     2**32 = 4294967296 has ten digits.  */
  int chars_per_limb;

  /* log(2)/log(conversion_base) */
  float chars_per_bit_exactly;

  /* big_base is conversion_base**chars_per_limb, i.e. the biggest
     number that fits a word, built by factors of conversion_base.
     Exception: For 2, 4, 8, etc, big_base is log2(base), i.e. the
     number of bits used to represent each digit in the base.  */
  mp_limb big_base;

  /* big_base_inverted is a BITS_PER_MP_LIMB bit approximation to
     1/big_base, represented as a fixed-point number.  Instead of
     dividing by big_base an application can choose to multiply
     by big_base_inverted.  */
  mp_limb big_base_inverted;
};

extern const struct bases __mp_bases[];
extern mp_size_t __gmp_default_fp_limb_precision;

/* Divide the two-limb number in (NH,,NL) by D, with DI being a 32 bit
   approximation to (2**(2*BITS_PER_MP_LIMB))/D - (2**BITS_PER_MP_LIMB).
   Put the quotient in Q and the remainder in R.  */
#define udiv_qrnnd_preinv(q, r, nh, nl, d, di) \
  do {									\
    mp_limb _q, _ql, _r;						\
    mp_limb _xh, _xl;							\
    umul_ppmm (_q, _ql, (nh), (di));					\
    _q += (nh);			/* DI is 2**BITS_PER_MP_LIMB too small */\
    umul_ppmm (_xh, _xl, _q, (d));					\
    sub_ddmmss (_xh, _r, (nh), (nl), _xh, _xl);				\
    if (_xh != 0)							\
      {									\
	sub_ddmmss (_xh, _r, _xh, _r, 0, (d));				\
	_q += 1;							\
	if (_xh != 0)							\
	  {								\
	    sub_ddmmss (_xh, _r, _xh, _r, 0, (d));			\
	    _q += 1;							\
	  }								\
      }									\
    if (_r >= (d))							\
      {									\
	_r -= (d);							\
	_q += 1;							\
      }									\
    (r) = _r;								\
    (q) = _q;								\
  } while (0)
#define udiv_qrnnd_preinv2gen(q, r, nh, nl, d, di, dnorm, lgup) \
  do {									\
    mp_limb n2, n10, n1, nadj, q1;					\
    mp_limb _xh, _xl;							\
    n2 = ((nh) << (BITS_PER_MP_LIMB - (lgup))) + ((nl) >> 1 >> (l - 1));\
    n10 = (nl) << (BITS_PER_MP_LIMB - (lgup));				\
    n1 = ((mp_limb_signed) n10 >> (BITS_PER_MP_LIMB - 1));		\
    nadj = n10 + (n1 & (dnorm));					\
    umul_ppmm (_xh, _xl, di, n2 - n1);					\
    add_ssaaaa (_xh, _xl, _xh, _xl, 0, nadj);				\
    q1 = ~(n2 + _xh);							\
    umul_ppmm (_xh, _xl, q1, d);					\
    add_ssaaaa (_xh, _xl, _xh, _xl, nh, nl);				\
    _xh -= (d);								\
    (r) = _xl + ((d) & _xh);						\
    (q) = _xh - q1;							\
  } while (0)
#define udiv_qrnnd_preinv2norm(q, r, nh, nl, d, di) \
  do {									\
    mp_limb n2, n10, n1, nadj, q1;					\
    mp_limb _xh, _xl;							\
    n2 = (nh);								\
    n10 = (nl);								\
    n1 = ((mp_limb_signed) n10 >> (BITS_PER_MP_LIMB - 1));		\
    nadj = n10 + (n1 & (d));						\
    umul_ppmm (_xh, _xl, di, n2 - n1);					\
    add_ssaaaa (_xh, _xl, _xh, _xl, 0, nadj);				\
    q1 = ~(n2 + _xh);							\
    umul_ppmm (_xh, _xl, q1, d);					\
    add_ssaaaa (_xh, _xl, _xh, _xl, nh, nl);				\
    _xh -= (d);								\
    (r) = _xl + ((d) & _xh);						\
    (q) = _xh - q1;							\
  } while (0)

#if defined (__GNUC__)
/* Define stuff for longlong.h asm macros.  */
#if __GNUC_NEW_ATTR_MODE_SYNTAX
typedef unsigned int UQItype	__attribute__ ((mode ("QI")));
typedef 	 int SItype	__attribute__ ((mode ("SI")));
typedef unsigned int USItype	__attribute__ ((mode ("SI")));
typedef		 int DItype	__attribute__ ((mode ("DI")));
typedef unsigned int UDItype	__attribute__ ((mode ("DI")));
#else
typedef unsigned int UQItype	__attribute__ ((mode (QI)));
typedef 	 int SItype	__attribute__ ((mode (SI)));
typedef unsigned int USItype	__attribute__ ((mode (SI)));
typedef		 int DItype	__attribute__ ((mode (DI)));
typedef unsigned int UDItype	__attribute__ ((mode (DI)));
#endif
#endif

typedef mp_limb UWtype;
typedef unsigned int UHWtype;
#define W_TYPE_SIZE BITS_PER_MP_LIMB
