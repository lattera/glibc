/*
 * IBM Accurate Mathematical Library
 * Copyright (c) International Business Machines Corp., 2001
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/**************************************************************************/
/*  MODULE_NAME urem.c                                                    */
/*                                                                        */
/*  FUNCTION: uremainder                                                  */
/*                                                                        */
/* An ultimate remainder routine. Given two IEEE double machine numbers x */
/* ,y   it computes the correctly rounded (to nearest) value of remainder */
/* of dividing x by y.                                                    */
/* Assumption: Machine arithmetic operations are performed in             */
/* round to nearest mode of IEEE 754 standard.                            */
/*                                                                        */
/* ************************************************************************/

#include "endian.h"
#include "mydefs.h"
#include "urem.h"
#include "MathLib.h"

/**************************************************************************/
/* An ultimate remainder routine. Given two IEEE double machine numbers x */
/* ,y   it computes the correctly rounded (to nearest) value of remainder */
/**************************************************************************/
double __ieee754_remainder(double x, double y)
{
  double z,d,xx;
#if 0
  double yy;
#endif
  int4 kx,ky,n,nn,n1,m1,l;
#if 0
  int4 m;
#endif
  mynumber u,t,w={{0,0}},v={{0,0}},ww={{0,0}},r;
  u.x=x;
  t.x=y;
  kx=u.i[HIGH_HALF]&0x7fffffff; /* no sign  for x*/
  t.i[HIGH_HALF]&=0x7fffffff;   /*no sign for y */
  ky=t.i[HIGH_HALF];
  /*------ |x| < 2^1024  and   2^-970 < |y| < 2^1024 ------------------*/
  if (kx<0x7ff00000 && ky<0x7ff00000 && ky>=0x03500000) {
    if (kx+0x00100000<ky) return x;
    if ((kx-0x01500000)<ky) {
      z=x/t.x;
      v.i[HIGH_HALF]=t.i[HIGH_HALF];
      d=(z+big.x)-big.x;
      xx=(x-d*v.x)-d*(t.x-v.x);
      if (d-z!=0.5&&d-z!=-0.5) return (xx!=0)?xx:((x>0)?ZERO.x:nZERO.x);
      else {
	if (ABS(xx)>0.5*t.x) return (z>d)?xx-t.x:xx+t.x;
	else return xx;
      }
    }   /*    (kx<(ky+0x01500000))         */
    else  {
      r.x=1.0/t.x;
      n=t.i[HIGH_HALF];
      nn=(n&0x7ff00000)+0x01400000;
      w.i[HIGH_HALF]=n;
      ww.x=t.x-w.x;
      l=(kx-nn)&0xfff00000;
      n1=ww.i[HIGH_HALF];
      m1=r.i[HIGH_HALF];
      while (l>0) {
	r.i[HIGH_HALF]=m1-l;
	z=u.x*r.x;
	w.i[HIGH_HALF]=n+l;
	ww.i[HIGH_HALF]=(n1)?n1+l:n1;
	d=(z+big.x)-big.x;
	u.x=(u.x-d*w.x)-d*ww.x;
	l=(u.i[HIGH_HALF]&0x7ff00000)-nn;
      }
      r.i[HIGH_HALF]=m1;
      w.i[HIGH_HALF]=n;
      ww.i[HIGH_HALF]=n1;
      z=u.x*r.x;
      d=(z+big.x)-big.x;
      u.x=(u.x-d*w.x)-d*ww.x;
      if (ABS(u.x)<0.5*t.x) return (u.x!=0)?u.x:((x>0)?ZERO.x:nZERO.x);
      else
        if (ABS(u.x)>0.5*t.x) return (d>z)?u.x+t.x:u.x-t.x;
        else
        {z=u.x/t.x; d=(z+big.x)-big.x; return ((u.x-d*w.x)-d*ww.x);}
    }

  }   /*   (kx<0x7ff00000&&ky<0x7ff00000&&ky>=0x03500000)     */
  else {
    if (kx<0x7ff00000&&ky<0x7ff00000&&(ky>0||t.i[LOW_HALF]!=0)) {
      y=ABS(y)*t128.x;
      z=uremainder(x,y)*t128.x;
      z=uremainder(z,y)*tm128.x;
      return z;
    }
    else { /* if x is too big */
      if (kx>=0x7ff00000||(ky==0&&t.i[LOW_HALF]==0)||ky>0x7ff00000||
	  (ky==0x7ff00000&&t.i[LOW_HALF]!=0))
	return (u.i[HIGH_HALF]&0x80000000)?nNAN.x:NAN.x;
      else return x;
    }
  }
}
