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

#include <rpcsvc/yp.h>
#include <rpcsvc/ypclnt.h>

bool_t
xdr_ypstat (XDR *xdrs, ypstat *objp)
{
  return xdr_enum (xdrs, (enum_t *) objp);
}
libnsl_hidden_def (xdr_ypstat)

bool_t
xdr_ypxfrstat (XDR *xdrs, ypxfrstat *objp)
{
  return xdr_enum (xdrs, (enum_t *) objp);
}
libnsl_hidden_def (xdr_ypxfrstat)

bool_t
xdr_domainname (XDR *xdrs, domainname *objp)
{
  return xdr_string (xdrs, objp, ~0);
}
libnsl_hidden_def (xdr_domainname)

bool_t
xdr_mapname (XDR *xdrs, mapname *objp)
{
  return xdr_string (xdrs, objp, ~0);
}
libnsl_hidden_def (xdr_mapname)

bool_t
xdr_peername (XDR *xdrs, peername *objp)
{
  return xdr_string (xdrs, objp, ~0);
}
libnsl_hidden_def (xdr_peername)

bool_t
xdr_keydat (XDR *xdrs, keydat *objp)
{
  return xdr_bytes (xdrs, (char **) &objp->keydat_val,
		    (u_int *) &objp->keydat_len, ~0);
}
libnsl_hidden_def (xdr_keydat)

bool_t
xdr_valdat (XDR *xdrs, valdat *objp)
{
  return xdr_bytes (xdrs, (char **) &objp->valdat_val,
		    (u_int *) &objp->valdat_len, ~0);
}
libnsl_hidden_def (xdr_valdat)

bool_t
xdr_ypmap_parms (XDR *xdrs, ypmap_parms *objp)
{
  if (!xdr_domainname (xdrs, &objp->domain))
    return FALSE;
  if (!xdr_mapname (xdrs, &objp->map))
    return FALSE;
  if (!xdr_u_int (xdrs, &objp->ordernum))
    return FALSE;
  return xdr_peername (xdrs, &objp->peer);
}
libnsl_hidden_def (xdr_ypmap_parms)

bool_t
xdr_ypreq_key (XDR *xdrs, ypreq_key *objp)
{
  if (!xdr_domainname (xdrs, &objp->domain))
    return FALSE;
  if (!xdr_mapname (xdrs, &objp->map))
    return FALSE;
  return xdr_keydat (xdrs, &objp->key);
}

bool_t
xdr_ypreq_nokey (XDR *xdrs, ypreq_nokey *objp)
{
  if (!xdr_domainname (xdrs, &objp->domain))
    return FALSE;
  return xdr_mapname (xdrs, &objp->map);
}

bool_t
xdr_ypreq_xfr (XDR *xdrs, ypreq_xfr *objp)
{
  if (!xdr_ypmap_parms (xdrs, &objp->map_parms))
    return FALSE;
  if (!xdr_u_int (xdrs, &objp->transid))
    return FALSE;
  if (!xdr_u_int (xdrs, &objp->prog))
    return FALSE;
  return xdr_u_int (xdrs, &objp->port);
}

bool_t
xdr_ypresp_val (XDR *xdrs, ypresp_val *objp)
{
  if (!xdr_ypstat (xdrs, &objp->stat))
    return FALSE;
  return xdr_valdat (xdrs, &objp->val);
}

bool_t
xdr_ypresp_key_val (XDR *xdrs, ypresp_key_val *objp)
{
  if (!xdr_ypstat (xdrs, &objp->stat))
    return FALSE;
  if (!xdr_valdat (xdrs, &objp->val))
    return FALSE;
  return xdr_keydat (xdrs, &objp->key);
}
libnsl_hidden_def (xdr_ypresp_key_val)

bool_t
xdr_ypresp_master (XDR *xdrs, ypresp_master *objp)
{
  if (!xdr_ypstat (xdrs, &objp->stat))
    return FALSE;
  return xdr_peername (xdrs, &objp->peer);
}

bool_t
xdr_ypresp_order (XDR *xdrs, ypresp_order *objp)
{
  if (!xdr_ypstat (xdrs, &objp->stat))
    return FALSE;
  return xdr_u_int (xdrs, &objp->ordernum);
}

bool_t
xdr_ypresp_all (XDR *xdrs, ypresp_all *objp)
{
  if (!xdr_bool (xdrs, &objp->more))
    return FALSE;
  switch (objp->more)
    {
    case TRUE:
      return xdr_ypresp_key_val (xdrs, &objp->ypresp_all_u.val);
    case FALSE:
      break;
    default:
      return FALSE;
    }
  return TRUE;
}
libnsl_hidden_def (xdr_ypresp_all)

bool_t
xdr_ypresp_xfr (XDR *xdrs, ypresp_xfr *objp)
{
  if (!xdr_u_int (xdrs, &objp->transid))
    return FALSE;
  return xdr_ypxfrstat (xdrs, &objp->xfrstat);
}

bool_t
xdr_ypmaplist (XDR *xdrs, ypmaplist *objp)
{
  if (!xdr_mapname (xdrs, &objp->map))
    return FALSE;
  return xdr_pointer (xdrs, (char **) &objp->next, sizeof (ypmaplist),
		      (xdrproc_t) xdr_ypmaplist);
}

bool_t
xdr_ypresp_maplist (XDR *xdrs, ypresp_maplist *objp)
{
  if (!xdr_ypstat (xdrs, &objp->stat))
    return FALSE;
  return xdr_pointer (xdrs, (char **) &objp->maps, sizeof (ypmaplist),
		      (xdrproc_t) xdr_ypmaplist);
}

bool_t
xdr_yppush_status (XDR *xdrs, yppush_status *objp)
{
  return xdr_enum (xdrs, (enum_t *) objp);
}
libnsl_hidden_def (xdr_yppush_status)

bool_t
xdr_yppushresp_xfr (XDR *xdrs, yppushresp_xfr *objp)
{
  if (!xdr_u_int (xdrs, &objp->transid))
    return FALSE;
  return xdr_yppush_status (xdrs, &objp->status);
}

bool_t
xdr_ypbind_resptype (XDR *xdrs, ypbind_resptype *objp)
{
  return xdr_enum (xdrs, (enum_t *) objp);
}
libnsl_hidden_def (xdr_ypbind_resptype)

bool_t
xdr_ypbind_binding (XDR *xdrs, ypbind_binding *objp)
{
  if (!xdr_opaque (xdrs, objp->ypbind_binding_addr, 4))
    return FALSE;
  return xdr_opaque (xdrs, objp->ypbind_binding_port, 2);
}
libnsl_hidden_def (xdr_ypbind_binding)

bool_t
xdr_ypbind_resp (XDR *xdrs, ypbind_resp *objp)
{
  if (!xdr_ypbind_resptype (xdrs, &objp->ypbind_status))
    return FALSE;
  switch (objp->ypbind_status)
    {
    case YPBIND_FAIL_VAL:
      return xdr_u_int (xdrs, &objp->ypbind_resp_u.ypbind_error);
    case YPBIND_SUCC_VAL:
      return xdr_ypbind_binding (xdrs, &objp->ypbind_resp_u.ypbind_bindinfo);
    }
  return FALSE;
}

bool_t
xdr_ypbind_setdom (XDR *xdrs, ypbind_setdom *objp)
{
  if (!xdr_domainname (xdrs, &objp->ypsetdom_domain))
    return FALSE;
  if (!xdr_ypbind_binding (xdrs, &objp->ypsetdom_binding))
    return FALSE;
  return xdr_u_int (xdrs, &objp->ypsetdom_vers);
}

bool_t
xdr_ypall(XDR *xdrs, struct ypall_callback *incallback)
{
    struct ypresp_key_val out;
    char key[YPMAXRECORD], val[YPMAXRECORD];

    /*
     * Set up key/val struct to be used during the transaction.
     */
    memset(&out, 0, sizeof out);
    out.key.keydat_val = key;
    out.key.keydat_len = sizeof(key);
    out.val.valdat_val = val;
    out.val.valdat_len = sizeof(val);

    for (;;) {
	bool_t more, status;

	/* Values pending? */
	if (!xdr_bool(xdrs, &more))
	    return FALSE;           /* can't tell! */
	if (!more)
	    return TRUE;            /* no more */

	/* Transfer key/value pair. */
	status = xdr_ypresp_key_val(xdrs, &out);

	/*
	 * If we succeeded, call the callback function.
	 * The callback will return TRUE when it wants
	 * no more values.  If we fail, indicate the
	 * error.
	 */
	if (status) {
	    if ((*incallback->foreach)(out.stat,
				       (char *)out.key.keydat_val, out.key.keydat_len,
				       (char *)out.val.valdat_val, out.val.valdat_len,
				       incallback->data))
		return TRUE;
	} else
	    return FALSE;
    }
}
