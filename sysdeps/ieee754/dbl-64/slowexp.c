
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
/*  MODULE_NAME:slowexp.c                                                 */
/*                                                                        */
/*  FUNCTION:slowexp                                                      */  
/*                                                                        */
/*  FILES NEEDED:mpa.h                                                    */
/*               mpa.c mpexp.c                                            */
/*                                                                        */
/*Converting from double precision to Multi-precision and calculating     */
/* e^x                                                                    */
/**************************************************************************/
#include "mpa.h"

void mpexp(mp_no *x, mp_no *y, int p);

/*Converting from double precision to Multi-precision and calculating  e^x */
double slowexp(double x) {
  double y,w,z,res,eps=3.0e-26;
  int orig,i,p;
  mp_no mpx, mpy, mpz,mpw,mpeps,mpcor;
  
  p=6;
  dbl_mp(x,&mpx,p); /* Convert a double precision number  x               */
                    /* into a multiple precision number mpx with prec. p. */
  mpexp(&mpx, &mpy, p); /* Multi-Precision exponential function */
  dbl_mp(eps,&mpeps,p);
  mul(&mpeps,&mpy,&mpcor,p);
  add(&mpy,&mpcor,&mpw,p);
  sub(&mpy,&mpcor,&mpz,p);
  mp_dbl(&mpw, &w, p);
  mp_dbl(&mpz, &z, p);
  if (w == z) return w;
  else  {                   /* if calculating is not exactly   */
    p = 32;
    dbl_mp(x,&mpx,p);
    mpexp(&mpx, &mpy, p);
    mp_dbl(&mpy, &res, p);
    return res;
  }
}

