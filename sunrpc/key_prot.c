/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 *
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */
#if 0
#pragma ident	"@(#)key_prot.x	1.7	94/04/29 SMI"
#endif

/* Copyright (c)  1990, 1991 Sun Microsystems, Inc. */

#include "rpc/key_prot.h"

bool_t
xdr_keystatus (XDR * xdrs, keystatus * objp)
{
  if (!INTUSE(xdr_enum) (xdrs, (enum_t *) objp))
    return FALSE;

  return TRUE;
}
INTDEF(xdr_keystatus)

bool_t
xdr_keybuf (XDR * xdrs, keybuf objp)
{
  if (!INTUSE(xdr_opaque) (xdrs, objp, HEXKEYBYTES))
    return FALSE;

  return TRUE;
}
INTDEF(xdr_keybuf)

bool_t
xdr_netnamestr (XDR * xdrs, netnamestr * objp)
{
  if (!INTUSE(xdr_string) (xdrs, objp, MAXNETNAMELEN))
    return FALSE;

  return TRUE;
}
INTDEF(xdr_netnamestr)

bool_t
xdr_cryptkeyarg (XDR * xdrs, cryptkeyarg * objp)
{
  if (!INTUSE(xdr_netnamestr) (xdrs, &objp->remotename))
    return FALSE;

  if (!INTUSE(xdr_des_block) (xdrs, &objp->deskey))
    return FALSE;

  return TRUE;
}
INTDEF(xdr_cryptkeyarg)

bool_t
xdr_cryptkeyarg2 (XDR * xdrs, cryptkeyarg2 * objp)
{
  if (!INTUSE(xdr_netnamestr) (xdrs, &objp->remotename))
    return FALSE;
  if (!INTUSE(xdr_netobj) (xdrs, &objp->remotekey))
    return FALSE;
  if (!INTUSE(xdr_des_block) (xdrs, &objp->deskey))
    return FALSE;
  return TRUE;
}
INTDEF(xdr_cryptkeyarg2)

bool_t
xdr_cryptkeyres (XDR * xdrs, cryptkeyres * objp)
{
  if (!INTUSE(xdr_keystatus) (xdrs, &objp->status))
    return FALSE;
  switch (objp->status)
    {
    case KEY_SUCCESS:
      if (!INTUSE(xdr_des_block) (xdrs, &objp->cryptkeyres_u.deskey))
	return FALSE;
      break;
    default:
      break;
    }
  return TRUE;
}
INTDEF(xdr_cryptkeyres)

bool_t
xdr_unixcred (XDR * xdrs, unixcred * objp)
{
  if (!INTUSE(xdr_u_int) (xdrs, &objp->uid))
    return FALSE;
  if (!INTUSE(xdr_u_int) (xdrs, &objp->gid))
    return FALSE;
  if (!INTUSE(xdr_array) (xdrs, (char **) &objp->gids.gids_val,
			  (u_int *) & objp->gids.gids_len, MAXGIDS,
			  sizeof (u_int), (xdrproc_t) INTUSE(xdr_u_int)))
    return FALSE;
  return TRUE;
}
INTDEF(xdr_unixcred)

bool_t
xdr_getcredres (XDR * xdrs, getcredres * objp)
{
  if (!INTUSE(xdr_keystatus) (xdrs, &objp->status))
    return FALSE;
  switch (objp->status)
    {
    case KEY_SUCCESS:
      if (!INTUSE(xdr_unixcred) (xdrs, &objp->getcredres_u.cred))
	return FALSE;
      break;
    default:
      break;
    }
  return TRUE;
}

bool_t
xdr_key_netstarg (XDR * xdrs, key_netstarg * objp)
{
  if (!INTUSE(xdr_keybuf) (xdrs, objp->st_priv_key))
    return FALSE;
  if (!INTUSE(xdr_keybuf) (xdrs, objp->st_pub_key))
    return FALSE;
  if (!INTUSE(xdr_netnamestr) (xdrs, &objp->st_netname))
    return FALSE;
  return TRUE;
}
INTDEF(xdr_key_netstarg)

bool_t
xdr_key_netstres (XDR * xdrs, key_netstres * objp)
{
  if (!INTUSE(xdr_keystatus) (xdrs, &objp->status))
    return FALSE;
  switch (objp->status)
    {
    case KEY_SUCCESS:
      if (!INTUSE(xdr_key_netstarg) (xdrs, &objp->key_netstres_u.knet))
	return FALSE;
      break;
    default:
      break;
    }
  return TRUE;
}
INTDEF(xdr_key_netstres)
