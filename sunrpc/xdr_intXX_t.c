/* Copyright (c) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Thorsten Kukuk <kukuk@vt.uni-paderborn.de>, 1998.

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
   Boston, MA 02111-1307, USA. */

#include <rpc/types.h>
#include <rpc/xdr.h>

/* XDR 32bit integers */
bool_t
xdr_int32_t (XDR *xdrs, int32_t *lp)
{
  switch (xdrs->x_op)
    {
    case XDR_ENCODE:
      return XDR_PUTINT32 (xdrs, lp);
    case XDR_DECODE:
      return XDR_GETINT32 (xdrs, lp);
    case XDR_FREE:
      return TRUE;
    default:
      return FALSE;
    }
}

/* XDR 32bit unsigned integers */
bool_t
xdr_uint32_t (XDR *xdrs, uint32_t *ulp)
{
  switch (xdrs->x_op)
    {
    case XDR_DECODE:
      return XDR_GETINT32 (xdrs, (int32_t *) ulp);
    case XDR_ENCODE:
      return XDR_PUTINT32 (xdrs, (int32_t *) ulp);
    case XDR_FREE:
      return TRUE;
    default:
      return FALSE;
    }
}

/* XDR 16bit integers */
bool_t
xdr_int16_t (XDR *xdrs, int16_t *ip)
{
  int32_t t;

  switch (xdrs->x_op)
    {
    case XDR_ENCODE:
      t = (int32_t) *ip;
      return XDR_PUTINT32 (xdrs, &t);
    case XDR_DECODE:
      if (!XDR_GETINT32 (xdrs, &t))
	return FALSE;
      *ip = (int16_t) t;
      return TRUE;
    case XDR_FREE:
      return TRUE;
    default:
      return FALSE;
    }
}

/* XDR 16bit unsigned integers */
bool_t
xdr_uint16_t (XDR *xdrs, uint16_t *uip)
{
  uint32_t ut;

  switch (xdrs->x_op)
    {
    case XDR_DECODE:
      ut = (uint32_t) *uip;
      return XDR_GETINT32 (xdrs, (int32_t *) &ut);
    case XDR_ENCODE:
      if (!XDR_PUTINT32 (xdrs, (int32_t *) &ut))
	return FALSE;
      *uip = (uint16_t) ut;
      return TRUE;
    case XDR_FREE:
      return TRUE;
    default:
      return FALSE;
    }
}

/* XDR 8bit integers */
bool_t
xdr_int8_t (XDR *xdrs, int8_t *ip)
{
  int32_t t;

  switch (xdrs->x_op)
    {
    case XDR_ENCODE:
      t = (int32_t) *ip;
      return XDR_PUTINT32 (xdrs, &t);
    case XDR_DECODE:
      if (!XDR_GETINT32 (xdrs, &t))
	return FALSE;
      *ip = (int8_t) t;
      return TRUE;
    case XDR_FREE:
      return TRUE;
    default:
      return FALSE;
    }
}

/* XDR 8bit unsigned integers */
bool_t
xdr_uint8_t (XDR *xdrs, uint8_t *uip)
{
  uint32_t ut;

  switch (xdrs->x_op)
    {
    case XDR_DECODE:
      ut = (uint32_t) *uip;
      return XDR_GETINT32 (xdrs, (int32_t *) &ut);
    case XDR_ENCODE:
      if (!XDR_PUTINT32 (xdrs, (int32_t *) &ut))
	return FALSE;
      *uip = (uint8_t) ut;
      return TRUE;
    case XDR_FREE:
      return TRUE;
    default:
      return FALSE;
    }
}
