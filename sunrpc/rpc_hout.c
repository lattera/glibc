/*
 * From: @(#)rpc_hout.c 1.12 89/02/22
 *
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
 */

/*
 * rpc_hout.c, Header file outputter for the RPC protocol compiler
 */
#include <stdio.h>
#include <ctype.h>
#include "rpc_parse.h"
#include "rpc_util.h"
#include "proto.h"

static void pconstdef (definition * def);
static void pargdef (definition * def);
static void pstructdef (definition * def);
static void puniondef (definition * def);
static void pdefine (const char *name, const char *num);
static int define_printed (proc_list * stop, version_list * start);
static void pprogramdef (definition * def);
static void parglist (proc_list * proc, const char *addargtype);
static void penumdef (definition * def);
static void ptypedef (definition * def);
static int undefined2 (const char *type, const char *stop);

/* store away enough information to allow the XDR functions to be spat
    out at the end of the file */

static void
storexdrfuncdecl (const char *name, int pointerp)
{
  xdrfunc * xdrptr;

  xdrptr = (xdrfunc *) malloc(sizeof (struct xdrfunc));

  xdrptr->name = (char *)name;
  xdrptr->pointerp = pointerp;
  xdrptr->next = NULL;

  if (xdrfunc_tail == NULL)
    {
      xdrfunc_head = xdrptr;
      xdrfunc_tail = xdrptr;
    }
  else
    {
      xdrfunc_tail->next = xdrptr;
      xdrfunc_tail = xdrptr;
    }
}

/*
 * Print the C-version of an xdr definition
 */
void
print_datadef (definition *def)
{

  if (def->def_kind == DEF_PROGRAM)	/* handle data only */
    return;

  if (def->def_kind != DEF_CONST)
    {
      f_print (fout, "\n");
    }
  switch (def->def_kind)
    {
    case DEF_STRUCT:
      pstructdef (def);
      break;
    case DEF_UNION:
      puniondef (def);
      break;
    case DEF_ENUM:
      penumdef (def);
      break;
    case DEF_TYPEDEF:
      ptypedef (def);
      break;
    case DEF_PROGRAM:
      pprogramdef (def);
      break;
    case DEF_CONST:
      pconstdef (def);
      break;
    }
  if (def->def_kind != DEF_PROGRAM && def->def_kind != DEF_CONST)
    {
      storexdrfuncdecl(def->def_name,
 		       def->def_kind != DEF_TYPEDEF ||
		       !isvectordef(def->def.ty.old_type,
				    def->def.ty.rel));
    }
}


void
print_funcdef (definition *def)
{
  switch (def->def_kind)
    {
    case DEF_PROGRAM:
      f_print (fout, "\n");
      pprogramdef (def);
      break;
    default:
      break;
      /* ?... shouldn't happen I guess */
    }
}

void
print_xdr_func_def (char *name, int pointerp, int i)
{
  if (i == 2)
    {
      f_print (fout, "extern bool_t xdr_%s ();\n", name);
      return;
    }
  else
    f_print(fout, "extern  bool_t xdr_%s (XDR *, %s%s);\n", name,
	    name, pointerp ? "*" : "");
}

static void
pconstdef (definition *def)
{
  pdefine (def->def_name, def->def.co);
}

/* print out the definitions for the arguments of functions in the
   header file
 */
static void
pargdef (definition * def)
{
  decl_list *l;
  version_list *vers;
  const char *name;
  proc_list *plist;

  for (vers = def->def.pr.versions; vers != NULL; vers = vers->next)
    {
      for (plist = vers->procs; plist != NULL;
	   plist = plist->next)
	{

	  if (!newstyle || plist->arg_num < 2)
	    {
	      continue;		/* old style or single args */
	    }
	  name = plist->args.argname;
	  f_print (fout, "struct %s {\n", name);
	  for (l = plist->args.decls;
	       l != NULL; l = l->next)
	    {
	      pdeclaration (name, &l->decl, 1, ";\n");
	    }
	  f_print (fout, "};\n");
	  f_print (fout, "typedef struct %s %s;\n", name, name);
	  storexdrfuncdecl (name, 1);
	  f_print (fout, "\n");
	}
    }

}

static void
pstructdef (definition *def)
{
  decl_list *l;
  const char *name = def->def_name;

  f_print (fout, "struct %s {\n", name);
  for (l = def->def.st.decls; l != NULL; l = l->next)
    {
      pdeclaration (name, &l->decl, 1, ";\n");
    }
  f_print (fout, "};\n");
  f_print (fout, "typedef struct %s %s;\n", name, name);
}

static void
puniondef (definition *def)
{
  case_list *l;
  const char *name = def->def_name;
  declaration *decl;

  f_print (fout, "struct %s {\n", name);
  decl = &def->def.un.enum_decl;
  if (streq (decl->type, "bool"))
    {
      f_print (fout, "\tbool_t %s;\n", decl->name);
    }
  else
    {
      f_print (fout, "\t%s %s;\n", decl->type, decl->name);
    }
  f_print (fout, "\tunion {\n");
  for (l = def->def.un.cases; l != NULL; l = l->next)
    {
      if (l->contflag == 0)
	pdeclaration (name, &l->case_decl, 2, ";\n");
    }
  decl = def->def.un.default_decl;
  if (decl && !streq (decl->type, "void"))
    {
      pdeclaration (name, decl, 2, ";\n");
    }
  f_print (fout, "\t} %s_u;\n", name);
  f_print (fout, "};\n");
  f_print (fout, "typedef struct %s %s;\n", name, name);
}

static void
pdefine (const char *name, const char *num)
{
  f_print (fout, "#define %s %s\n", name, num);
}

static int
define_printed (proc_list *stop, version_list *start)
{
  version_list *vers;
  proc_list *proc;

  for (vers = start; vers != NULL; vers = vers->next)
    {
      for (proc = vers->procs; proc != NULL; proc = proc->next)
	{
	  if (proc == stop)
	    {
	      return 0;
	    }
	  else if (streq (proc->proc_name, stop->proc_name))
	    {
	      return 1;
	    }
	}
    }
  abort ();
  /* NOTREACHED */
}

static void
pfreeprocdef (const char *name, const char *vers, int mode)
{
  f_print (fout, "extern int ");
  pvname (name, vers);
  if (mode == 1)
    f_print (fout,"_freeresult (SVCXPRT *, xdrproc_t, caddr_t);\n");
  else
    f_print (fout,"_freeresult ();\n");
}

static void
pprogramdef (definition *def)
{
  version_list *vers;
  proc_list *proc;
  int i;
  const char *ext;

  pargdef (def);

  pdefine (def->def_name, def->def.pr.prog_num);
  for (vers = def->def.pr.versions; vers != NULL; vers = vers->next)
    {
      if (tblflag)
	{
	  f_print (fout, "extern struct rpcgen_table %s_%s_table[];\n",
		   locase (def->def_name), vers->vers_num);
	  f_print (fout, "extern %s_%s_nproc;\n",
		   locase (def->def_name), vers->vers_num);
	}
      pdefine (vers->vers_name, vers->vers_num);

      /*
       * Print out 2 definitions, one for ANSI-C, another for
       * old K & R C
       */

      if(!Cflag)
	{
	  ext = "extern  ";
	  for (proc = vers->procs; proc != NULL;
	       proc = proc->next)
	    {
	      if (!define_printed(proc, def->def.pr.versions))
		{
		  pdefine (proc->proc_name, proc->proc_num);
		}
	      f_print (fout, "%s", ext);
	      pprocdef (proc, vers, NULL, 0, 2);

	      if (mtflag)
		{
		  f_print(fout, "%s", ext);
		  pprocdef (proc, vers, NULL, 1, 2);
		}
	    }
	  pfreeprocdef (def->def_name, vers->vers_num, 2);
	}
      else
	{
	  for (i = 1; i < 3; i++)
	    {
	      if (i == 1)
		{
		  f_print (fout, "\n#if defined(__STDC__) || defined(__cplusplus)\n");
		  ext = "extern  ";
		}
	      else
		{
		  f_print (fout, "\n#else /* K&R C */\n");
		  ext = "extern  ";
		}

	      for (proc = vers->procs; proc != NULL; proc = proc->next)
		{
		  if (!define_printed(proc, def->def.pr.versions))
		    {
		      pdefine(proc->proc_name, proc->proc_num);
		    }
		  f_print (fout, "%s", ext);
		  pprocdef (proc, vers, "CLIENT *", 0, i);
		  f_print (fout, "%s", ext);
		  pprocdef (proc, vers, "struct svc_req *", 1, i);
		}
	      pfreeprocdef (def->def_name, vers->vers_num, i);
	    }
	  f_print (fout, "#endif /* K&R C */\n");
	}
    }
}

void
pprocdef (proc_list * proc, version_list * vp,
	  const char *addargtype, int server_p, int mode)
{
  if (mtflag)
    {/* Print MT style stubs */
      if (server_p)
	f_print (fout, "bool_t ");
      else
	f_print (fout, "enum clnt_stat ");
    }
  else
    {
      ptype (proc->res_prefix, proc->res_type, 1);
      f_print (fout, "* ");
    }
  if (server_p)
    pvname_svc (proc->proc_name, vp->vers_num);
  else
    pvname (proc->proc_name, vp->vers_num);

  /*
   * mode  1 = ANSI-C, mode 2 = K&R C
   */
  if (mode == 1)
    parglist (proc, addargtype);
  else
    f_print (fout, "();\n");
}

/* print out argument list of procedure */
static void
parglist (proc_list *proc, const char *addargtype)
{
  decl_list *dl;

  f_print(fout,"(");
  if (proc->arg_num < 2 && newstyle &&
      streq (proc->args.decls->decl.type, "void"))
    {
      /* 0 argument in new style:  do nothing */
    }
  else
    {
      for (dl = proc->args.decls; dl != NULL; dl = dl->next)
	{
	  ptype (dl->decl.prefix, dl->decl.type, 1);
	  if (!newstyle)
	    f_print (fout, "*");	/* old style passes by reference */

	  f_print (fout, ", ");
	}
    }
  if (mtflag)
    {
      ptype(proc->res_prefix, proc->res_type, 1);
      f_print(fout, "*, ");
    }

  f_print (fout, "%s);\n", addargtype);
}

static void
penumdef (definition *def)
{
  const char *name = def->def_name;
  enumval_list *l;
  const char *last = NULL;
  int count = 0;

  f_print (fout, "enum %s {\n", name);
  for (l = def->def.en.vals; l != NULL; l = l->next)
    {
      f_print (fout, "\t%s", l->name);
      if (l->assignment)
	{
	  f_print (fout, " = %s", l->assignment);
	  last = l->assignment;
	  count = 1;
	}
      else
	{
	  if (last == NULL)
	    {
	      f_print (fout, " = %d", count++);
	    }
	  else
	    {
	      f_print (fout, " = %s + %d", last, count++);
	    }
	}
      f_print (fout, ",\n");
    }
  f_print (fout, "};\n");
  f_print (fout, "typedef enum %s %s;\n", name, name);
}

static void
ptypedef (definition *def)
{
  const char *name = def->def_name;
  const char *old = def->def.ty.old_type;
  char prefix[8];	  /* enough to contain "struct ", including NUL */
  relation rel = def->def.ty.rel;

  if (!streq (name, old))
    {
      if (streq (old, "string"))
	{
	  old = "char";
	  rel = REL_POINTER;
	}
      else if (streq (old, "opaque"))
	{
	  old = "char";
	}
      else if (streq (old, "bool"))
	{
	  old = "bool_t";
	}
      if (undefined2 (old, name) && def->def.ty.old_prefix)
	{
	  s_print (prefix, "%s ", def->def.ty.old_prefix);
	}
      else
	{
	  prefix[0] = 0;
	}
      f_print (fout, "typedef ");
      switch (rel)
	{
	case REL_ARRAY:
	  f_print (fout, "struct {\n");
	  f_print (fout, "\tu_int %s_len;\n", name);
	  f_print (fout, "\t%s%s *%s_val;\n", prefix, old, name);
	  f_print (fout, "} %s", name);
	  break;
	case REL_POINTER:
	  f_print (fout, "%s%s *%s", prefix, old, name);
	  break;
	case REL_VECTOR:
	  f_print (fout, "%s%s %s[%s]", prefix, old, name,
		   def->def.ty.array_max);
	  break;
	case REL_ALIAS:
	  f_print (fout, "%s%s %s", prefix, old, name);
	  break;
	}
      f_print (fout, ";\n");
    }
}

void
pdeclaration (const char *name, declaration * dec, int tab,
	      const char *separator)
{
  char buf[8];			/* enough to hold "struct ", include NUL */
  const char *prefix;
  const char *type;

  if (streq (dec->type, "void"))
    {
      return;
    }
  tabify (fout, tab);
  if (streq (dec->type, name) && !dec->prefix)
    {
      f_print (fout, "struct ");
    }
  if (streq (dec->type, "string"))
    {
      f_print (fout, "char *%s", dec->name);
    }
  else
    {
      prefix = "";
      if (streq (dec->type, "bool"))
	{
	  type = "bool_t";
	}
      else if (streq (dec->type, "opaque"))
	{
	  type = "char";
	}
      else
	{
	  if (dec->prefix)
	    {
	      s_print (buf, "%s ", dec->prefix);
	      prefix = buf;
	    }
	  type = dec->type;
	}
      switch (dec->rel)
	{
	case REL_ALIAS:
	  f_print (fout, "%s%s %s", prefix, type, dec->name);
	  break;
	case REL_VECTOR:
	  f_print (fout, "%s%s %s[%s]", prefix, type, dec->name,
		   dec->array_max);
	  break;
	case REL_POINTER:
	  f_print (fout, "%s%s *%s", prefix, type, dec->name);
	  break;
	case REL_ARRAY:
	  f_print (fout, "struct {\n");
	  tabify (fout, tab);
	  f_print (fout, "\tu_int %s_len;\n", dec->name);
	  tabify (fout, tab);
	  f_print (fout, "\t%s%s *%s_val;\n", prefix, type, dec->name);
	  tabify (fout, tab);
	  f_print (fout, "} %s", dec->name);
	  break;
	}
    }
  f_print (fout, "%s", separator);
}

static int
undefined2 (const char *type, const char *stop)
{
  list *l;
  definition *def;

  for (l = defined; l != NULL; l = l->next)
    {
      def = (definition *) l->val;
      if (def->def_kind != DEF_PROGRAM)
	{
	  if (streq (def->def_name, stop))
	    {
	      return 1;
	    }
	  else if (streq (def->def_name, type))
	    {
	      return 0;
	    }
	}
    }
  return 1;
}
