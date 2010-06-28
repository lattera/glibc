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

/*      @(#)rpc_util.h  1.5  90/08/29  (C) 1987 SMI   */

/*
 * rpc_util.h, Useful definitions for the RPC protocol compiler
 */

#include <stdlib.h>

#define alloc(size)		malloc((unsigned)(size))
#define ALLOC(object)   (object *) malloc(sizeof(object))

#define s_print	(void) sprintf
#define f_print (void) fprintf

struct list {
	definition *val;
	struct list *next;
};
typedef struct list list;

struct xdrfunc {
        char *name;
        int pointerp;
        struct xdrfunc *next;
};
typedef struct xdrfunc xdrfunc;

#define PUT 1
#define GET 2

/*
 * Global variables
 */
#define MAXLINESIZE 1024
extern char curline[MAXLINESIZE];
extern const char *where;
extern int linenum;

extern const char *infilename;
extern FILE *fout;
extern FILE *fin;

extern list *defined;

extern bas_type *typ_list_h;
extern bas_type *typ_list_t;
extern xdrfunc *xdrfunc_head, *xdrfunc_tail;

/*
 * All the option flags
 */
extern int inetdflag;
extern int pmflag;
extern int tblflag;
extern int logflag;
extern int newstyle;
extern int Cflag;      /* C++ flag */
extern int CCflag;     /* C++ flag */
extern int tirpcflag;  /* flag for generating tirpc code */
extern int inlineflag; /* if this is 0, then do not generate inline code */
extern int mtflag;

/*
 * Other flags related with inetd jumpstart.
 */
extern int indefinitewait;
extern int exitnow;
extern int timerflag;

extern int nonfatalerrors;

/*
 * rpc_util routines
 */
void storeval(list **lstp, definition *val);
#define STOREVAL(list,item) storeval(list,item)

definition *findval(list *lst, const char *val,
		    int (*cmp)(const definition *, const char *));
#define FINDVAL(list,item,finder) findval(list, item, finder)

const char *fixtype(const char *type);
const char *stringfix(const char *type);
char *locase(const char *str);
void pvname_svc(const char *pname, const char *vnum);
void pvname(const char *pname, const char *vnum);
void ptype(const char *prefix, const char *type, int follow);
int isvectordef(const char *type, relation rel);
int streq(const char *a, const char *b);
void error(const char *msg);
void tabify(FILE *f, int tab);
void record_open(const char *file);
bas_type *find_type(const char *type);


/*
 * rpc_cout routines
 */
void emit(definition *def);

/*
 * rpc_hout routines
 */
void print_datadef(definition *def);
void print_funcdef(definition *def);

/*
 * rpc_svcout routines
 */
void write_most(const char *infile, int netflag, int nomain);
void write_register(void);
void write_rest(void);
void write_programs(const char *storage);
void write_svc_aux(int nomain);
void write_inetd_register(const char *transp);
void write_netid_register(const char *);
void write_nettype_register(const char *);
/*
 * rpc_clntout routines
 */
void write_stubs(void);

/*
 * rpc_tblout routines
 */
void write_tables(void);
