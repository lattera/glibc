#ifndef _MATH_PRIVATE_H_
#error "Never use <math_ldbl.h> directly; include <math_private.h> instead."
#endif

#include <sysdeps/ieee754/ldbl-128/math_ldbl.h>

#define EXTRACT_IBM_EXTENDED_MANTISSA(hi64, lo64, expnt, ibm_ext_ldbl) \
  do									      \
    {									      \
      /* We have 105 bits of mantissa plus one implicit digit.  Since	      \
	 106 bits are representable without the rest using hexadecimal	      \
	 digits we use only the implicit digits for the number before	      \
	 the decimal point.  */						      \
      unsigned long long hi, lo;					      \
      int ediff;							      \
      union ibm_extended_long_double eldbl;				      \
      eldbl.d = ibm_ext_ldbl;						      \
      expnt = eldbl.ieee.exponent - IBM_EXTENDED_LONG_DOUBLE_BIAS;	      \
									      \
      lo = ((long long)eldbl.ieee.mantissa2 << 32) | eldbl.ieee.mantissa3;    \
      hi = ((long long)eldbl.ieee.mantissa0 << 32) | eldbl.ieee.mantissa1;    \
      /* If the lower double is not a denomal or zero then set the hidden     \
	 53rd bit.  */							      \
      if (eldbl.ieee.exponent2 > 0x001)					      \
	{								      \
	  lo |= (1ULL << 52);						      \
	  lo = lo << 7; /* pre-shift lo to match ieee854.  */		      \
          /* The lower double is normalized separately from the upper.  We    \
	     may need to adjust the lower manitissa to reflect this.  */      \
	  ediff = eldbl.ieee.exponent - eldbl.ieee.exponent2;		      \
	  if (ediff > 53)						      \
	    lo = lo >> (ediff-53);					      \
	}								      \
      hi |= (1ULL << 52);						      \
  									      \
      if ((eldbl.ieee.negative != eldbl.ieee.negative2)			      \
	  && ((eldbl.ieee.exponent2 != 0) && (lo != 0LL)))		      \
	{								      \
	  hi--;								      \
	  lo = (1ULL << 60) - lo;					      \
	  if (hi < (1ULL << 52))					      \
	    {								      \
	      /* we have a borrow from the hidden bit, so shift left 1.  */   \
	      hi = (hi << 1) | (lo >> 59);				      \
	      lo = 0xfffffffffffffffLL & (lo << 1);			      \
	      expnt--;							      \
	    }								      \
	}								      \
      lo64 = (hi << 60) | lo;						      \
      hi64 = hi >> 4;							      \
    }									      \
  while (0)

#define INSERT_IBM_EXTENDED_MANTISSA(ibm_ext_ldbl, sign, expnt, hi64, lo64) \
  do									      \
    {									      \
      union ibm_extended_long_double u;					      \
      unsigned long hidden2, lzcount;					      \
      unsigned long long hi, lo;					      \
									      \
      u.ieee.negative = sign;						      \
      u.ieee.negative2 = sign;						      \
      u.ieee.exponent = expnt + IBM_EXTENDED_LONG_DOUBLE_BIAS;		      \
      u.ieee.exponent2 = expnt-53 + IBM_EXTENDED_LONG_DOUBLE_BIAS;	      \
      /* Expect 113 bits (112 bits + hidden) right justified in two longs.    \
	 The low order 53 bits (52 + hidden) go into the lower double */      \
      lo = (lo64 >> 7)& ((1ULL << 53) - 1);				      \
      hidden2 = (lo64 >> 59) &  1ULL;					      \
      /* The high order 53 bits (52 + hidden) go into the upper double */     \
      hi = (lo64 >> 60) & ((1ULL << 11) - 1);				      \
      hi |= (hi64 << 4);						      \
  									      \
      if (lo != 0LL)							      \
	{								      \
	  /* hidden2 bit of low double controls rounding of the high double.  \
	     If hidden2 is '1' then round up hi and adjust lo (2nd mantissa)  \
	     plus change the sign of the low double to compensate.  */	      \
	  if (hidden2)							      \
	    {								      \
	      hi++;    							      \
	      u.ieee.negative2 = !sign;					      \
	      lo = (1ULL << 53) - lo;					      \
	    }								      \
	  /* The hidden bit of the lo mantissa is zero so we need to	      \
	     normalize the it for the low double.  Shift it left until the    \
	     hidden bit is '1' then adjust the 2nd exponent accordingly.  */  \
									      \
	  if (sizeof (lo) == sizeof (long))				      \
	    lzcount = __builtin_clzl (lo);				      \
	  else if ((lo >> 32) != 0)					      \
	    lzcount = __builtin_clzl ((long) (lo >> 32));		      \
	  else								      \
	    lzcount = __builtin_clzl ((long) lo) + 32;			      \
	  lzcount = lzcount - 11;					      \
	  if (lzcount > 0)						      \
	    {								      \
	      int expnt2 = u.ieee.exponent2 - lzcount;			      \
	      if (expnt2 >= 1)						      \
		{							      \
		  /* Not denormal.  Normalize and set low exponent.  */	      \
		  lo = lo << lzcount; 					      \
		  u.ieee.exponent2 = expnt2;				      \
		}							      \
	      else							      \
		{							      \
		  /* Is denormal.  */					      \
		  lo = lo << (lzcount + expnt2);			      \
		  u.ieee.exponent2 = 0;					      \
		}							      \
	    }								      \
	}								      \
      else 								      \
	{								      \
	  u.ieee.negative2 = 0;						      \
	  u.ieee.exponent2 = 0;						      \
	}								      \
  									      \
      u.ieee.mantissa3 = lo & ((1ULL << 32) - 1);			      \
      u.ieee.mantissa2 = (lo >> 32) & ((1ULL << 20) - 1);		      \
      u.ieee.mantissa1 = hi & ((1ULL << 32) - 1);			      \
      u.ieee.mantissa0 = (hi >> 32) & ((1ULL << 20) - 1);		      \
      ibm_ext_ldbl = u.d;						      \
    }									      \
  while (0)
