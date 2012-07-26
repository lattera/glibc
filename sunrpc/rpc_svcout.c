/*
 * From: @(#)rpc_svcout.c 1.29 89/03/30
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
 * rpc_svcout.c, Server-skeleton outputter for the RPC protocol compiler
 */
#include <stdio.h>
#include <string.h>
#include "rpc_parse.h"
#include "rpc_util.h"
#include "proto.h"

static const char RQSTP[] = "rqstp";
static const char TRANSP[] = "transp";
static const char ARG[] = "argument";
static const char RESULT[] = "result";
static const char ROUTINE[] = "local";
static char RETVAL[] = "retval";

char _errbuf[256];		/* For all messages */

static void internal_proctype (const proc_list * plist);
static void p_xdrfunc (const char *rname, const char *typename);
static void write_real_program (const definition * def);
static void write_program (const definition * def, const char *storage);
static void printerr (const char *err, const char *transp);
static void printif (const char *proc, const char *transp, const char *arg);
static void write_inetmost (const char *infile);
static void print_return (const char *space);
static void print_pmapunset (const char *space);
static void print_err_message (const char *space);
static void write_timeout_func (void);
static void write_pm_most (const char *infile, int netflag);
static void write_rpc_svc_fg (const char *infile, const char *sp);
static void open_log_file (const char *infile, const char *sp);

static void
p_xdrfunc (const char *rname, const char *typename)
{
  if (Cflag)
    f_print (fout, "\t\t_xdr_%s = (xdrproc_t) xdr_%s;\n", rname,
	     stringfix (typename));
  else
    f_print (fout, "\t\t_xdr_%s = xdr_%s;\n", rname, stringfix (typename));
}

void
internal_proctype (const proc_list * plist)
{
  f_print (fout, "static ");
  ptype (plist->res_prefix, plist->res_type, 1);
  f_print (fout, "*");
}


/*
 * write most of the service, that is, everything but the registrations.
 */
void
write_most (const char *infile /* our name */ , int netflag, int nomain)
{
  if (inetdflag || pmflag)
    {
      const char *var_type;
      /* WHY? */
      var_type = (nomain ? "extern" : "");
      f_print (fout, "%s int _rpcpmstart;", var_type);
      f_print (fout, "\t\t/* Started by a port monitor ? */\n");
      if (!tirpcflag)
	{
	  f_print (fout, "%s int _rpcfdtype;", var_type);
	  f_print (fout, "\t\t/* Whether Stream or Datagram ? */\n");
	}
      if (timerflag)
	{
#if 0
	  f_print (fout, "%s int _rpcsvcdirty;", var_type);
	  f_print (fout, "\t/* Still serving ? */\n");
#else
	  f_print(fout, " /* States a server can be in wrt request */\n\n");
	  f_print(fout, "#define\t_IDLE 0\n");
	  f_print(fout, "#define\t_SERVED 1\n");
	  f_print(fout, "#define\t_SERVING 2\n\n");
	  f_print(fout, "static int _rpcsvcstate = _IDLE;");
	  f_print(fout, "\t /* Set when a request is serviced */\n");

	  if (mtflag)
	    {
	      f_print (fout, "mutex_t _svcstate_lock;");
	      f_print (fout,
		       "\t\t\t/* Mutex lock for variable_rpcsvcstate */\n");
	    }
#endif
	}
      write_svc_aux (nomain);
    }
  /* write out dispatcher and stubs */
  write_programs (nomain ? NULL : "static");

  if (nomain)
    return;

  if (Cflag)
    f_print (fout, "\nint\nmain (int argc, char **argv)\n");
  else
    {
      f_print (fout, "\nint\nmain (argc, argv)\n");
      f_print (fout, "\tint argc;\n");
      f_print (fout, "\tchar **argv;\n");
    }
  f_print (fout, "{\n");
  if (inetdflag)
    {
      write_inetmost (infile);	/* Includes call to write_rpc_svc_fg() */
    }
  else
    {
      if (tirpcflag)
	{
	  if (netflag)
	    {
	      f_print (fout, "\tregister SVCXPRT *%s;\n", TRANSP);
	      f_print (fout, "\tstruct netconfig *nconf = NULL;\n");
	    }
	  f_print (fout, "\tpid_t pid;\n");
	  f_print (fout, "\tint i;\n");
	  f_print (fout, "\tchar mname[FMNAMESZ + 1];\n\n");

	  if (mtflag & timerflag)
	    f_print (fout,
		     "\tmutex_init (&_svcstate_lock, USYNC_THREAD, NULL);\n");

 	  write_pm_most (infile, netflag);
	  f_print (fout, "\telse {\n");
	  write_rpc_svc_fg (infile, "\t\t");
	  f_print (fout, "\t}\n");
	}
      else
	{
	  f_print (fout, "\tregister SVCXPRT *%s;\n", TRANSP);
	  f_print (fout, "\n");
	  print_pmapunset ("\t");
	}
    }

  if (logflag && !inetdflag)
    {
      open_log_file (infile, "\t");
    }
}

/*
 * write a registration for the given transport
 */
void
write_netid_register (const char *transp)
{
  list *l;
  definition *def;
  version_list *vp;
  const char *sp;
  char tmpbuf[32];

  sp = "";
  f_print (fout, "\n");
  f_print (fout, "%s\tnconf = getnetconfigent (\"%s\");\n", sp, transp);
  f_print (fout, "%s\tif (nconf == NULL) {\n", sp);
  (void) sprintf (_errbuf, "cannot find %s netid.", transp);
  sprintf (tmpbuf, "%s\t\t", sp);
  print_err_message (tmpbuf);
  f_print (fout, "%s\t\texit (1);\n", sp);
  f_print (fout, "%s\t}\n", sp);
  f_print (fout, "%s\t%s = svc_tli_create (RPC_ANYFD, nconf, 0, 0, 0);\n",
	   sp, TRANSP /*, transp *//* ?!?... */ );
  f_print (fout, "%s\tif (%s == NULL) {\n", sp, TRANSP);
  sprintf (_errbuf, "cannot create %s service.", transp);
  print_err_message (tmpbuf);
  f_print (fout, "%s\t\texit (1);\n", sp);
  f_print (fout, "%s\t}\n", sp);

  for (l = defined; l != NULL; l = l->next)
    {
      def = (definition *) l->val;
      if (def->def_kind != DEF_PROGRAM)
	{
	  continue;
	}
      for (vp = def->def.pr.versions; vp != NULL; vp = vp->next)
	{
	  f_print (fout, "%s\t(void) rpcb_unset (%s, %s, nconf);\n",
		   sp, def->def_name, vp->vers_name);
	  f_print (fout, "%s\tif (!svc_reg (%s, %s, %s, ",
		   sp, TRANSP, def->def_name, vp->vers_name);
	  pvname (def->def_name, vp->vers_num);
	  f_print (fout, ", nconf)) {\n");
	  (void) sprintf (_errbuf, "unable to register (%s, %s, %s).",
			  def->def_name, vp->vers_name, transp);
	  print_err_message (tmpbuf);
	  f_print (fout, "%s\t\texit (1);\n", sp);
	  f_print (fout, "%s\t}\n", sp);
	}
    }
  f_print (fout, "%s\tfreenetconfigent (nconf);\n", sp);
}

/*
 * write a registration for the given transport for TLI
 */
void
write_nettype_register (const char *transp)
{
  list *l;
  definition *def;
  version_list *vp;

  for (l = defined; l != NULL; l = l->next)
    {
      def = (definition *) l->val;
      if (def->def_kind != DEF_PROGRAM)
	{
	  continue;
	}
      for (vp = def->def.pr.versions; vp != NULL; vp = vp->next)
	{
	  f_print (fout, "\tif (!svc_create (");
	  pvname (def->def_name, vp->vers_num);
	  f_print (fout, ", %s, %s, \"%s\")) {\n ",
		   def->def_name, vp->vers_name, transp);
	  (void) sprintf (_errbuf,
			  "unable to create (%s, %s) for %s.",
			  def->def_name, vp->vers_name, transp);
	  print_err_message ("\t\t");
	  f_print (fout, "\t\texit (1);\n");
	  f_print (fout, "\t}\n");
	}
    }
}

/*
 * write the rest of the service
 */
void
write_rest (void)
{
  f_print (fout, "\n");
  if (inetdflag)
    {
      f_print (fout, "\tif (%s == (SVCXPRT *)NULL) {\n", TRANSP);
      (void) sprintf (_errbuf, "could not create a handle");
      print_err_message ("\t\t");
      f_print (fout, "\t\texit (1);\n");
      f_print (fout, "\t}\n");
      if (timerflag)
	{
	  f_print (fout, "\tif (_rpcpmstart) {\n");
	  f_print (fout,
		   "\t\t(void) signal (SIGALRM, %s closedown);\n",
		   Cflag ? "(SIG_PF)" : "(void(*)())");
	  f_print (fout, "\t\t(void) alarm (_RPCSVC_CLOSEDOWN);\n");
	  f_print (fout, "\t}\n");
	}
    }
  f_print (fout, "\tsvc_run ();\n");
  (void) sprintf (_errbuf, "svc_run returned");
  print_err_message ("\t");
  f_print (fout, "\texit (1);\n");
  f_print (fout, "\t/* NOTREACHED */\n");
  f_print (fout, "}\n");
}

void
write_programs (const char *storage)
{
  list *l;
  definition *def;

  /* write out stubs for procedure  definitions */
  for (l = defined; l != NULL; l = l->next)
    {
      def = (definition *) l->val;
      if (def->def_kind == DEF_PROGRAM)
	{
	  write_real_program (def);
	}
    }

  /* write out dispatcher for each program */
  for (l = defined; l != NULL; l = l->next)
    {
      def = (definition *) l->val;
      if (def->def_kind == DEF_PROGRAM)
	{
	  write_program (def, storage);
	}
    }
}

/* write out definition of internal function (e.g. _printmsg_1(...))
   which calls server's defintion of actual function (e.g. printmsg_1(...)).
   Unpacks single user argument of printmsg_1 to call-by-value format
   expected by printmsg_1. */
static void
write_real_program (const definition * def)
{
  version_list *vp;
  proc_list *proc;
  decl_list *l;

  if (!newstyle)
    return;			/* not needed for old style */
  for (vp = def->def.pr.versions; vp != NULL; vp = vp->next)
    {
      for (proc = vp->procs; proc != NULL; proc = proc->next)
	{
	  fprintf (fout, "\n");
	  if (!mtflag)
	    internal_proctype (proc);
	  else
	    f_print (fout, "int");
	  f_print (fout, "\n_");
	  pvname (proc->proc_name, vp->vers_num);
	  if (Cflag)
	    {
	      f_print (fout, " (");
	      /* arg name */
	      if (proc->arg_num > 1)
		f_print (fout, "%s", proc->args.argname);
	      else
		ptype (proc->args.decls->decl.prefix,
		       proc->args.decls->decl.type, 0);
	      if (mtflag)
		{
		  f_print(fout, " *argp, void *%s, struct svc_req *%s)\n",
			  RESULT, RQSTP);
		}
	      else
		f_print (fout, " *argp, struct svc_req *%s)\n",
			 RQSTP);
	    }
	  else
	    {
	      if (mtflag)
		f_print(fout, " (argp, %s, %s)\n", RESULT, RQSTP);
	      else
		f_print (fout, " (argp, %s)\n", RQSTP);
	      /* arg name */
	      if (proc->arg_num > 1)
		f_print (fout, "\t%s *argp;\n", proc->args.argname);
	      else
		{
		  f_print (fout, "\t");
		  ptype (proc->args.decls->decl.prefix,
			 proc->args.decls->decl.type, 0);
		  f_print (fout, " *argp;\n");
		}
	      f_print (fout, "	struct svc_req *%s;\n", RQSTP);
	    }

	  f_print (fout, "{\n");
	  f_print (fout, "\treturn (");
	  if (Cflag || mtflag)
	    pvname_svc (proc->proc_name, vp->vers_num);
	  else
	    pvname (proc->proc_name, vp->vers_num);
	  f_print (fout, "(");
	  if (proc->arg_num < 2)
	    {			/* single argument */
	      if (!streq (proc->args.decls->decl.type, "void"))
		f_print (fout, "*argp, ");	/* non-void */
	    }
	  else
	    {
	      for (l = proc->args.decls; l != NULL; l = l->next)
		f_print (fout, "argp->%s, ", l->decl.name);
	    }
	  if (mtflag)
	    f_print (fout, "%s, ", RESULT);
	  f_print (fout, "%s));\n}\n", RQSTP);
	}
    }
}

static void
write_program (const definition * def, const char *storage)
{
  version_list *vp;
  proc_list *proc;
  int filled;

  for (vp = def->def.pr.versions; vp != NULL; vp = vp->next)
    {
      f_print (fout, "\n");
      if (storage != NULL)
	{
	  f_print (fout, "%s ", storage);
	}
      f_print (fout, "void\n");
      pvname (def->def_name, vp->vers_num);

      if (Cflag)
	{
	  f_print (fout, "(struct svc_req *%s, ", RQSTP);
	  f_print (fout, "register SVCXPRT *%s)\n", TRANSP);
	}
      else
	{
	  f_print (fout, "(%s, %s)\n", RQSTP, TRANSP);
	  f_print (fout, "	struct svc_req *%s;\n", RQSTP);
	  f_print (fout, "	register SVCXPRT *%s;\n", TRANSP);
	}

      f_print (fout, "{\n");

      filled = 0;
      f_print (fout, "\tunion {\n");
      for (proc = vp->procs; proc != NULL; proc = proc->next)
	{
	  if (proc->arg_num < 2)
	    {			/* single argument */
	      if (streq (proc->args.decls->decl.type,
			 "void"))
		{
		  continue;
		}
	      filled = 1;
	      f_print (fout, "\t\t");
	      ptype (proc->args.decls->decl.prefix,
		     proc->args.decls->decl.type, 0);
	      pvname (proc->proc_name, vp->vers_num);
	      f_print (fout, "_arg;\n");

	    }
	  else
	    {
	      filled = 1;
	      f_print (fout, "\t\t%s", proc->args.argname);
	      f_print (fout, " ");
	      pvname (proc->proc_name, vp->vers_num);
	      f_print (fout, "_arg;\n");
	    }
	}
      if (!filled)
	{
	  f_print (fout, "\t\tint fill;\n");
	}
      f_print (fout, "\t} %s;\n", ARG);
      if (mtflag)
	{
	  f_print(fout, "\tunion {\n");
	  for (proc = vp->procs; proc != NULL; proc = proc->next)
	    if (!streq (proc->res_type, "void"))
	      {
		f_print(fout, "\t\t");
		ptype(proc->res_prefix, proc->res_type, 0);
		pvname(proc->proc_name, vp->vers_num);
		f_print(fout, "_res;\n");
	      }
	  f_print(fout, "\t} %s;\n", RESULT);
	  f_print(fout, "\tbool_t %s;\n", RETVAL);

	} else
	  f_print (fout, "\tchar *%s;\n", RESULT);

      if (Cflag)
	{
	  f_print (fout, "\txdrproc_t _xdr_%s, _xdr_%s;\n", ARG, RESULT);
	  if (mtflag)
	    f_print(fout,
		    "\tbool_t (*%s)(char *, void *, struct svc_req *);\n",
		    ROUTINE);
	  else
	    f_print (fout, "\tchar *(*%s)(char *, struct svc_req *);\n",
		     ROUTINE);
	}
      else
	{
	  f_print (fout, "\tbool_t (*_xdr_%s)(), (*_xdr_%s)();\n", ARG, RESULT);
	  if (mtflag)
	    f_print(fout, "\tbool_t (*%s)();\n", ROUTINE);
	  else
	    f_print (fout, "\tchar *(*%s)();\n", ROUTINE);
	}
      f_print (fout, "\n");

      if (timerflag)
#if 0
	f_print (fout, "\t_rpcsvcdirty = 1;\n");
#else
      {
	if (mtflag)
	  f_print(fout, "\tmutex_lock(&_svcstate_lock);\n");
	f_print(fout, "\t_rpcsvcstate = _SERVING;\n");
	if (mtflag)
	  f_print(fout, "\tmutex_unlock(&_svcstate_lock);\n");
      }
#endif

      f_print (fout, "\tswitch (%s->rq_proc) {\n", RQSTP);
      if (!nullproc (vp->procs))
	{
	  f_print (fout, "\tcase NULLPROC:\n");
	  f_print (fout,
		   "\t\t(void) svc_sendreply (%s, (xdrproc_t) xdr_void, (char *)NULL);\n",
		   TRANSP);
	  print_return ("\t\t");
	  f_print (fout, "\n");
	}
      for (proc = vp->procs; proc != NULL; proc = proc->next)
	{
	  f_print (fout, "\tcase %s:\n", proc->proc_name);
	  if (proc->arg_num < 2)
	    {			/* single argument */
	      p_xdrfunc (ARG, proc->args.decls->decl.type);
	    }
	  else
	    {
	      p_xdrfunc (ARG, proc->args.argname);
	    }
	  p_xdrfunc (RESULT, proc->res_type);
	  if (Cflag)
	    {
	      if (mtflag)
		f_print(fout,
			"\t\t%s = (bool_t (*) (char *, void *,  struct svc_req *))",
			ROUTINE);
	      else
		f_print (fout,
			 "\t\t%s = (char *(*)(char *, struct svc_req *)) ",
			 ROUTINE);
	    }
	  else
	    if (mtflag)
	      f_print(fout, "\t\t%s = (bool_t (*)()) ", ROUTINE);
	    else
	      f_print (fout, "\t\t%s = (char *(*)()) ", ROUTINE);

	  if (newstyle)
	    {			/* new style: calls internal routine */
	      f_print (fout, "_");
	    }
	  if ((Cflag || mtflag) && !newstyle)
	    pvname_svc (proc->proc_name, vp->vers_num);
	  else
	    pvname (proc->proc_name, vp->vers_num);
	  f_print (fout, ";\n");
	  f_print (fout, "\t\tbreak;\n\n");
	}
      f_print (fout, "\tdefault:\n");
      printerr ("noproc", TRANSP);
      print_return ("\t\t");
      f_print (fout, "\t}\n");

      f_print (fout, "\tmemset ((char *)&%s, 0, sizeof (%s));\n", ARG, ARG);
      printif ("getargs", TRANSP, ARG);
      printerr ("decode", TRANSP);
      print_return ("\t\t");
      f_print (fout, "\t}\n");

      if (!mtflag)
	{
	  if (Cflag)
	    f_print (fout, "\t%s = (*%s)((char *)&%s, %s);\n",
		     RESULT, ROUTINE, ARG, RQSTP);
	  else
	    f_print (fout, "\t%s = (*%s)(&%s, %s);\n",
		     RESULT, ROUTINE, ARG, RQSTP);
	}
      else
	if (Cflag)
	  f_print(fout, "\t%s = (bool_t) (*%s)((char *)&%s, (void *)&%s, %s);\n",
		  RETVAL, ROUTINE, ARG, RESULT, RQSTP);
	else
	  f_print(fout, "\t%s = (bool_t) (*%s)(&%s, &%s, %s);\n",
		  RETVAL, ROUTINE, ARG, RESULT, RQSTP);
      if (mtflag)
	f_print(fout,
		"\tif (%s > 0 && !svc_sendreply(%s, (xdrproc_t) _xdr_%s, (char *)&%s)) {\n",
		RETVAL, TRANSP, RESULT, RESULT);
      else
	f_print(fout,
		"\tif (%s != NULL && !svc_sendreply(%s, (xdrproc_t) _xdr_%s, %s)) {\n",
		RESULT, TRANSP, RESULT, RESULT);

      printerr ("systemerr", TRANSP);
      f_print (fout, "\t}\n");

      printif ("freeargs", TRANSP, ARG);

      sprintf (_errbuf, "unable to free arguments");
      print_err_message ("\t\t");
      f_print (fout, "\t\texit (1);\n");
      f_print (fout, "\t}\n");
      /* print out free routine */
      if (mtflag)
	{
	  f_print(fout,"\tif (!");
	  pvname(def->def_name, vp->vers_num);
	  f_print(fout,"_freeresult (%s, _xdr_%s, (caddr_t) &%s))\n",
		  TRANSP, RESULT, RESULT);
	  (void) sprintf(_errbuf, "unable to free results");
	  print_err_message("\t\t");
	  f_print(fout, "\n");
	}
      print_return ("\t");
      f_print (fout, "}\n");
    }
}

static void
printerr (const char *err, const char *transp)
{
  f_print (fout, "\t\tsvcerr_%s (%s);\n", err, transp);
}

static void
printif (const char *proc, const char *transp, const char *arg)
{
  f_print (fout, "\tif (!svc_%s (%s, (xdrproc_t) _xdr_%s, (caddr_t) &%s)) {\n",
	   proc, transp, arg, arg);
}

int
nullproc (const proc_list * proc)
{
  for (; proc != NULL; proc = proc->next)
    {
      if (streq (proc->proc_num, "0"))
	{
	  return 1;
	}
    }
  return 0;
}

static void
write_inetmost (const char *infile)
{
  f_print (fout, "\tregister SVCXPRT *%s;\n", TRANSP);
  f_print (fout, "\tint sock;\n");
  f_print (fout, "\tint proto;\n");
  f_print (fout, "\tstruct sockaddr_in saddr;\n");
  f_print (fout, "\tint asize = sizeof (saddr);\n");
  f_print (fout, "\n");
  f_print (fout,
       "\tif (getsockname (0, (struct sockaddr *)&saddr, &asize) == 0) {\n");
  f_print (fout, "\t\tint ssize = sizeof (int);\n\n");
  f_print (fout, "\t\tif (saddr.sin_family != AF_INET)\n");
  f_print (fout, "\t\t\texit (1);\n");
  f_print (fout, "\t\tif (getsockopt (0, SOL_SOCKET, SO_TYPE,\n");
  f_print (fout, "\t\t\t\t(char *)&_rpcfdtype, &ssize) == -1)\n");
  f_print (fout, "\t\t\texit (1);\n");
  f_print (fout, "\t\tsock = 0;\n");
  f_print (fout, "\t\t_rpcpmstart = 1;\n");
  f_print (fout, "\t\tproto = 0;\n");
  open_log_file (infile, "\t\t");
  f_print (fout, "\t} else {\n");
  write_rpc_svc_fg (infile, "\t\t");
  f_print (fout, "\t\tsock = RPC_ANYSOCK;\n");
  print_pmapunset ("\t\t");
  f_print (fout, "\t}\n");
}

static void
print_return (const char *space)
{
  if (exitnow)
    f_print (fout, "%sexit (0);\n", space);
  else
    {
      if (timerflag)
	{
#if 0
	  f_print (fout, "%s_rpcsvcdirty = 0;\n", space);
#else
	  if (mtflag)
	    f_print(fout, "%smutex_lock(&_svcstate_lock);\n", space);
	  f_print(fout, "%s_rpcsvcstate = _SERVED;\n", space);
	  if (mtflag)
	    f_print(fout, "%smutex_unlock(&_svcstate_lock);\n", space);
#endif
	}
      f_print (fout, "%sreturn;\n", space);
    }
}

static void
print_pmapunset (const char *space)
{
  list *l;
  definition *def;
  version_list *vp;

  for (l = defined; l != NULL; l = l->next)
    {
      def = (definition *) l->val;
      if (def->def_kind == DEF_PROGRAM)
	{
	  for (vp = def->def.pr.versions; vp != NULL;
	       vp = vp->next)
	    {
	      f_print (fout, "%spmap_unset (%s, %s);\n",
		       space, def->def_name, vp->vers_name);
	    }
	}
    }
}

static void
print_err_message (const char *space)
{
  if (logflag)
    f_print (fout, "%ssyslog (LOG_ERR, \"%%s\", \"%s\");\n", space, _errbuf);
  else if (inetdflag || pmflag)
    f_print (fout, "%s_msgout (\"%s\");\n", space, _errbuf);
  else
    f_print (fout, "%sfprintf (stderr, \"%%s\", \"%s\");\n", space, _errbuf);
}

/*
 * Write the server auxiliary function ( _msgout, timeout)
 */
void
write_svc_aux (int nomain)
{
  if (!logflag)
    write_msg_out ();
  if (!nomain)
    write_timeout_func ();
}

/*
 * Write the _msgout function
 */

void
write_msg_out (void)
{
  f_print (fout, "\n");
  f_print (fout, "static\n");
  if (!Cflag)
    {
      f_print (fout, "void _msgout (msg)\n");
      f_print (fout, "\tchar *msg;\n");
    }
  else
    {
      f_print (fout, "void _msgout (char* msg)\n");
    }
  f_print (fout, "{\n");
  f_print (fout, "#ifdef RPC_SVC_FG\n");
  if (inetdflag || pmflag)
    f_print (fout, "\tif (_rpcpmstart)\n");
  f_print (fout, "\t\tsyslog (LOG_ERR, \"%%s\", msg);\n");
  f_print (fout, "\telse\n");
  f_print (fout, "\t\tfprintf (stderr, \"%%s\\n\", msg);\n");
  f_print (fout, "#else\n");
  f_print (fout, "\tsyslog (LOG_ERR, \"%%s\", msg);\n");
  f_print (fout, "#endif\n");
  f_print (fout, "}\n");
}

/*
 * Write the timeout function
 */
static void
write_timeout_func (void)
{
  if (!timerflag)
    return;
  f_print (fout, "\n");
  f_print (fout, "static void\n");
  if (Cflag)
    f_print (fout, "closedown (int sig)\n");
  else
    f_print (fout, "closedown (sig)\n\tint sig;\n");
  f_print (fout, "{\n");

#if 0
  f_print (fout, "\t(void) signal (sig, %s closedown);\n",
	   Cflag ? "(SIG_PF)" : "(void(*)())");
#endif
  if (mtflag)
    f_print(fout, "\tmutex_lock(&_svcstate_lock);\n");
#if 0
  f_print (fout, "\tif (_rpcsvcdirty == 0) {\n");
#else
  f_print(fout, "\tif (_rpcsvcstate == _IDLE) {\n");
#endif
  f_print (fout, "\t\textern fd_set svc_fdset;\n");
  f_print (fout, "\t\tstatic int size;\n");
  f_print (fout, "\t\tint i, openfd;\n");
  if (tirpcflag && pmflag)
    {
      f_print (fout, "\t\tstruct t_info tinfo;\n\n");
      f_print (fout, "\t\tif (!t_getinfo(0, &tinfo) && (tinfo.servtype == T_CLTS))\n");
    }
  else
    {
      f_print (fout, "\n\t\tif (_rpcfdtype == SOCK_DGRAM)\n");
    }
  f_print (fout, "\t\t\texit (0);\n");
  f_print (fout, "\t\tif (size == 0) {\n");
  if (tirpcflag)
    {
      f_print (fout, "\t\t\tstruct rlimit rl;\n\n");
      f_print (fout, "\t\t\trl.rlim_max = 0;\n");
      f_print (fout, "\t\t\tgetrlimit(RLIMIT_NOFILE, &rl);\n");
      f_print (fout, "\t\t\tif ((size = rl.rlim_max) == 0) {\n");
      if (mtflag)
	f_print(fout, "\t\t\t\tmutex_unlock(&_svcstate_lock);\n");
       f_print (fout, "\t\t\t\treturn;\n\t\t\t}\n");
    }
  else
    {
      f_print (fout, "\t\t\tsize = getdtablesize();\n");
    }
  f_print (fout, "\t\t}\n");
  f_print (fout, "\t\tfor (i = 0, openfd = 0; i < size && openfd < 2; i++)\n");
  f_print (fout, "\t\t\tif (FD_ISSET(i, &svc_fdset))\n");
  f_print (fout, "\t\t\t\topenfd++;\n");
  f_print (fout, "\t\tif (openfd <= 1)\n");
  f_print (fout, "\t\t\texit (0);\n");
  f_print (fout, "\t}\n");
  f_print(fout, "\tif (_rpcsvcstate == _SERVED)\n");
  f_print(fout, "\t\t_rpcsvcstate = _IDLE;\n\n");
  if (mtflag)
    f_print(fout, "\tmutex_unlock(&_svcstate_lock);\n");
  f_print(fout, "\t(void) signal(SIGALRM, %s closedown);\n",
	  Cflag? "(SIG_PF)" : "(void(*)())");
  f_print (fout, "\talarm (_RPCSVC_CLOSEDOWN);\n");
  f_print (fout, "}\n");
}

/*
 * Write the most of port monitor support
 */
static void
write_pm_most (const char *infile, int netflag)
{
  list *l;
  definition *def;
  version_list *vp;

  f_print (fout, "\tif (!ioctl(0, I_LOOK, mname) &&\n");
  f_print (fout, "\t\t(!strcmp(mname, \"sockmod\") ||");
  f_print (fout, " !strcmp(mname, \"timod\"))) {\n");
  f_print (fout, "\t\tchar *netid;\n");
  if (!netflag)
    {				/* Not included by -n option */
      f_print (fout, "\t\tstruct netconfig *nconf = NULL;\n");
      f_print (fout, "\t\tSVCXPRT *%s;\n", TRANSP);
    }
  if (timerflag)
    f_print (fout, "\t\tint pmclose;\n");
/* not necessary, defined in /usr/include/stdlib */
/*      f_print(fout, "\t\textern char *getenv();\n"); */
  f_print (fout, "\n");
  f_print (fout, "\t\t_rpcpmstart = 1;\n");
  if (logflag)
    open_log_file (infile, "\t\t");
  f_print (fout, "\t\tif ((netid = getenv(\"NLSPROVIDER\")) == NULL) {\n");
  sprintf (_errbuf, "cannot get transport name");
  print_err_message ("\t\t\t");
  f_print (fout, "\t\t} else if ((nconf = getnetconfigent(netid)) == NULL) {\n");
  sprintf (_errbuf, "cannot get transport info");
  print_err_message ("\t\t\t");
  f_print (fout, "\t\t}\n");
  /*
   * A kludgy support for inetd services. Inetd only works with
   * sockmod, and RPC works only with timod, hence all this jugglery
   */
  f_print (fout, "\t\tif (strcmp(mname, \"sockmod\") == 0) {\n");
  f_print (fout, "\t\t\tif (ioctl(0, I_POP, 0) || ioctl(0, I_PUSH, \"timod\")) {\n");
  sprintf (_errbuf, "could not get the right module");
  print_err_message ("\t\t\t\t");
  f_print (fout, "\t\t\t\texit(1);\n");
  f_print (fout, "\t\t\t}\n");
  f_print (fout, "\t\t}\n");
  if (timerflag)
    f_print (fout, "\t\tpmclose = (t_getstate(0) != T_DATAXFER);\n");
  f_print (fout, "\t\tif ((%s = svc_tli_create(0, nconf, NULL, 0, 0)) == NULL) {\n",
	   TRANSP);
  sprintf (_errbuf, "cannot create server handle");
  print_err_message ("\t\t\t");
  f_print (fout, "\t\t\texit(1);\n");
  f_print (fout, "\t\t}\n");
  f_print (fout, "\t\tif (nconf)\n");
  f_print (fout, "\t\t\tfreenetconfigent(nconf);\n");
  for (l = defined; l != NULL; l = l->next)
    {
      def = (definition *) l->val;
      if (def->def_kind != DEF_PROGRAM)
	{
	  continue;
	}
      for (vp = def->def.pr.versions; vp != NULL; vp = vp->next)
	{
	  f_print (fout,
		   "\t\tif (!svc_reg(%s, %s, %s, ",
		   TRANSP, def->def_name, vp->vers_name);
	  pvname (def->def_name, vp->vers_num);
	  f_print (fout, ", 0)) {\n");
	  (void) sprintf (_errbuf, "unable to register (%s, %s).",
			  def->def_name, vp->vers_name);
	  print_err_message ("\t\t\t");
	  f_print (fout, "\t\t\texit(1);\n");
	  f_print (fout, "\t\t}\n");
	}
    }
  if (timerflag)
    {
      f_print (fout, "\t\tif (pmclose) {\n");
      f_print (fout, "\t\t\t(void) signal(SIGALRM, %s closedown);\n",
	       Cflag ? "(SIG_PF)" : "(void(*)())");
      f_print (fout, "\t\t\t(void) alarm(_RPCSVC_CLOSEDOWN);\n");
      f_print (fout, "\t\t}\n");
    }
  f_print (fout, "\t\tsvc_run();\n");
  f_print (fout, "\t\texit(1);\n");
  f_print (fout, "\t\t/* NOTREACHED */\n");
  f_print (fout, "\t}\n");
}

/*
 * Support for backgrounding the server if self started.
 */
static void
write_rpc_svc_fg (const char *infile, const char *sp)
{
  f_print (fout, "#ifndef RPC_SVC_FG\n");
  f_print (fout, "%sint size;\n", sp);
  if (tirpcflag)
    f_print (fout, "%sstruct rlimit rl;\n", sp);
  if (inetdflag)
    f_print (fout, "%sint pid, i;\n\n", sp);
  f_print (fout, "%spid = fork();\n", sp);
  f_print (fout, "%sif (pid < 0) {\n", sp);
  f_print (fout, "%s\tperror(\"cannot fork\");\n", sp);
  f_print (fout, "%s\texit(1);\n", sp);
  f_print (fout, "%s}\n", sp);
  f_print (fout, "%sif (pid)\n", sp);
  f_print (fout, "%s\texit(0);\n", sp);
  /* get number of file descriptors */
  if (tirpcflag)
    {
      f_print (fout, "%srl.rlim_max = 0;\n", sp);
      f_print (fout, "%sgetrlimit(RLIMIT_NOFILE, &rl);\n", sp);
      f_print (fout, "%sif ((size = rl.rlim_max) == 0)\n", sp);
      f_print (fout, "%s\texit(1);\n", sp);
    }
  else
    {
      f_print (fout, "%ssize = getdtablesize();\n", sp);
    }

  f_print (fout, "%sfor (i = 0; i < size; i++)\n", sp);
  f_print (fout, "%s\t(void) close(i);\n", sp);
  /* Redirect stderr and stdout to console */
  f_print (fout, "%si = open(\"/dev/console\", 2);\n", sp);
  f_print (fout, "%s(void) dup2(i, 1);\n", sp);
  f_print (fout, "%s(void) dup2(i, 2);\n", sp);
  /* This removes control of the controlling terminal */
  if (tirpcflag)
    f_print (fout, "%ssetsid();\n", sp);
  else
    {
      f_print (fout, "%si = open(\"/dev/tty\", 2);\n", sp);
      f_print (fout, "%sif (i >= 0) {\n", sp);
      f_print (fout, "%s\t(void) ioctl(i, TIOCNOTTY, (char *)NULL);\n", sp);;
      f_print (fout, "%s\t(void) close(i);\n", sp);
      f_print (fout, "%s}\n", sp);
    }
  if (!logflag)
    open_log_file (infile, sp);
  f_print (fout, "#endif\n");
  if (logflag)
    open_log_file (infile, sp);
}

static void
open_log_file (const char *infile, const char *sp)
{
  char *s;

  s = strrchr (infile, '.');
  if (s)
    *s = '\0';
  f_print (fout, "%sopenlog(\"%s\", LOG_PID, LOG_DAEMON);\n", sp, infile);
  if (s)
    *s = '.';
}

/*
 * write a registration for the given transport for Inetd
 */
void
write_inetd_register (const char *transp)
{
  list *l;
  definition *def;
  version_list *vp;
  const char *sp;
  int isudp;
  char tmpbuf[32];

  if (inetdflag)
    sp = "\t";
  else
    sp = "";
  if (streq (transp, "udp") || streq (transp, "udp6"))
    isudp = 1;
  else
    isudp = 0;
  f_print (fout, "\n");
  if (inetdflag)
    {
      f_print (fout, "\tif ((_rpcfdtype == 0) || (_rpcfdtype == %s)) {\n",
	       isudp ? "SOCK_DGRAM" : "SOCK_STREAM");
    }
  f_print (fout, "%s\t%s = svc%s_create(%s",
	   sp, TRANSP, transp, inetdflag ? "sock" : "RPC_ANYSOCK");
  if (!isudp)
    f_print (fout, ", 0, 0");
  f_print (fout, ");\n");
  f_print (fout, "%s\tif (%s == NULL) {\n", sp, TRANSP);
  (void) sprintf (_errbuf, "cannot create %s service.", transp);
  (void) sprintf (tmpbuf, "%s\t\t", sp);
  print_err_message (tmpbuf);
  f_print (fout, "%s\t\texit(1);\n", sp);
  f_print (fout, "%s\t}\n", sp);

  if (inetdflag)
    {
      f_print (fout, "%s\tif (!_rpcpmstart)\n\t", sp);
      f_print (fout, "%s\tproto = IPPROTO_%s;\n",
	       sp, isudp ? "UDP" : "TCP");
    }
  for (l = defined; l != NULL; l = l->next)
    {
      def = (definition *) l->val;
      if (def->def_kind != DEF_PROGRAM)
	{
	  continue;
	}
      for (vp = def->def.pr.versions; vp != NULL; vp = vp->next)
	{
	  f_print (fout, "%s\tif (!svc_register(%s, %s, %s, ",
		   sp, TRANSP, def->def_name, vp->vers_name);
	  pvname (def->def_name, vp->vers_num);
	  if (inetdflag)
	    f_print (fout, ", proto)) {\n");
	  else
	    f_print (fout, ", IPPROTO_%s)) {\n",
		     isudp ? "UDP" : "TCP");
	  (void) sprintf (_errbuf, "unable to register (%s, %s, %s).",
			  def->def_name, vp->vers_name, transp);
	  print_err_message (tmpbuf);
	  f_print (fout, "%s\t\texit(1);\n", sp);
	  f_print (fout, "%s\t}\n", sp);
	}
    }
  if (inetdflag)
    f_print (fout, "\t}\n");
}
