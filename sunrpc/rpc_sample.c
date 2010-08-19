/*
 * From: @(#)rpc_sample.c  1.1  90/08/30
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
 * rpc_sample.c, Sample client-server code outputter for the RPC protocol compiler
 */

#include <stdio.h>
#include <string.h>
#include "rpc_parse.h"
#include "rpc_util.h"
#include "proto.h"


static const char RQSTP[] = "rqstp";

static void write_sample_client (const char *program_name, version_list * vp);
static void write_sample_server (definition * def);
static void return_type (proc_list * plist);


void
write_sample_svc (definition * def)
{

  if (def->def_kind != DEF_PROGRAM)
    return;
  write_sample_server (def);
}


int
write_sample_clnt (definition * def)
{
  version_list *vp;
  int count = 0;

  if (def->def_kind != DEF_PROGRAM)
    return 0;
  /* generate sample code for each version */
  for (vp = def->def.pr.versions; vp != NULL; vp = vp->next)
    {
      write_sample_client (def->def_name, vp);
      ++count;
    }
  return count;
}


static void
write_sample_client (const char *program_name, version_list * vp)
{
  proc_list *proc;
  int i;
  decl_list *l;

  f_print (fout, "\n\nvoid\n");
  pvname (program_name, vp->vers_num);
  if (Cflag)
    f_print (fout, "(char *host)\n{\n");
  else
    f_print (fout, "(host)\nchar *host;\n{\n");
  f_print (fout, "\tCLIENT *clnt;\n");

  i = 0;
  for (proc = vp->procs; proc != NULL; proc = proc->next)
    {
      f_print (fout, "\t");
      ++i;
      if (mtflag)
	{
	  f_print (fout, "enum clnt_stat retval_%d;\n\t", i);
	  ptype (proc->res_prefix, proc->res_type, 1);
	  if (!streq (proc->res_type, "void"))
	    f_print (fout, "result_%d;\n", i);
	  else
	    fprintf (fout, "*result_%d;\n", i);
	}
      else
	{
	  ptype (proc->res_prefix, proc->res_type, 1);
	  f_print (fout, " *result_%d;\n", i);
	}
      /* print out declarations for arguments */
      if (proc->arg_num < 2 && !newstyle)
	{
	  f_print (fout, "\t");
	  if (!streq (proc->args.decls->decl.type, "void"))
	    {
	      ptype (proc->args.decls->decl.prefix,
		     proc->args.decls->decl.type, 1);
	      f_print (fout, " ");
	    }
	  else
	    f_print (fout, "char *");	/* cannot have "void" type */
	  pvname (proc->proc_name, vp->vers_num);
	  f_print (fout, "_arg;\n");
	}
      else if (!streq (proc->args.decls->decl.type, "void"))
	{
	  for (l = proc->args.decls; l != NULL; l = l->next)
	    {
	      f_print (fout, "\t");
	      ptype (l->decl.prefix, l->decl.type, 1);
	      if (strcmp (l->decl.type, "string") == 1)
		f_print (fout, " ");
	      pvname (proc->proc_name, vp->vers_num);
	      f_print (fout, "_%s;\n", l->decl.name);
	    }
	}
    }

  /* generate creation of client handle */
  f_print(fout, "\n#ifndef\tDEBUG\n");
  f_print (fout, "\tclnt = clnt_create (host, %s, %s, \"%s\");\n",
	   program_name, vp->vers_name, tirpcflag ? "netpath" : "udp");
  f_print (fout, "\tif (clnt == NULL) {\n");
  f_print (fout, "\t\tclnt_pcreateerror (host);\n");
  f_print (fout, "\t\texit (1);\n\t}\n");
  f_print(fout, "#endif\t/* DEBUG */\n\n");

  /* generate calls to procedures */
  i = 0;
  for (proc = vp->procs; proc != NULL; proc = proc->next)
    {
      if (mtflag)
	f_print(fout, "\tretval_%d = ",++i);
      else
	f_print (fout, "\tresult_%d = ", ++i);
      pvname (proc->proc_name, vp->vers_num);
      if (proc->arg_num < 2 && !newstyle)
	{
	  f_print (fout, "(");
	  if (streq (proc->args.decls->decl.type, "void"))/* cast to void* */
	    f_print (fout, "(void*)");
	  f_print (fout, "&");
	  pvname (proc->proc_name, vp->vers_num);
	  if (mtflag)
	    f_print(fout, "_arg, &result_%d, clnt);\n", i);
	  else
	    f_print (fout, "_arg, clnt);\n");
	}
      else if (streq (proc->args.decls->decl.type, "void"))
	{
	  if (mtflag)
	    f_print (fout, "(&result_%d, clnt);\n", i);
	  else
	    f_print (fout, "(clnt);\n");
	}
      else
	{
	  f_print (fout, "(");
	  for (l = proc->args.decls; l != NULL; l = l->next)
	    {
	      pvname (proc->proc_name, vp->vers_num);
	      f_print (fout, "_%s, ", l->decl.name);
	    }
	  if (mtflag)
	    f_print(fout, "&result_%d, ", i);
	  f_print (fout, "clnt);\n");
	}
      if (mtflag)
	{
	  f_print(fout, "\tif (retval_%d != RPC_SUCCESS) {\n", i);
	}
      else
	{
	  f_print(fout, "\tif (result_%d == (", i);
	  ptype(proc->res_prefix, proc->res_type, 1);
	  f_print(fout, "*) NULL) {\n");
	}
      f_print(fout, "\t\tclnt_perror (clnt, \"call failed\");\n");
      f_print(fout, "\t}\n");
    }

  f_print (fout, "#ifndef\tDEBUG\n");
  f_print (fout, "\tclnt_destroy (clnt);\n");
  f_print (fout, "#endif\t /* DEBUG */\n");
  f_print (fout, "}\n");
}

static void
write_sample_server (definition * def)
{
  version_list *vp;
  proc_list *proc;

  for (vp = def->def.pr.versions; vp != NULL; vp = vp->next)
    {
      for (proc = vp->procs; proc != NULL; proc = proc->next)
	{
	  f_print (fout, "\n");
	  if (!mtflag)
	    {
	      return_type (proc);
	      f_print (fout, "*\n");
	    }
	  else
	    f_print (fout, "bool_t\n");
	  if (Cflag || mtflag)
	    pvname_svc (proc->proc_name, vp->vers_num);
	  else
	    pvname(proc->proc_name, vp->vers_num);
	  printarglist(proc, "result", RQSTP, "struct svc_req *");
	  f_print(fout, "{\n");
	  if (!mtflag)
	    {
	      f_print(fout, "\tstatic ");
	      if(!streq(proc->res_type, "void"))
		return_type(proc);
	      else
		f_print(fout, "char *");
				/* cannot have void type */
	      /* f_print(fout, " result;\n", proc->res_type); */
	      f_print(fout, " result;\n");
	    }
	  else
	    f_print(fout, "\tbool_t retval;\n");
	  fprintf (fout, "\n\t/*\n\t * insert server code here\n\t */\n\n");

	  if (!mtflag)
	    {
	      if (!streq(proc->res_type, "void"))
		f_print(fout, "\treturn &result;\n}\n");
	      else /* cast back to void * */
		f_print(fout, "\treturn (void *) &result;\n}\n");
	    }
	  else
	    f_print(fout, "\treturn retval;\n}\n");
	}

      /* put in sample freeing routine */
      if (mtflag)
	{
	  f_print(fout, "\nint\n");
	  pvname(def->def_name, vp->vers_num);
	  if (Cflag)
	    f_print(fout,"_freeresult (SVCXPRT *transp, xdrproc_t xdr_result, caddr_t result)\n");
	  else
	    {
	      f_print(fout,"_freeresult (transp, xdr_result, result)\n");
	      f_print(fout,"\tSVCXPRT *transp;\n");
	      f_print(fout,"\txdrproc_t xdr_result;\n");
	      f_print(fout,"\tcaddr_t result;\n");
	    }
	  f_print(fout, "{\n");
	  f_print(fout, "\txdr_free (xdr_result, result);\n");
	  f_print(fout,
		  "\n\t/*\n\t * Insert additional freeing code here, if needed\n\t */\n");
	  f_print(fout, "\n\treturn 1;\n}\n");
	}
    }
}



static void
return_type (proc_list * plist)
{
  ptype (plist->res_prefix, plist->res_type, 1);
}

void
add_sample_msg (void)
{
  f_print (fout, "/*\n");
  f_print (fout, " * This is sample code generated by rpcgen.\n");
  f_print (fout, " * These are only templates and you can use them\n");
  f_print (fout, " * as a guideline for developing your own functions.\n");
  f_print (fout, " */\n\n");
}

void
write_sample_clnt_main (void)
{
  list *l;
  definition *def;
  version_list *vp;

  f_print (fout, "\n\n");
  if (Cflag)
    f_print (fout, "int\nmain (int argc, char *argv[])\n{\n");
  else
    f_print (fout, "int\nmain (argc, argv)\nint argc;\nchar *argv[];\n{\n");

  f_print (fout, "\tchar *host;");
  f_print (fout, "\n\n\tif (argc < 2) {");
  f_print (fout, "\n\t\tprintf (\"usage: %%s server_host\\n\", argv[0]);\n");
  f_print (fout, "\t\texit (1);\n\t}");
  f_print (fout, "\n\thost = argv[1];\n");

  for (l = defined; l != NULL; l = l->next)
    {
      def = l->val;
      if (def->def_kind != DEF_PROGRAM)
	{
	  continue;
	}
      for (vp = def->def.pr.versions; vp != NULL; vp = vp->next)
	{
	  f_print (fout, "\t");
	  pvname (def->def_name, vp->vers_num);
	  f_print (fout, " (host);\n");
	}
    }
  f_print (fout, "exit (0);\n}\n");
}
