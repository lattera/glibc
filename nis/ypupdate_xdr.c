/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user or with the express written consent of
 * Sun Microsystems, Inc.
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
/*
 * Copyright (c) 1986, 1990 by Sun Microsystems, Inc.
 */

#include <rpcsvc/ypupd.h>

bool_t
xdr_yp_buf (XDR *xdrs, yp_buf *objp)
{
  return xdr_bytes (xdrs, (char **) &objp->yp_buf_val,
		    (u_int *) &objp->yp_buf_len, ~0);
}
libnsl_hidden_def (xdr_yp_buf)

bool_t
xdr_ypupdate_args (XDR *xdrs, ypupdate_args *objp)
{
  if (!xdr_string (xdrs, &objp->mapname, ~0))
    return FALSE;
  if (!xdr_yp_buf (xdrs, &objp->key))
    return FALSE;
  return xdr_yp_buf (xdrs, &objp->datum);
}

bool_t
xdr_ypdelete_args (XDR *xdrs, ypdelete_args *objp)
{
  if (!xdr_string (xdrs, &objp->mapname, ~0))
    return FALSE;
  return xdr_yp_buf (xdrs, &objp->key);
}
