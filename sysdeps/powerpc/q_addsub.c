/* Add or subtract two 128-bit floating point values.  C prototype.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <quad_float.h>

/* Add 'a' to 'b' and put the result in 'result', but treat a[0]=axx,
   b[0]=bxx.  bxx differs from b[0] only in the high bit, similarly axx.  */
/* Exceptions to raise:
   - Invalid (SNaN)
   - Invalid (Inf-Inf)
   - Overflow
   - Underflow
   - Inexact
   */

/* Handle cases where exponent of a or b is maximum.  */
static void
handle_max_exponent(unsigned result[4],
		    const unsigned a[4], const unsigned b[4],
		    const unsigned axx,  /* Treat as a[0].  */
		    const unsigned bxx,  /* Treat as b[0].  */
		    const unsigned ax,   /* axx >> 16 & 0x7fff.  */
		    const unsigned bx)   /* bxx >> 16 & 0x7fff.  */
{
  int ax_ismax, bx_ismax;
  unsigned a1,a2,a3, b1,b2,b3;
  int a_zeromant, b_zeromant;

  ax_ismax = ax == 0x7fff;
  bx_ismax = bx == 0x7fff;

  assert(ax_ismax || bx_ismax);

  a1 = a[1]; a2 = a[2]; a3 = a[3];
  b1 = b[1]; b2 = b[2]; b3 = b[3];
  
  a_zeromant = (axx & 0xffff  |  a1 | a2 | a3) == 0;
  b_zeromant = (bxx & 0xffff  |  b1 | b2 | b3) == 0;
  
  /* Deal with SNaNs.  */
  if (   ax_ismax && !a_zeromant && (axx & 0x8000) == 0
      || bx_ismax && !b_zeromant && (bxx & 0x8000) == 0)
    {
      set_fpscr_bit(FPSCR_VXSNAN);
      axx |= 0x8000; /* Demote the SNaN to a QNaN (whichever of */
      bxx |= 0x8000; /* a or b it was).  */
    }
  /* Deal with Inf-Inf.  */
  else if (a_zeromant && b_zeromant && (axx ^ bxx) == 0x80000000)
    {
      set_fpscr_bit(FPSCR_VXISI);
      bxx |= 0x8000; /* Return an appropriate QNaN.  */
    }
  
  /* Return the lexicographically larger of a or b, ignoring the sign
     bits.  */
  if ((axx & 0x7fffffff) > (bxx & 0x7fffffff)) goto return_a;
  else if ((axx & 0x7fffffff) < (bxx & 0x7fffffff)) goto return_b;
  else if (a1 > b1) goto return_a;
  else if (a1 < b1) goto return_b;
  else if (a2 > b2) goto return_a;
  else if (a2 < b2) goto return_b;
  else if (a3 > b3) goto return_a;  /* I've clearly been writing too */
  else if (a3 < b3) goto return_b;  /* much Fortran...  */
  
  /* If they are equal except for the sign bits, return 'b'.  */
  
return_b:
  result[0] = bxx; result[1] = b1; result[2] = b2; result[3] = b3;
  return;
    
return_a:
  result[0] = axx; result[1] = a1; result[2] = a2; result[3] = a3;
  return;
}

/* Renormalise and output a FP number.  */
static void
renormalise_value(unsigned result[4],
		  const unsigned axx,
		  unsigned ax,
		  unsigned r0,
		  unsigned r1,
		  unsigned r2,
		  unsigned r3)
{
  int rshift;
  if (r0 != 0 || cntlzw(a1) < 16 || 32 > ax-1)
    {
      rshift = cntlzw(r0)-15 + (-(cntlzw(r0) >> 5) & cntlzw(a1));
      assert(rshift < 32);
      if (rshift > ax-1)
	{
	  ax--;
	  rshift = ax;
	}

      result[0] = (axx & 0x80000000
		   | ax-rshift << 16
		   | r0 << rshift & 0xffff
		   | a1 >> 32-rshift & 0xffff);
      result[1] = a1 << rshift | a2 >> 32-rshift;
      result[2] = a2 << rshift | a3 >> 32-rshift;
      result[3] = a3 << rshift;
      return;
    }
  result[3] = 0;
  /* Special case for zero.  */
  if (a1 == 0 && a2 == 0 && a3 == 0)
    {
      result[0] = axx & 0x80000000;
      result[1] = result[2] = 0;
      return;
    }
  while (a1 != 0 && cntlzw(a2) >= 16 && 64 <= ax-1)
    {
      ax -= 32;
      a1 = a2; a2 = a3; a3 = 0;
    }
  rshift = cntlzw(a1)-15 + (-(cntlzw(a1) >> 5) & cntlzw(a2));
  assert(rshift < 32);
  if (rshift > ax-1-32)
    {
      ax--;
      rshift = ax-32;
    }
  
  result[0] = (axx & 0x80000000
	       | ax-rshift-32 << 16
	       | a1 << rshift & 0xffff
	       | a2 >> 32-rshift & 0xffff);
  result[1] = a2 << rshift | a3 >> 32-rshift;
  result[2] = a3 << rshift;
  return;
}

/* Handle the case where one or both numbers are denormalised or zero. 
   This case almost never happens, so we don't slow the main code
   with it.  */
static void
handle_min_exponent(unsigned result[4],
		    const unsigned a[4], const unsigned b[4],
		    const unsigned axx,  /* Treat as a[0].  */
		    const unsigned bxx,  /* Treat as b[0].  */
		    const unsigned ax,   /* axx >> 16 & 0x7fff.  */
		    const unsigned bx)   /* bxx >> 16 & 0x7fff.  */
{
  int ax_denorm, bx_denorm;
  unsigned a1,a2,a3, b1,b2,b3;
  int a_zeromant, b_zeromant;

  ax_denorm = ax == 0;
  bx_denorm = bx == 0;

  assert(ax_denorm || bx_denorm);

  a1 = a[1]; a2 = a[2]; a3 = a[3];
  b1 = b[1]; b2 = b[2]; b3 = b[3];
  

}

/* Add a+b+cin modulo 2^32, put result in 'r' and carry in 'cout'.  */
#define addc(r,cout,a,b,cin) \
  do { \
    unsigned long long addc_tmp = (a)+(b)+(cin);
    (cout) = addc_tmp >> 32;
    (r) = addc_tmp;
  }

/* Calculate a+~b+cin modulo 2^32, put result in 'r' and carry in 'cout'.  */
#define subc(r,cout,a,b,cin) \
  do { \
    unsigned long long addc_tmp = (a)-(b)+(cin)-1;
    (cout) = addc_tmp >> 63;
    (r) = addc_tmp;
  }

/* Handle the case where both exponents are the same.  This requires quite
   a different algorithm than the general case.  */
static void
handle_equal_exponents(unsigned result[4],
		       const unsigned a[4], const unsigned b[4],
		       const unsigned axx,  /* Treat as a[0].  */
		       const unsigned bxx,  /* Treat as b[0].  */
		       unsigned ax)         /* [ab]xx >> 16 & 0x7fff.  */
{
  unsigned a1,a2,a3, b1,b2,b3;
  int roundmode;
  unsigned carry, r0;

  a1 = a[1]; a2 = a[2]; a3 = a[3];
  b1 = b[1]; b2 = b[2]; b3 = b[3];

  if ((int)(axx ^ bxx) >= 0)
    {
      int roundmode;

      /* Adding.  */
      roundmode = fegetround();
  
      /* What about overflow?  */
      if (ax == 0x7ffe)
	{
	  /* Oh no!  Too big!  */
	  /* Result:
	     rounding result
	     -------- ------
	     nearest  return Inf with sign of a,b
	     zero     return nearest possible non-Inf value with
	              sign of a,b
	     +Inf     return +Inf if a,b>0, otherwise return
	              value just before -Inf.
	     -Inf     return +Inf if a,b>0, otherwise return
	              value just before -Inf.
	   */
	  set_fpscr_bit(FPSCR_OX);
	  /* Overflow always produces inexact result.  */
	  set_fpscr_bit(FPSCR_XX);

	  if (   roundmode == FE_TONEAREST
	      || roundmode == FE_UPWARD && (int)axx >= 0
	      || roundmode == FE_DOWNWARD && (int)axx < 0)
	    {
	      result[3] = result[2] = result[1] = 0;
	      result[0] = axx & 0xffff0000 | 0x7fff0000;
	    }
	  else
	    {
	      result[3] = result[2] = result[1] = 0xffffffff;
	      result[0] = axx & 0xfffe0000 | 0x7ffeffff;
	    }
	  return;
	}

      /* We need to worry about rounding/inexact here.  Do it like this: */
      if (a3 + b3  &  1)
	{
	  /* Need to round.  Upwards?  */
	  set_fpscr_bit(FPSCR_XX);
	  carry = (   roundmode == FE_NEAREST && (a3 + b3  &  2) != 0
		   || roundmode == FE_UPWARD && (int)axx >= 0
		   || roundmode == FE_DOWNWARD && (int)axx < 0);
	}
      else
	carry = 0; /* Result will be exact.  */

      /* Perform the addition.  */
      addc(a3,carry,a3,b3,carry);
      addc(a2,carry,a2,b2,carry);
      addc(a1,carry,a1,b1,carry);
      r0 = (axx & 0xffff) + (bxx & 0xffff) + carry;

      /* Shift right by 1.  */
      result[3] = a3 >> 1 | a2 << 31;
      result[2] = a2 >> 1 | a1 << 31;
      result[1] = a1 >> 1 | r0 << 31;
      /* Exponent of result is exponent of inputs plus 1.  
         Sign of result is common sign of inputs.  */
      result[0] = r0 >> 1 & 0xffff  |  axx + 0x10000 & 0xffff0000;
    }
  else
    {
      /* Subtracting.  */
      
      /* Perform the subtraction, a-b.  */
      subc(a3,carry,a3,b3,0);
      subc(a2,carry,a2,b2,carry);
      subc(a1,carry,a1,b1,carry);
      subc(r0,carry,a0&0xffff,b0&0xffff,carry);
      
      /* Maybe we should have calculated b-a... */
      if (carry)
	{
	  subc(a3,carry,0,a3,0);
	  subc(a2,carry,0,a2,carry);
	  subc(a1,carry,0,a1,carry);
	  subc(r0,carry,0,r0,carry);
	  axx ^= 0x80000000;
	}
      
      renormalise_value(result, axx, ax, r0, a1, a2, a3);
    }
}


static void
add(unsigned result[4], const unsigned a[4], const unsigned b[4],
    unsigned axx, unsigned bxx)
{
  int ax, bx, diff, carry;
  unsigned a0,a1,a2,a3, b0,b1,b2,b3,b4, sdiff;

  ax = axx >> 16  &  0x7fff;
  bx = bxx >> 16  &  0x7fff;

  /* Deal with NaNs and Inf.  */
  if (ax == 0x7fff || bx == 0x7fff)
    {
      handle_max_exponent(result, a, b, axx, bxx, ax, bx);
      return;
    }
  /* Deal with denorms and zero.  */
  if (ax == 0 || bx == 0)
    {
      handle_min_exponent(result, a, b, axx, bxx, ax, bx);
      return;
    }
  /* Finally, one special case, when both exponents are equal.  */
  if (ax == bx)
    {
      handle_equal_exponents(result, a, b, axx, bxx, ax);
      return;
    }

  sdiff = axx ^ bxx;
  /* Swap a and b if b has a larger magnitude than a, so that a will have
     the larger magnitude.  */
  if (ax < bx)
    {
      const unsigned *t;
      t = b; b = a; a = t;
      diff = bx - ax;
      ax = bx;
      axx = bxx;
    }
  else
    diff = ax - bx;

  a0 = a[0] & 0xffff | 0x10000; a1 = a[1]; a2 = a[2]; a3 = a[3];
  b0 = b[0] & 0xffff | 0x10000; b1 = b[1]; b2 = b[2]; b3 = b[3];
  if (diff < 32)
    {
      b4 = b3 << 32-diff;
      b3 = b3 >> diff | b2 << 32-biff;
      b2 = b2 >> diff | b1 << 32-diff;
      b1 = b1 >> diff | b0 << 32-diff;
      b0 = b0 >> diff;
    }
  else if (diff < 64)
    {
      diff -= 32;
      b4 = b3 & 1 | b3 >> (diff == 32) | b2 << 32-biff;
      b3 = b2 >> diff | b1 << 32-diff;
      b2 = b1 >> diff | b0 << 32-diff;
      b1 = b0 >> diff;
      b0 = 0;
    }
  else if (diff < 96)
    {
      b4 = b2 | b3 | b1 << 32-diff;
      b3 = b1 >> diff | b0 << 32-diff;
      b2 = b0 >> diff;
      b1 = b0 = 0;
    }
  else if (diff < 128)
    {
      b4 = b1 | b2 | b3 | b0 << 32-diff;
      b3 = b0 >> diff;
      b2 = b1 = b0 = 0;
    }
  else
    {
      b4 = b0|b1|b2|b3;
      b3 = b2 = b1 = b0 = 0;
    }

  /* Now, two cases: one for addition, one for subtraction.  */
  if ((int)sdiff >= 0)
    {
      /* Addition.  */

      /* 

      /* Perform the addition.  */
      addc(a3,carry,a3,b3,0);
      addc(a2,carry,a2,b2,carry);
      addc(a1,carry,a1,b1,carry);
      addc(a0,carry,a0,b0,carry);

      

      if (a0 & 0x20000)
	{
	  /* Need to renormalise by shifting right.  */
	  /* Shift right by 1.  */
	  b4 = b4 | a3 << 31;
	  a3 = a3 >> 1 | a2 << 31;
	  a2 = a2 >> 1 | a1 << 31;
	  result[1] = a1 >> 1 | r0 << 31;
	  /* Exponent of result is exponent of inputs plus 1.  
	     Sign of result is common sign of inputs.  */
	  result[0] = r0 >> 1 & 0xffff  |  axx + 0x10000 & 0xffff0000;
	}
      

    }
  else
    {
      /* Subtraction.  */
      
    }
}

/* Add the absolute values of two 128-bit floating point values,
   give the result the sign of one of them.  The only exception this
   can raise is for SNaN.  */
static void
aadd(unsigned result[4], const unsigned a[4], const unsigned b[4])
{
  unsigned ax, bx, xd;
  const unsigned *sml;
  unsigned t0,t1,t2,t3,tx, s0,s1,s2,s3,s4, carry;
  int rmode, xdelta, shift;

  ax = a[0] >> 16 & 0x7fff;
  bx = b[0] >> 16 & 0x7fff;
  
  /* Deal with .  */
  if (ax == 0x7fff)
    {
      t0 = a[0]; t1 = a[1]; t2 = a[2]; t3 = a[3];
      /* Check for SNaN.  */
      if ((t0 & 0x8000) == 0
	  && (t0 & 0x7fff  |  t1  |  t2  |  t3) != 0)
	set_fpscr_bit(FPSCR_VXSNAN);
      /* Return b.  */
      result[0] = t0; result[1] = t1; result[2] = t2; result[3] = t3;
      return;
    }
  /* Deal with b==Inf or b==NaN. */
  if (bx == 0x7fff)
    {
      t0 = b[0]; t1 = b[1]; t2 = b[2]; t3 = b[3];
      /* Check for SNaN.  */
      if ((t0 & 0x8000) == 0
	  && (t0 & 0x7fff  |  t1  |  t2  |  t3) != 0)
	set_fpscr_bit(FPSCR_VXSNAN);
      /* Return b.  */
      result[0] = t0; result[1] = t1; result[2] = t2; result[3] = t3;
      return;
    }

  /* Choose the larger of the two to be 't', and the smaller to be 's'.  */
  if (ax > bx)
    {
      t0 = a[0] & 0xffff  |  (ax != 0) << 16;
      t1 = a[1]; t2 = a[2]; t3 = a[3]; tx = ax;
      s0 = b[0] & 0xffff  |  (bx != 0) << 16;
      s1 = b[1]; s2 = b[2]; s3 = b[3];
      xd = ax-bx;
    }
  else
    {
      t0 = b[0] & 0xffff  |  (bx != 0) << 16;
      t1 = b[1]; t2 = b[2]; t3 = b[3]; tx = bx;
      s0 = a[0] & 0xffff  |  (ax != 0) << 16;
      s1 = a[1]; s2 = a[2]; s3 = a[3];
      sml = a;
      xd = bx-ax;
    }

  /* Shift 's2' right by 'xd' bits. */
  switch (xd >> 5)
    {
    case 0:
      s4 = 0;
      break;
    case 1:
      s4 = s3; s3 = s2; s2 = s1; s1 = s0; s0 = 0;
      break;
    case 2:
      s4 = s2  |  s3 != 0;
      s3 = s1; s2 = s0; s1 = s0 = 0;
      break;
    case 3:
      s4 = s1  |  (s3|s2) != 0;
      s3 = s0; s2 = s1 = s0 = 0;
      break;
    default:
      s4 = s0  |  (s3|s2|s1) != 0;
      s3 = s2 = s1 = s0 = 0;
    }
  xd = xd & 0x1f;
  if (xd != 0)
    {
      s4 = s4 >> xd  |  (s4 << 32-xd) != 0  |  s3 << 32-xd;
      s3 = s3 >> xd  |  s2 << 32-xd;
      s2 = s2 >> xd  |  s1 << 32-xd;
      s1 = s1 >> xd  |  s0 << 32-xd;
      s0 = s0 >> xd;
    }

  /* Do the addition.  */
#define addc(r,cout,a,b,cin) \
  do { \
    unsigned long long addc_tmp = (a)+(b)+(cin);
    (cout) = addc_tmp >> 32;
    (r) = addc_tmp;
  }
  addc(t3,carry,t3,s3,0);
  addc(t2,carry,t2,s2,carry);
  addc(t1,carry,t1,s1,carry);
  t0 = t0 + s0 + carry;
  
  /* Renormalise.  */
  xdelta = 15-cntlzw(t0);
  if (tx + xdelta <= 0x7fff)
    shift = xdelta;
  else
    {
    }
}

/* Add two 128-bit floating point values.  */
void
__q_add(unsigned result[4], const unsigned a[4], const unsigned b[4])
{
  if ((a[0] ^ b[0]) >= 0)
    aadd(result, a, b);
  else
    asubtract(result, a, b);
}

/* Subtract two 128-bit floating point values.  */
void
__q_sub(unsigned result[4], const unsigned a[4], const unsigned b[4])
{
  if ((a[0] ^ b[0]) < 0)
    aadd(result, a, b);
  else
    asubtract(result, a, b);
}
