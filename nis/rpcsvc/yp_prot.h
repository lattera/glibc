/*
 * This file contains symbols and structures defining the rpc protocol
 * between the NIS clients and the NIS servers.  The servers
 * are the NIS database servers, and the NIS binders.  
 */

#ifndef _RPCSVC_YP_PROT_H
#define _RPCSVC_YP_PROT_H

#include <features.h>

#include <rpc/rpc.h>
#include <rpcsvc/ypclnt.h>

/*
 * The following procedures are supported by the protocol:
 * 
 * YPPROC_NULL() returns () takes nothing, returns nothing.  This indicates
 * that the NIS server is alive.
 * 
 * YPPROC_DOMAIN (char *) returns (bool_t) TRUE.  Indicates that the
 * responding NIS server does serve the named domain; FALSE indicates no
 * support.
 * 
 * YPPROC_DOMAIN_NONACK (char *) returns (TRUE) if the NIS server does serve
 * the named domain, otherwise does not return.  Used in the broadcast case.
 * 
 * YPPROC_MATCH (struct ypreq_key) returns (struct ypresp_val).  Returns the
 * right-hand value for a passed left-hand key, within a named map and
 * domain.
 * 
 * YPPROC_FIRST (struct ypreq_nokey) returns (struct ypresp_key_val).
 * Returns the first key-value pair from a named domain and map.
 * 
 * YPPROC_NEXT (struct ypreq_key) returns (struct ypresp_key_val).  Returns
 * the key-value pair following a passed key-value pair within a named
 * domain and map.
 *
 * YPPROC_XFR (struct ypreq_xfr) returns nothing.  Indicates to a server that
 * a map should be updated.
 *
 * YPPROC_CLEAR	takes nothing, returns nothing.  Instructs a NIS server to
 * close the current map, so that old versions of the disk file don't get
 * held open.
 * 
 * YPPROC_ALL (struct ypreq_nokey), returns
 * 	union switch (bool_t more) {
 *		TRUE:	(struct ypresp_key_val);
 *		FALSE:	(struct) {};
 *	}
 *
 * YPPROC_MASTER (struct ypreq_nokey), returns (ypresp_master)
 *
 * YPPROC_ORDER (struct ypreq_nokey), returns (ypresp_order)
 *
 * YPPROC_MAPLIST (char *), returns (struct ypmaplist *)
 */

/* Program and version symbols, magic numbers */

#define YPPROG		((u_long)100004)
#define YPVERS		((u_long)2)
#define YPVERS_ORIG	((u_long)1)
#define YPMAXRECORD	((u_long)1024)
#define YPMAXDOMAIN	((u_long)256)
#define YPMAXMAP	((u_long)64)
#define YPMAXPEER	((u_long)256)

/* byte size of a large NIS packet */
#define YPMSGSZ		1600

#ifndef DATUM
typedef struct {
	char	*dptr;
	int	dsize;
} datum;
#define DATUM
#endif

struct ypmap_parms {
  char *domain;			/* Null string means not available */
  char *map;			/* Null string means not available */
  unsigned long int ordernum;	/* 0 means not available */
  char *owner;			/* Null string means not available */
};

/*
 * Request parameter structures
 */

struct ypreq_key {
  char *domain;
  char *map;
  datum keydat;
};

struct ypreq_nokey {
  char *domain;
  char *map;
};

struct ypreq_xfr {
  struct ypmap_parms map_parms;
  unsigned long transid;
  unsigned long proto;
  unsigned short port;
};

struct ypreq_newxfr {
  struct ypmap_parms map_parms;
  unsigned long transid;
  unsigned long proto;
  char *name;
};

#define ypxfr_domain map_parms.domain
#define ypxfr_map map_parms.map
#define ypxfr_ordernum map_parms.ordernum
#define ypxfr_owner map_parms.owner

/*
 * Response parameter structures
 */

struct ypresp_val {
  long unsigned status;
  datum valdat;
};

struct ypresp_key_val {
  long unsigned status;
  datum keydat;
  datum valdat;
};

struct ypresp_master {
  long unsigned status;
  char *master;
};

struct ypresp_order {
  long unsigned status;
  unsigned long int ordernum;
};

struct ypmaplist {
  char ypml_name[YPMAXMAP + 1];
  struct ypmaplist *ypml_next;
};

struct ypresp_maplist {
  long unsigned status;
  struct ypmaplist *list;
};

/*
 * Procedure symbols.  YPPROC_NULL, YPPROC_DOMAIN, and YPPROC_DOMAIN_NONACK
 * must keep the same values (0, 1, and 2) that they had in the first version
 * of the protocol.
 */

#define YPPROC_NULL	((u_long)0)
#define YPPROC_DOMAIN	((u_long)1)
#define YPPROC_DOMAIN_NONACK ((u_long)2)
#define YPPROC_MATCH	((u_long)3)
#define YPPROC_FIRST	((u_long)4)
#define YPPROC_NEXT	((u_long)5)
#define YPPROC_XFR	((u_long)6)
#define YPPROC_CLEAR	((u_long)7)
#define YPPROC_ALL	((u_long)8)
#define YPPROC_MASTER	((u_long)9)
#define YPPROC_ORDER	((u_long)10)
#define YPPROC_MAPLIST	((u_long)11)
#define	YPPROC_NEWXFR	((u_long)12)

/* Return status values */

#define YP_TRUE	 	((long)1)	/* General purpose success code */
#define YP_NOMORE 	((long)2)	/* No more entries in map */
#define YP_FALSE 	((long)0)	/* General purpose failure code */
#define YP_NOMAP 	((long)-1)	/* No such map in domain */
#define YP_NODOM 	((long)-2)	/* Domain not supported */
#define YP_NOKEY 	((long)-3)	/* No such key in map */
#define YP_BADOP 	((long)-4)	/* Invalid operation */
#define YP_BADDB 	((long)-5)	/* Server data base is bad */
#define YP_YPERR 	((long)-6)	/* NIS server error */
#define YP_BADARGS 	((long)-7)	/* Request arguments bad */
#define YP_VERS		((long)-8)	/* NIS server version mismatch - server
					 *   can't supply requested service. */
/*
 *		Protocol between clients and NIS binder servers
 */

/*
 * The following procedures are supported by the protocol:
 *
 * YPBINDPROC_NULL() returns ()
 * 	takes nothing, returns nothing
 *
 * YPBINDPROC_DOMAIN takes (char *) returns (struct ypbind_resp)
 *
 * YPBINDPROC_SETDOM takes (struct ypbind_setdom) returns nothing
 */
 
/* Program and version symbols, magic numbers */

#define YPBINDPROG		((u_long)100007)
#define YPBINDVERS		((u_long)2)
#define YPBINDVERS_ORIG		((u_long)1)

/* Procedure symbols */

#define YPBINDPROC_NULL		((u_long)0)
#define YPBINDPROC_DOMAIN	((u_long)1)
#define YPBINDPROC_SETDOM	((u_long)2)
/*
 * Response structure and overall result status codes.  Success and failure
 * represent two separate response message types.
 */

enum ypbind_resptype {YPBIND_SUCC_VAL = 1, YPBIND_FAIL_VAL = 2};

struct ypbind_binding {
  struct in_addr ypbind_binding_addr;	        /* In network order */
  unsigned short int ypbind_binding_port;	/* In network order */
};

struct ypbind_resp {
  enum ypbind_resptype ypbind_status;
  union {
    unsigned long ypbind_error;
    struct ypbind_binding ypbind_bindinfo;
  } ypbind_respbody;
};


/* Detailed failure reason codes for response field ypbind_error*/

#define YPBIND_ERR_ERR 1		/* Internal error */
#define YPBIND_ERR_NOSERV 2		/* No bound server for passed domain */
#define YPBIND_ERR_RESC 3		/* System resource allocation failure */

/*
 * Request data structure for ypbind "Set domain" procedure.
 */
struct ypbind_setdom {
  char ypsetdom_domain[YPMAXDOMAIN + 1];
  struct ypbind_binding ypsetdom_binding;
  unsigned short ypsetdom_vers;
};
#define ypsetdom_addr ypsetdom_binding.ypbind_binding_addr
#define ypsetdom_port ypsetdom_binding.ypbind_binding_port

/*
 *		Protocol between clients (ypxfr, only) and yppush
 *		yppush speaks a protocol in the transient range, which
 *		is supplied to ypxfr as a command-line parameter when it
 *		is activated by ypserv.
 */
#define YPPUSHVERS		((u_long) 1)
#define YPPUSHVERS_ORIG		((u_long)1)

/* Procedure symbols */

#define YPPUSHPROC_NULL		((u_long)0)
#define YPPUSHPROC_XFRRESP	((u_long)1)

struct yppushresp_xfr {
  unsigned long transid;
  unsigned long status;
};

/* Status values for yppushresp_xfr.status */

#define YPPUSH_SUCC	((long)1)	/* Success */
#define YPPUSH_AGE	((long)2)	/* Master's version not newer */
#define YPPUSH_NOMAP 	((long)-1)	/* Can't find server for map */
#define YPPUSH_NODOM 	((long)-2)	/* Domain not supported */
#define YPPUSH_RSRC 	((long)-3)	/* Local resouce alloc failure */
#define YPPUSH_RPC 	((long)-4)	/* RPC failure talking to server */
#define YPPUSH_MADDR	((long)-5)	/* Can't get master address */
#define YPPUSH_YPERR 	((long)-6)	/* NIS server/map db error */
#define YPPUSH_BADARGS 	((long)-7)	/* Request arguments bad */
#define YPPUSH_DBM	((long)-8)	/* Local dbm operation failed */
#define YPPUSH_FILE	((long)-9)	/* Local file I/O operation failed */
#define YPPUSH_SKEW	((long)-10)	/* Map version skew during transfer */
#define YPPUSH_CLEAR	((long)-11)	/* Can't send "Clear" req to local
					 *   ypserv */
#define YPPUSH_FORCE	((long)-12)	/* No local order number in map -
					 *   use -f flag. */
#define YPPUSH_XFRERR	((long)-13)	/* ypxfr error */
#define YPPUSH_REFUSED	((long)-14)	/* Transfer request refused by ypserv */
#define	YPPUSH_NOALIAS	((long)-15)	/* Alias not found for map or domain */

struct ypresp_all {
  bool_t more;
  union {
    struct ypresp_key_val val;
  } ypresp_all_u;
};

__BEGIN_DECLS

extern bool_t xdr_datum __P ((XDR *__xdrs, datum * __objp));
extern bool_t xdr_ypdomain_wrap_string __P ((XDR *__xdrs, char ** __objp));
extern bool_t xdr_ypmap_wrap_string __P ((XDR *__xdrs, char ** __objp));
extern bool_t xdr_ypreq_key __P ((XDR *__xdrs, struct ypreq_key * __objp));
extern bool_t xdr_ypreq_nokey __P ((XDR *__xdrs, struct ypreq_nokey * __objp));
extern bool_t xdr_ypreq_xfr __P ((XDR *__xdrs, struct ypreq_xfr * __objp));
extern bool_t xdr_ypreq_newxfr __P ((XDR *__xdrs, struct ypreq_newxfr * __objp));
extern bool_t xdr_ypresp_val __P ((XDR *__xdrs, struct ypresp_val * __objp));
extern bool_t xdr_ypresp_key_val __P ((XDR *__xdrs, struct ypresp_key_val * __objp));
extern bool_t xdr_ypbind_resp __P ((XDR *__xdrs, struct ypbind_resp * __objp));
extern bool_t xdr_ypbind_setdom __P ((XDR *__xdrs, struct ypbind_setdom * __objp));
extern bool_t xdr_ypmap_parms __P ((XDR *__xdrs, struct ypmap_parms * __objp));
extern bool_t xdr_ypowner_wrap_string __P ((XDR *__xdrs, char ** __objp));
extern bool_t xdr_yppushresp_xfr __P ((XDR *__xdrs, struct yppushresp_xfr * __objp));
extern bool_t xdr_ypresp_order __P ((XDR *__xdrs, struct ypresp_order  * __objp));
extern bool_t xdr_ypresp_master __P ((XDR *__xdrs, struct ypresp_master * __objp));
extern bool_t xdr_ypall __P ((XDR *__xdrs, struct ypall_callback * __objp));
extern bool_t xdr_ypresp_maplist __P ((XDR *__xdrs, struct ypresp_maplist * __objp));

extern bool_t xdr_domainname_ypbind __P ((XDR *__xdrs, char * __objp));
extern bool_t xdr_ypbind_binding __P ((XDR *__xdrs, struct ypbind_binding * __objp));
extern bool_t xdr_ypbind_resptype __P ((XDR *__xdrs, enum ypbind_resptype * __objp));
extern bool_t xdr_ypstat __P ((XDR *__xdrs, enum ypbind_resptype * __objp));
extern bool_t xdr_ypresp_all __P ((XDR *__xdrs, struct ypresp_all  * __objp));
extern bool_t xdr_ypresp_all_seq __P ((XDR *__xdrs, u_long * __objp));
extern bool_t xdr_ypmaplist_str __P ((XDR *__xdrs, char * __objp));
extern bool_t xdr_ypmaplist __P ((XDR *__xdrs, struct ypmaplist * __objp));

__END_DECLS

#endif	/* _RPCSVC_YP_PROT_H */
