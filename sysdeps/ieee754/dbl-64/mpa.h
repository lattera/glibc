/*
 * IBM Accurate Mathematical Library
 * Written by International Business Machines Corp.
 * Copyright (C) 2001-2013 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/************************************************************************/
/*  MODULE_NAME: mpa.h                                                  */
/*                                                                      */
/*  FUNCTIONS:                                                          */
/*               mcr                                                    */
/*               acr                                                    */
/*               cpy                                                    */
/*               mp_dbl                                                 */
/*               dbl_mp                                                 */
/*               add                                                    */
/*               sub                                                    */
/*               mul                                                    */
/*               dvd                                                    */
/*                                                                      */
/* Arithmetic functions for multiple precision numbers.                 */
/* Common types and definition                                          */
/************************************************************************/


/* The mp_no structure holds the details of a multi-precision floating point
   number.

   - The radix of the number (R) is 2 ^ 24.

   - E: The exponent of the number.

   - D[0]: The sign (-1, 1) or 0 if the value is 0.  In the latter case, the
     values of the remaining members of the structure are ignored.

   - D[1] - D[p]: The mantissa of the number where:

	0 <= D[i] < R and
	P is the precision of the number and 1 <= p <= 32

     D[p+1] ... D[39] have no significance.

   - The value of the number is:

	D[1] * R ^ (E - 1) + D[2] * R ^ (E - 2) ... D[p] * R ^ (E - p)

   */
typedef struct
{
  int e;
  double d[40];
} mp_no;

typedef union
{
  int i[2];
  double d;
} number;

extern const mp_no mpone;
extern const mp_no mptwo;

#define  X   x->d
#define  Y   y->d
#define  Z   z->d
#define  EX  x->e
#define  EY  y->e
#define  EZ  z->e

#define ABS(x)   ((x) <  0  ? -(x) : (x))

int __acr (const mp_no *, const mp_no *, int);
void __cpy (const mp_no *, mp_no *, int);
void __mp_dbl (const mp_no *, double *, int);
void __dbl_mp (double, mp_no *, int);
void __add (const mp_no *, const mp_no *, mp_no *, int);
void __sub (const mp_no *, const mp_no *, mp_no *, int);
void __mul (const mp_no *, const mp_no *, mp_no *, int);
void __dvd (const mp_no *, const mp_no *, mp_no *, int);

extern void __mpatan (mp_no *, mp_no *, int);
extern void __mpatan2 (mp_no *, mp_no *, mp_no *, int);
extern void __mpsqrt (mp_no *, mp_no *, int);
extern void __mpexp (mp_no *, mp_no *, int);
extern void __c32 (mp_no *, mp_no *, mp_no *, int);
extern int __mpranred (double, mp_no *, int);
