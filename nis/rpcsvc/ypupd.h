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
/* from @(#)ypupdate_prot.x	1.3 91/03/11 TIRPC 1.0 */

#ifndef __RPCSVC_YPUPD_H__
#define __RPCSVC_YPUPD_H__

#include <features.h>

#include <rpc/rpc.h>

#define MAXMAPNAMELEN 255
#define MAXYPDATALEN 1023
#define MAXERRMSGLEN 255

__BEGIN_DECLS

typedef struct {
	u_int yp_buf_len;
	char *yp_buf_val;
} yp_buf;

extern  bool_t xdr_yp_buf (XDR *, yp_buf*);

struct ypupdate_args {
	char *mapname;
	yp_buf key;
	yp_buf datum;
};
typedef struct ypupdate_args ypupdate_args;

extern  bool_t xdr_ypupdate_args (XDR *, ypupdate_args*);

struct ypdelete_args {
	char *mapname;
	yp_buf key;
};
typedef struct ypdelete_args ypdelete_args;

extern  bool_t xdr_ypdelete_args (XDR *, ypdelete_args*);

#define YPU_PROG 100028
#define YPU_VERS 1

#define YPU_CHANGE 1
extern  u_int * ypu_change_1 (ypupdate_args *, CLIENT *);
extern  u_int * ypu_change_1_svc (ypupdate_args *, struct svc_req *);
#define YPU_INSERT 2
extern  u_int * ypu_insert_1 (ypupdate_args *, CLIENT *);
extern  u_int * ypu_insert_1_svc (ypupdate_args *, struct svc_req *);
#define YPU_DELETE 3
extern  u_int * ypu_delete_1 (ypdelete_args *, CLIENT *);
extern  u_int * ypu_delete_1_svc (ypdelete_args *, struct svc_req *);
#define YPU_STORE 4
extern  u_int * ypu_store_1 (ypupdate_args *, CLIENT *);
extern  u_int * ypu_store_1_svc (ypupdate_args *, struct svc_req *);

__END_DECLS

#endif /* !__RPCSVC_YPUPD_H__ */
