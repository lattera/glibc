/****** rpc_clntout.c ******/

void write_stubs(void);
void printarglist(proc_list *proc, const char *result,
		  const char *addargname, const char *addargtype);

/****** rpc_cout.c ******/

void emit (definition *def);

/****** rpc_hout.c ******/

void print_datadef(definition *def);
void print_funcdef(definition *def);
void pxdrfuncdecl(const char *name, int pointerp);
void pprocdef(proc_list *proc, version_list *vp,
	      const char *addargtype, int server_p, int mode);
void pdeclaration(const char *name, declaration *dec, int tab,
		  const char *separator);
void print_xdr_func_def (char* name, int pointerp, int i);

/****** rpc_main.c ******/
	/* nil */

/****** rpc_parse.c ******/
definition *get_definition(void);

/****** rpc_sample.c ******/
void write_sample_svc(definition *def);
int write_sample_clnt(definition *def);
void add_sample_msg(void);
void write_sample_clnt_main(void);

/****** rpc_scan.c ******/
   /* see rpc_scan.h */

/****** rpc_svcout.c ******/
int nullproc(const proc_list *proc);
void write_svc_aux(int nomain);
void write_msg_out(void);

/****** rpc_tblout.c ******/
void write_tables(void);

/****** rpc_util.c ******/
void reinitialize(void);
int streq(const char *a, const char *b);
void error(const char *msg) __attribute__ ((noreturn));
void crash(void) __attribute__ ((noreturn));
void tabify(FILE *f, int tab);
char *make_argname(const char *pname, const char *vname);
void add_type(int len, const char *type);
