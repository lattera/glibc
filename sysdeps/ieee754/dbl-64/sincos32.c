
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
/****************************************************************/
/*  MODULE_NAME: sincos32.c                                     */
/*                                                              */
/*  FUNCTIONS: ss32                                             */
/*             cc32                                             */
/*             c32                                              */
/*             sin32                                            */
/*             cos32                                            */
/*             mpsin                                            */
/*             mpcos                                            */
/*             mpranred                                         */
/*             mpsin1                                           */
/*             mpcos1                                           */
/*                                                              */
/* FILES NEEDED: endian.h mpa.h sincos32.h                      */
/*               mpa.c                                          */
/*                                                              */
/* Multi Precision sin() and cos() function with p=32  for sin()*/
/* cos() arcsin() and arccos() routines                         */
/* In addition mpranred() routine  performs range  reduction of */
/* a double number x into multi precision number   y,           */
/* such that y=x-n*pi/2, abs(y)<pi/4,  n=0,+-1,+-2,....         */
/****************************************************************/
#include "endian.h"
#include "mpa.h"
#include "sincos32.h"

/****************************************************************/
/* Compute Multi-Precision sin() function for given p.  Receive */
/* Multi  Precision number x and result stored at y             */
/****************************************************************/
void ss32(mp_no *x, mp_no *y, int p) {
  int i;
  double a,b;
  static const mp_no mpone = {1,1.0,1.0};
  mp_no mpt1,mpt2,x2,gor,sum ,mpk={1,1.0};
  for (i=1;i<=p;i++) mpk.d[i]=0;

  mul(x,x,&x2,p);
  cpy(&oofac27,&gor,p);
  cpy(&gor,&sum,p);
  for (a=27.0;a>1.0;a-=2.0) {
    mpk.d[1]=a*(a-1.0);
    mul(&gor,&mpk,&mpt1,p);
    cpy(&mpt1,&gor,p);
    mul(&x2,&sum,&mpt1,p);
    sub(&gor,&mpt1,&sum,p);
  }
  mul(x,&sum,y,p);
}

/**********************************************************************/
/* Compute Multi-Precision cos() function for given p. Receive Multi  */
/* Precision number x and result stored at y                          */
/**********************************************************************/
void cc32(mp_no *x, mp_no *y, int p) {
  int i;
  double a,b;
  static const mp_no mpone = {1,1.0,1.0};
  mp_no mpt1,mpt2,x2,gor,sum ,mpk={1,1.0};
  for (i=1;i<=p;i++) mpk.d[i]=0;

  mul(x,x,&x2,p);
  mpk.d[1]=27.0;
  mul(&oofac27,&mpk,&gor,p);
  cpy(&gor,&sum,p);
  for (a=26.0;a>2.0;a-=2.0) {
    mpk.d[1]=a*(a-1.0);
    mul(&gor,&mpk,&mpt1,p);
    cpy(&mpt1,&gor,p);
    mul(&x2,&sum,&mpt1,p);
    sub(&gor,&mpt1,&sum,p);
  }
  mul(&x2,&sum,y,p);
}

/***************************************************************************/
/* c32()   computes both sin(x), cos(x) as Multi precision numbers         */
/***************************************************************************/
void c32(mp_no *x, mp_no *y, mp_no *z, int p) {
  static const mp_no mpt={1,1.0,2.0}, one={1,1.0,1.0};
  mp_no u,t,t1,t2,c,s;
  int i;
  cpy(x,&u,p);
  u.e=u.e-1;
  cc32(&u,&c,p);
  ss32(&u,&s,p);
  for (i=0;i<24;i++) {
    mul(&c,&s,&t,p);
    sub(&s,&t,&t1,p);
    add(&t1,&t1,&s,p);
    sub(&mpt,&c,&t1,p);
    mul(&t1,&c,&t2,p);
    add(&t2,&t2,&c,p);
  }
  sub(&one,&c,y,p);
  cpy(&s,z,p);
}

/************************************************************************/
/*Routine receive double x and two double results of sin(x) and return  */
/*result which is more accurate                                         */
/*Computing sin(x) with multi precision routine c32                     */
/************************************************************************/
double sin32(double x, double res, double res1) {
  int p;
  mp_no a,b,c;
  p=32;
  dbl_mp(res,&a,p);
  dbl_mp(0.5*(res1-res),&b,p);
  add(&a,&b,&c,p);
  if (x>0.8)
  { sub(&hp,&c,&a,p);
    c32(&a,&b,&c,p);
  }
  else c32(&c,&a,&b,p);     /* b=sin(0.5*(res+res1))  */
  dbl_mp(x,&c,p);           /* c = x                  */
  sub(&b,&c,&a,p);    
  /* if a>0 return min(res,res1), otherwise return max(res,res1) */
  if (a.d[0]>0)  return (res<res1)?res:res1;
  else  return (res>res1)?res:res1;
}

/************************************************************************/
/*Routine receive double x and two double results of cos(x) and return  */
/*result which is more accurate                                         */
/*Computing cos(x) with multi precision routine c32                     */
/************************************************************************/
double cos32(double x, double res, double res1) {
  int p;
  mp_no a,b,c;
  p=32;
  dbl_mp(res,&a,p);
  dbl_mp(0.5*(res1-res),&b,p);
  add(&a,&b,&c,p);
  if (x>2.4)
  { sub(&pi,&c,&a,p);
    c32(&a,&b,&c,p);
    b.d[0]=-b.d[0];
  }
  else if (x>0.8) 
       { sub(&hp,&c,&a,p);
         c32(&a,&c,&b,p); 
       }
  else c32(&c,&b,&a,p);     /* b=cos(0.5*(res+res1))  */
  dbl_mp(x,&c,p);    /* c = x                  */
  sub(&b,&c,&a,p);
             /* if a>0 return max(res,res1), otherwise return min(res,res1) */
  if (a.d[0]>0)  return (res>res1)?res:res1;
  else  return (res<res1)?res:res1;
}

/*******************************************************************/
/*Compute sin(x+dx) as Multi Precision number and return result as */
/* double                                                          */
/*******************************************************************/
double mpsin(double x, double dx) {
  int p;
  double y;
  mp_no a,b,c;
  p=32;
  dbl_mp(x,&a,p);
  dbl_mp(dx,&b,p);
  add(&a,&b,&c,p);
  if (x>0.8) { sub(&hp,&c,&a,p); c32(&a,&b,&c,p); }
  else c32(&c,&a,&b,p);     /* b = sin(x+dx)     */
  mp_dbl(&b,&y,p);
  return y;
}

/*******************************************************************/
/* Compute cos()of double-length number (x+dx) as Multi Precision  */
/* number and return result as double                              */
/*******************************************************************/
double mpcos(double x, double dx) {
  int p;
  double y;
  mp_no a,b,c;
  p=32;
  dbl_mp(x,&a,p);
  dbl_mp(dx,&b,p);
  add(&a,&b,&c,p);
  if (x>0.8) 
  { sub(&hp,&c,&b,p);
    c32(&b,&a,&c,p);
  }
  else c32(&c,&a,&b,p);     /* a = cos(x+dx)     */
  mp_dbl(&a,&y,p);
  return y;
}

/******************************************************************/
/* mpranred() performs range reduction of a double number x into  */
/* multi precision number y, such that y=x-n*pi/2, abs(y)<pi/4,   */
/* n=0,+-1,+-2,....                                               */
/* Return int which indicates in which quarter of circle x is     */
/******************************************************************/
int mpranred(double x, mp_no *y, int p)
{
  number v;
  double t,xn;
  int i,k,n;
  static const mp_no one = {1,1.0,1.0};
  mp_no a,b,c;
  
  if (ABS(x) < 2.8e14) {
    t = (x*hpinv.d + toint.d);
    xn = t - toint.d;
    v.d = t;
    n =v.i[LOW_HALF]&3;
    dbl_mp(xn,&a,p);
    mul(&a,&hp,&b,p);
    dbl_mp(x,&c,p);
    sub(&c,&b,y,p);
    return n;
  }
  else {                      /* if x is very big more precision required */
    dbl_mp(x,&a,p);
    a.d[0]=1.0;
    k = a.e-5;
    if (k < 0) k=0;
    b.e = -k;
    b.d[0] = 1.0;
    for (i=0;i<p;i++) b.d[i+1] = toverp[i+k];
    mul(&a,&b,&c,p);
    t = c.d[c.e];
    for (i=1;i<=p-c.e;i++) c.d[i]=c.d[i+c.e];
    for (i=p+1-c.e;i<=p;i++) c.d[i]=0;
    c.e=0;
    if (c.d[1] >=  8388608.0)  
    { t +=1.0;
      sub(&c,&one,&b,p);
      mul(&b,&hp,y,p);
    }
    else mul(&c,&hp,y,p);
    n = (int) t;
    if (x < 0) { y->d[0] = - y->d[0]; n = -n; }
    return (n&3);
  }
}

/*******************************************************************/
/* Multi-Precision sin() function subroutine, for p=32.  It is     */
/* based on the routines mpranred() and c32().                     */
/*******************************************************************/
double mpsin1(double x)
{
  int p;
  int n;
  mp_no u,s,c;
  double y;
  p=32;
  n=mpranred(x,&u,p);               /* n is 0, 1, 2 or 3 */
  c32(&u,&c,&s,p);
  switch (n) {                      /* in which quarter of unit circle y is*/
  case 0:
    mp_dbl(&s,&y,p);
    return y;
    break;

  case 2:
    mp_dbl(&s,&y,p);
    return -y;
    break;

  case 1:
    mp_dbl(&c,&y,p);
    return y;
    break;
    
  case 3:
    mp_dbl(&c,&y,p);
    return -y;
    break;
    
  }
  return 0;                     /* unreachable, to make the compiler happy */
}

/*****************************************************************/
/* Multi-Precision cos() function subroutine, for p=32.  It is   */
/* based  on the routines mpranred() and c32().                  */
/*****************************************************************/

double mpcos1(double x)
{
  int p;
  int n;
  mp_no u,s,c;
  double y;
  
  p=32;
  n=mpranred(x,&u,p);              /* n is 0, 1, 2 or 3 */
  c32(&u,&c,&s,p);
  switch (n) {                     /* in what quarter of unit circle y is*/
    
  case 0:
    mp_dbl(&c,&y,p);
    return y;
    break;
    
  case 2:
    mp_dbl(&c,&y,p);
    return -y;
    break;
    
  case 1:
    mp_dbl(&s,&y,p);
    return -y;
    break;
    
  case 3:
    mp_dbl(&s,&y,p);
    return y;
    break;
    
  }
  return 0;                     /* unreachable, to make the compiler happy */
}
/******************************************************************/
