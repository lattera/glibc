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
#ifndef _RPCSVC_NIS_CALLBACK_H
#define _RPCSVC_NIS_CALLBACK_H 1

#include <rpc/rpc.h>
#include <rpcsvc/nis.h>

__BEGIN_DECLS

typedef nis_object *obj_p;

struct cback_data {
	struct {
		u_int entries_len;
		obj_p *entries_val;
	} entries;
};
typedef struct cback_data cback_data;

#define CB_PROG 100302
#define CB_VERS 1

#define CBPROC_RECEIVE 1
extern  bool_t * cbproc_receive_1 __P ((cback_data *, CLIENT *));
extern  bool_t * cbproc_receive_1_svc __P ((cback_data *, struct svc_req *));

#define CBPROC_FINISH 2
extern  void * cbproc_finish_1 __P ((void *, CLIENT *));
extern  void * cbproc_finish_1_svc __P ((void *, struct svc_req *));

#define CBPROC_ERROR 3
extern  void * cbproc_error_1 __P ((nis_error *, CLIENT *));
extern  void * cbproc_error_1_svc __P ((nis_error *, struct svc_req *));
extern int cb_prog_1_freeresult __P ((SVCXPRT *, xdrproc_t, caddr_t));

/* the xdr functions */

extern  bool_t xdr_obj_p __P ((XDR *, obj_p*));
extern  bool_t xdr_cback_data __P ((XDR *, cback_data*));

__END_DECLS

#endif /* !_RPCVSC_NIS_CALLBACK_H */
