/*      @(#)rpc_util.h  1.5  90/08/29  */

/*
 * Copyright (c) 2010, Oracle America, Inc.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *     * Neither the name of the "Oracle America, Inc." nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *   FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *   COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 *   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *   GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
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
