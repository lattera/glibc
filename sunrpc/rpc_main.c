/*
 * From @(#)rpc_main.c 1.30 89/03/30
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
 * rpc_main.c, Top level of the RPC protocol compiler.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libintl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "rpc_parse.h"
#include "rpc_util.h"
#include "rpc_scan.h"
#include "proto.h"

#include "../version.h"
#define PACKAGE _libc_intl_domainname

#define EXTEND	1		/* alias for TRUE */
#define DONT_EXTEND	0	/* alias for FALSE */

struct commandline
  {
    int cflag;			/* xdr C routines */
    int hflag;			/* header file */
    int lflag;			/* client side stubs */
    int mflag;			/* server side stubs */
    int nflag;			/* netid flag */
    int sflag;			/* server stubs for the given transport */
    int tflag;			/* dispatch Table file */
    int Ssflag;			/* produce server sample code */
    int Scflag;			/* produce client sample code */
    int makefileflag;		/* Generate a template Makefile */
    const char *infile;		/* input module name */
    const char *outfile;	/* output module name */
  };


static const char *cmdname;

static const char *svcclosetime = "120";
static int cppDefined;	/* explicit path for C preprocessor */
static const char *CPP = "/lib/cpp";
static const char CPPFLAGS[] = "-C";
static char *pathbuf;
static int cpp_pid;
static const char *allv[] =
{
  "rpcgen", "-s", "udp", "-s", "tcp"
};
static int allc = sizeof (allv) / sizeof (allv[0]);
static const char *allnv[] =
{
  "rpcgen", "-s", "netpath",
};
static int allnc = sizeof (allnv) / sizeof (allnv[0]);

/*
 * machinations for handling expanding argument list
 */
static void addarg (const char *);	/* add another argument to the list */
static void putarg (int, const char *);		/* put argument at specified location */
static void clear_args (void);	/* clear argument list */
static void checkfiles (const char *, const char *);
				       /* check if out file already exists */

static void clear_args (void);
static char *extendfile (const char *file, const char *ext);
static void open_output (const char *infile, const char *outfile);
static void add_warning (void);
static void clear_args (void);
static void find_cpp (void);
static void open_input (const char *infile, const char *define);
static int check_nettype (const char *name, const char *list_to_check[]);
static void c_output (const char *infile, const char *define,
		      int extend, const char *outfile);
static void h_output (const char *infile, const char *define,
		      int extend, const char *outfile);
static void s_output (int argc, const char *argv[], const char *infile,
		      const char *define, int extend,
		      const char *outfile, int nomain, int netflag);
static void l_output (const char *infile, const char *define,
		      int extend, const char *outfile);
static void t_output (const char *infile, const char *define,
		      int extend, const char *outfile);
static void svc_output (const char *infile, const char *define,
			int extend, const char *outfile);
static void clnt_output (const char *infile, const char *define,
			 int extend, const char *outfile);
static void mkfile_output (struct commandline *cmd);
static int do_registers (int argc, const char *argv[]);
static void addarg (const char *cp);
static void putarg (int whereto, const char *cp);
static void checkfiles (const char *infile, const char *outfile);
static int parseargs (int argc, const char *argv[], struct commandline *cmd);
static void usage (FILE *stream, int status) __attribute__ ((noreturn));
static void options_usage (FILE *stream, int status) __attribute__ ((noreturn));
static void print_version (void);
static void c_initialize (void);
static char *generate_guard (const char *pathname);


#define ARGLISTLEN	20
#define FIXEDARGS         2

static const char *arglist[ARGLISTLEN];
static int argcount = FIXEDARGS;


int nonfatalerrors;		/* errors */
int inetdflag /* = 1 */ ;	/* Support for inetd *//* is now the default */
int pmflag;			/* Support for port monitors */
int logflag;			/* Use syslog instead of fprintf for errors */
int tblflag;			/* Support for dispatch table file */
int mtflag;			/* Support for MT */

#define INLINE 3
/*length at which to start doing an inline */

int inlineflag = INLINE;	/* length at which to start doing an inline. 3 = default
				   if 0, no xdr_inline code */

int indefinitewait;		/* If started by port monitors, hang till it wants */
int exitnow;			/* If started by port monitors, exit after the call */
int timerflag;			/* TRUE if !indefinite && !exitnow */
int newstyle;			/* newstyle of passing arguments (by value) */
int Cflag = 1;			/* ANSI C syntax */
int CCflag;			/* C++ files */
static int allfiles;		/* generate all files */
int tirpcflag;			/* generating code for tirpc, by default */
xdrfunc *xdrfunc_head;		/* xdr function list */
xdrfunc *xdrfunc_tail;		/* xdr function list */

int
main (int argc, const char *argv[])
{
  struct commandline cmd;

  (void) memset ((char *) &cmd, 0, sizeof (struct commandline));
  clear_args ();
  if (!parseargs (argc, argv, &cmd))
    usage (stderr, 1);

  if (cmd.cflag || cmd.hflag || cmd.lflag || cmd.tflag || cmd.sflag ||
      cmd.mflag || cmd.nflag || cmd.Ssflag || cmd.Scflag)
    {
      checkfiles (cmd.infile, cmd.outfile);
    }
  else
    checkfiles (cmd.infile, NULL);

  if (cmd.cflag)
    c_output (cmd.infile, "-DRPC_XDR", DONT_EXTEND, cmd.outfile);
  else if (cmd.hflag)
    h_output (cmd.infile, "-DRPC_HDR", DONT_EXTEND, cmd.outfile);
  else if (cmd.lflag)
    l_output (cmd.infile, "-DRPC_CLNT", DONT_EXTEND, cmd.outfile);
  else if (cmd.sflag || cmd.mflag || (cmd.nflag))
    s_output (argc, argv, cmd.infile, "-DRPC_SVC", DONT_EXTEND,
	      cmd.outfile, cmd.mflag, cmd.nflag);
  else if (cmd.tflag)
    t_output (cmd.infile, "-DRPC_TBL", DONT_EXTEND, cmd.outfile);
  else if (cmd.Ssflag)
    svc_output (cmd.infile, "-DRPC_SERVER", DONT_EXTEND, cmd.outfile);
  else if (cmd.Scflag)
    clnt_output (cmd.infile, "-DRPC_CLIENT", DONT_EXTEND, cmd.outfile);
  else if (cmd.makefileflag)
    mkfile_output (&cmd);
  else
    {
      /* the rescans are required, since cpp may effect input */
      c_output (cmd.infile, "-DRPC_XDR", EXTEND, "_xdr.c");
      reinitialize ();
      h_output (cmd.infile, "-DRPC_HDR", EXTEND, ".h");
      reinitialize ();
      l_output (cmd.infile, "-DRPC_CLNT", EXTEND, "_clnt.c");
      reinitialize ();
      if (inetdflag || !tirpcflag)
	s_output (allc, allv, cmd.infile, "-DRPC_SVC", EXTEND,
		  "_svc.c", cmd.mflag, cmd.nflag);
      else
	s_output (allnc, allnv, cmd.infile, "-DRPC_SVC",
		  EXTEND, "_svc.c", cmd.mflag, cmd.nflag);
      if (tblflag)
	{
	  reinitialize ();
	  t_output (cmd.infile, "-DRPC_TBL", EXTEND, "_tbl.i");
	}
      if (allfiles)
	{
	  reinitialize ();
	  svc_output (cmd.infile, "-DRPC_SERVER", EXTEND, "_server.c");
	  reinitialize ();
	  clnt_output (cmd.infile, "-DRPC_CLIENT", EXTEND, "_client.c");
	}
      if (allfiles || (cmd.makefileflag == 1))
	{
	  reinitialize ();
	  mkfile_output (&cmd);
	}
    }

  return nonfatalerrors;
}

/*
 * add extension to filename
 */
static char *
extendfile (const char *file, const char *ext)
{
  char *res;
  const char *p;

  res = alloc (strlen (file) + strlen (ext) + 1);
  if (res == NULL)
    abort ();
  p = strrchr (file, '.');
  if (p == NULL)
    p = file + strlen (file);
  strcpy (res, file);
  strcpy (res + (p - file), ext);
  return res;
}

/*
 * Open output file with given extension
 */
static void
open_output (const char *infile, const char *outfile)
{
  if (outfile == NULL)
    {
      fout = stdout;
      return;
    }

  if (infile != NULL && streq (outfile, infile))
    {
      fprintf (stderr, _ ("%s: output would overwrite %s\n"), cmdname,
	       infile);
      crash ();
    }
  fout = fopen (outfile, "w");
  if (fout == NULL)
    {
      fprintf (stderr, _ ("%s: unable to open %s: %m\n"), cmdname, outfile);
      crash ();
    }
  record_open (outfile);
}

/* Close the output file and check for write errors.  */
static void
close_output (const char *outfile)
{
  if (fclose (fout) == EOF)
    {
      fprintf (stderr, _("%s: while writing output %s: %m"), cmdname,
	       outfile ?: "<stdout>");
      crash ();
    }
}

static void
add_warning (void)
{
  fprintf (fout, "/*\n");
  fprintf (fout, " * Please do not edit this file.\n");
  fprintf (fout, " * It was generated using rpcgen.\n");
  fprintf (fout, " */\n\n");
}

/* clear list of arguments */
static void
clear_args (void)
{
  int i;
  for (i = FIXEDARGS; i < ARGLISTLEN; ++i)
    arglist[i] = NULL;
  argcount = FIXEDARGS;
}

/* make sure that a CPP exists */
static void
find_cpp (void)
{
  struct stat buf;

  if (stat (CPP, &buf) == 0)
    return;

  if (cppDefined) /* user specified cpp but it does not exist */
    {
      fprintf (stderr, _ ("cannot find C preprocessor: %s\n"), CPP);
      crash ();
    }

  /* fall back to system CPP */
  CPP = "cpp";
}

/*
 * Open input file with given define for C-preprocessor
 */
static void
open_input (const char *infile, const char *define)
{
  int pd[2];

  infilename = (infile == NULL) ? "<stdin>" : infile;
  if (pipe (pd) != 0)
    {
      perror ("pipe");
      exit (1);
    }
  cpp_pid = fork ();
  switch (cpp_pid)
    {
    case 0:
      find_cpp ();
      putarg (0, CPP);
      putarg (1, CPPFLAGS);
      addarg (define);
      if (infile)
	addarg (infile);
      addarg ((char *) NULL);
      close (1);
      dup2 (pd[1], 1);
      close (pd[0]);
      execvp (arglist[0], (char **) arglist);
      if (errno == ENOENT)
        {
          fprintf (stderr, _ ("cannot find C preprocessor: %s\n"), CPP);
          exit (1);
        }
      perror ("execvp");
      exit (1);
    case -1:
      perror ("fork");
      exit (1);
    }
  close (pd[1]);
  fin = fdopen (pd[0], "r");
  if (fin == NULL)
    {
      fprintf (stderr, "%s: ", cmdname);
      perror (infilename);
      crash ();
    }
}

/* Close the connection to the C-preprocessor and check for successfull
   termination.  */
static void
close_input (void)
{
  int status;

  fclose (fin);
  /* Check the termination status.  */
  if (waitpid (cpp_pid, &status, 0) < 0)
    {
      perror ("waitpid");
      crash ();
    }
  if (WIFSIGNALED (status) || WEXITSTATUS (status) != 0)
    {
      if (WIFSIGNALED (status))
	fprintf (stderr, _("%s: C preprocessor failed with signal %d\n"),
		 cmdname, WTERMSIG (status));
      else
	fprintf (stderr, _("%s: C preprocessor failed with exit code %d\n"),
		 cmdname, WEXITSTATUS (status));
      crash ();
    }
}

/* valid tirpc nettypes */
static const char *valid_ti_nettypes[] =
{
  "netpath",
  "visible",
  "circuit_v",
  "datagram_v",
  "circuit_n",
  "datagram_n",
  "udp",
  "tcp",
  "raw",
  NULL
};

/* valid inetd nettypes */
static const char *valid_i_nettypes[] =
{
  "udp",
  "tcp",
  NULL
};

static int
check_nettype (const char *name, const char *list_to_check[])
{
  int i;
  for (i = 0; list_to_check[i] != NULL; i++)
    {
      if (strcmp (name, list_to_check[i]) == 0)
	{
	  return 1;
	}
    }
  fprintf (stderr, _ ("illegal nettype: `%s'\n"), name);
  return 0;
}

/*
 * Compile into an XDR routine output file
 */

static void
c_output (const char *infile, const char *define, int extend,
	  const char *outfile)
{
  definition *def;
  char *include;
  const char *outfilename;
  long tell;

  c_initialize ();
  open_input (infile, define);
  outfilename = extend ? extendfile (infile, outfile) : outfile;
  open_output (infile, outfilename);
  add_warning ();
  if (infile && (include = extendfile (infile, ".h")))
    {
      fprintf (fout, "#include \"%s\"\n", include);
      free (include);
      /* .h file already contains rpc/rpc.h */
    }
  else
    fprintf (fout, "#include <rpc/rpc.h>\n");
  tell = ftell (fout);
  while ((def = get_definition ()) != NULL)
    emit (def);

  if (extend && tell == ftell (fout))
    unlink (outfilename);
  close_input ();
  close_output (outfilename);
}

void
c_initialize (void)
{

  /* add all the starting basic types */

  add_type (1, "int");
  add_type (1, "long");
  add_type (1, "short");
  add_type (1, "bool");

  add_type (1, "u_int");
  add_type (1, "u_long");
  add_type (1, "u_short");

}

char rpcgen_table_dcl[] = "struct rpcgen_table {\n\
	char	*(*proc)();\n\
	xdrproc_t	xdr_arg;\n\
	unsigned	len_arg;\n\
	xdrproc_t	xdr_res;\n\
	unsigned	len_res;\n\
};\n";


static char *
generate_guard (const char *pathname)
{
  const char *filename;
  char *guard, *tmp;

  filename = strrchr (pathname, '/');	/* find last component */
  filename = ((filename == NULL) ? pathname : filename + 1);
  guard = extendfile (filename, "_H_RPCGEN");
  /* convert to upper case */
  tmp = guard;
  while (*tmp)
    {
      if (islower (*tmp))
	*tmp = toupper (*tmp);
      tmp++;
    }

  return guard;
}

/*
 * Compile into an XDR header file
 */


static void
h_output (const char *infile, const char *define, int extend,
	  const char *outfile)
{
  xdrfunc *xdrfuncp;
  definition *def;
  const char *ifilename;
  const char *outfilename;
  long tell;
  char *guard;
  list *l;

  open_input (infile, define);
  outfilename = extend ? extendfile (infile, outfile) : outfile;
  open_output (infile, outfilename);
  add_warning ();
  ifilename = (infile == NULL) ? "STDIN" : infile;
  guard = generate_guard (outfilename ? outfilename : ifilename);

  fprintf (fout, "#ifndef _%s\n#define _%s\n\n", guard,
	   guard);

  fprintf (fout, "#include <rpc/rpc.h>\n\n");

  if (mtflag)
    {
      fprintf (fout, "#include <pthread.h>\n");
    }

  /* put the C++ support */
  if (Cflag && !CCflag)
    {
      fprintf (fout, "\n#ifdef __cplusplus\n");
      fprintf (fout, "extern \"C\" {\n");
      fprintf (fout, "#endif\n\n");
    }

  tell = ftell (fout);
  /* print data definitions */
  while ((def = get_definition ()) != NULL)
    {
      print_datadef (def);
    }

  /* print function declarations.
     Do this after data definitions because they might be used as
     arguments for functions */
  for (l = defined; l != NULL; l = l->next)
    {
      print_funcdef (l->val);
    }
  /* Now  print all xdr func declarations */
  if (xdrfunc_head != NULL)
    {
      fprintf (fout, "\n/* the xdr functions */\n");
      if (CCflag)
	{
	  fprintf (fout, "\n#ifdef __cplusplus\n");
	  fprintf (fout, "extern \"C\" {\n");
	  fprintf (fout, "#endif\n");
	}
      if (!Cflag)
	{
	  xdrfuncp = xdrfunc_head;
	  while (xdrfuncp != NULL)
	    {
	      print_xdr_func_def (xdrfuncp->name,
				  xdrfuncp->pointerp, 2);
	      xdrfuncp = xdrfuncp->next;
	    }
	}
      else
	{
	  int i;

	  for (i = 1; i < 3; ++i)
	    {
	      if (i == 1)
		fprintf (fout, "\n#if defined(__STDC__) || defined(__cplusplus)\n");
	      else
		fprintf (fout, "\n#else /* K&R C */\n");

	      xdrfuncp = xdrfunc_head;
	      while (xdrfuncp != NULL)
		{
		  print_xdr_func_def (xdrfuncp->name,
				      xdrfuncp->pointerp, i);
		  xdrfuncp = xdrfuncp->next;
		}
	    }
	  fprintf (fout, "\n#endif /* K&R C */\n");
	}
    }

  if (extend && tell == ftell (fout))
    {
      unlink (outfilename);
    }
  else if (tblflag)
    {
      fprintf (fout, "%s", rpcgen_table_dcl);
    }

  if (Cflag)
    {
      fprintf (fout, "\n#ifdef __cplusplus\n");
      fprintf (fout, "}\n");
      fprintf (fout, "#endif\n");
    }

  fprintf (fout, "\n#endif /* !_%s */\n", guard);
  free (guard);
  close_input ();
  close_output (outfilename);
}

/*
 * Compile into an RPC service
 */
static void
s_output (int argc, const char *argv[], const char *infile, const char *define,
	  int extend, const char *outfile, int nomain, int netflag)
{
  char *include;
  definition *def;
  int foundprogram = 0;
  const char *outfilename;

  open_input (infile, define);
  outfilename = extend ? extendfile (infile, outfile) : outfile;
  open_output (infile, outfilename);
  add_warning ();
  if (infile && (include = extendfile (infile, ".h")))
    {
      fprintf (fout, "#include \"%s\"\n", include);
      free (include);
    }
  else
    fprintf (fout, "#include <rpc/rpc.h>\n");

  fprintf (fout, "#include <stdio.h>\n");
  fprintf (fout, "#include <stdlib.h>\n");
  fprintf (fout, "#include <rpc/pmap_clnt.h>\n");
  if (Cflag)
    fprintf (fout, "#include <string.h>\n");
  if (strcmp (svcclosetime, "-1") == 0)
    indefinitewait = 1;
  else if (strcmp (svcclosetime, "0") == 0)
    exitnow = 1;
  else if (inetdflag || pmflag)
    {
      fprintf (fout, "#include <signal.h>\n");
      timerflag = 1;
    }

  if (!tirpcflag && inetdflag)
    fprintf (fout, "#include <sys/ioctl.h> /* ioctl, TIOCNOTTY */\n");
  if (Cflag && (inetdflag || pmflag))
    {
      fprintf (fout, "#include <sys/types.h> /* open */\n");
      fprintf (fout, "#include <sys/stat.h> /* open */\n");
      fprintf (fout, "#include <fcntl.h> /* open */\n");
      fprintf (fout, "#include <unistd.h> /* getdtablesize */\n");
    }
  if (tirpcflag && !(Cflag && (inetdflag || pmflag)))
    fprintf (fout, "#include <sys/types.h>\n");

  fprintf (fout, "#include <memory.h>\n");
  if (inetdflag || !tirpcflag)
    {
      fprintf (fout, "#include <sys/socket.h>\n");
      fprintf (fout, "#include <netinet/in.h>\n");
    }

  if ((netflag || pmflag) && tirpcflag && !nomain)
    {
      fprintf (fout, "#include <netconfig.h>\n");
    }
  if ( /*timerflag && */ tirpcflag)
    fprintf (fout, "#include <sys/resource.h> /* rlimit */\n");
  if (logflag || inetdflag || pmflag)
    {
      fprintf (fout, "#include <syslog.h>\n");
    }

  /* for ANSI-C */
  if (Cflag)
    fprintf (fout, "\n#ifndef SIG_PF\n#define SIG_PF void(*)(int)\n#endif\n");

  if (timerflag)
    fprintf (fout, "\n#define _RPCSVC_CLOSEDOWN %s\n", svcclosetime);
  while ((def = get_definition ()) != NULL)
    {
      foundprogram |= (def->def_kind == DEF_PROGRAM);
    }
  if (extend && !foundprogram)
    {
      unlink (outfilename);
      return;
    }
  write_most (infile, netflag, nomain);
  if (!nomain)
    {
      if (!do_registers (argc, argv))
	{
	  if (outfilename)
	    unlink (outfilename);
	  usage (stderr, 1);
	}
      write_rest ();
    }
  close_input ();
  close_output (outfilename);
}

/*
 * generate client side stubs
 */
static void
l_output (const char *infile, const char *define, int extend,
	  const char *outfile)
{
  char *include;
  definition *def;
  int foundprogram = 0;
  const char *outfilename;

  open_input (infile, define);
  outfilename = extend ? extendfile (infile, outfile) : outfile;
  open_output (infile, outfilename);
  add_warning ();
  if (Cflag)
    fprintf (fout, "#include <memory.h> /* for memset */\n");
  if (infile && (include = extendfile (infile, ".h")))
    {
      fprintf (fout, "#include \"%s\"\n", include);
      free (include);
    }
  else
    fprintf (fout, "#include <rpc/rpc.h>\n");
  while ((def = get_definition ()) != NULL)
    {
      foundprogram |= (def->def_kind == DEF_PROGRAM);
    }
  if (extend && !foundprogram)
    {
      unlink (outfilename);
      return;
    }
  write_stubs ();
  close_input ();
  close_output (outfilename);
}

/*
 * generate the dispatch table
 */
static void
t_output (const char *infile, const char *define, int extend,
	  const char *outfile)
{
  definition *def;
  int foundprogram = 0;
  const char *outfilename;

  open_input (infile, define);
  outfilename = extend ? extendfile (infile, outfile) : outfile;
  open_output (infile, outfilename);
  add_warning ();
  while ((def = get_definition ()) != NULL)
    {
      foundprogram |= (def->def_kind == DEF_PROGRAM);
    }
  if (extend && !foundprogram)
    {
      unlink (outfilename);
      return;
    }
  write_tables ();
  close_input ();
  close_output (outfilename);
}

/* sample routine for the server template */
static void
svc_output (const char *infile, const char *define, int extend,
	    const char *outfile)
{
  definition *def;
  char *include;
  const char *outfilename;
  long tell;

  open_input (infile, define);
  outfilename = extend ? extendfile (infile, outfile) : outfile;
  checkfiles (infile, outfilename);
  /*check if outfile already exists.
     if so, print an error message and exit */
  open_output (infile, outfilename);
  add_sample_msg ();

  if (infile && (include = extendfile (infile, ".h")))
    {
      fprintf (fout, "#include \"%s\"\n", include);
      free (include);
    }
  else
    fprintf (fout, "#include <rpc/rpc.h>\n");

  tell = ftell (fout);
  while ((def = get_definition ()) != NULL)
    {
      write_sample_svc (def);
    }
  if (extend && tell == ftell (fout))
    {
      unlink (outfilename);
    }
  close_input ();
  close_output (outfilename);
}


/* sample main routine for client */
static void
clnt_output (const char *infile, const char *define, int extend,
	     const char *outfile)
{
  definition *def;
  char *include;
  const char *outfilename;
  long tell;
  int has_program = 0;

  open_input (infile, define);
  outfilename = extend ? extendfile (infile, outfile) : outfile;
  checkfiles (infile, outfilename);
  /*check if outfile already exists.
     if so, print an error message and exit */

  open_output (infile, outfilename);
  add_sample_msg ();
  if (infile && (include = extendfile (infile, ".h")))
    {
      fprintf (fout, "#include \"%s\"\n", include);
      free (include);
    }
  else
    fprintf (fout, "#include <rpc/rpc.h>\n");
  tell = ftell (fout);
  while ((def = get_definition ()) != NULL)
    {
      has_program += write_sample_clnt (def);
    }

  if (has_program)
    write_sample_clnt_main ();

  if (extend && tell == ftell (fout))
    {
      unlink (outfilename);
    }
  close_input ();
  close_output (outfilename);
}

static const char space[] = " ";

static char *
file_name (const char *file, const char *ext)
{
  char *temp;
  temp = extendfile (file, ext);

  if (access (temp, F_OK) != -1)
    return (temp);

  free (temp);
  return (char *) space;
}

static void
mkfile_output (struct commandline *cmd)
{
  char *mkfilename;
  char *clientname, *clntname, *xdrname, *hdrname;
  char *servername, *svcname, *servprogname, *clntprogname;

  svcname = file_name (cmd->infile, "_svc.c");
  clntname = file_name (cmd->infile, "_clnt.c");
  xdrname = file_name (cmd->infile, "_xdr.c");
  hdrname = file_name (cmd->infile, ".h");

  if (allfiles)
    {
      servername = extendfile (cmd->infile, "_server.c");
      clientname = extendfile (cmd->infile, "_client.c");
    }
  else
    {
      servername = (char *) space;
      clientname = (char *) space;
    }
  servprogname = extendfile (cmd->infile, "_server");
  clntprogname = extendfile (cmd->infile, "_client");

  if (allfiles)
    {
      char *cp, *temp;

      mkfilename = alloc (strlen ("Makefile.") + strlen (cmd->infile) + 1);
      if (mkfilename == NULL)
	abort ();
      temp = rindex (cmd->infile, '.');
      cp = stpcpy (mkfilename, "Makefile.");
      if (temp != NULL)
	*((char *) stpncpy (cp, cmd->infile, temp - cmd->infile)) = '\0';
      else
	stpcpy (cp, cmd->infile);

    }
  else
    mkfilename = (char *) cmd->outfile;

  checkfiles (NULL, mkfilename);
  open_output (NULL, mkfilename);

  fprintf (fout, "\n# This is a template Makefile generated by rpcgen\n");

  f_print (fout, "\n# Parameters\n\n");

  f_print (fout, "CLIENT = %s\nSERVER = %s\n\n", clntprogname, servprogname);
  f_print (fout, "SOURCES_CLNT.c = \nSOURCES_CLNT.h = \n");
  f_print (fout, "SOURCES_SVC.c = \nSOURCES_SVC.h = \n");
  f_print (fout, "SOURCES.x = %s\n\n", cmd->infile);
  f_print (fout, "TARGETS_SVC.c = %s %s %s \n",
	   svcname, servername, xdrname);
  f_print (fout, "TARGETS_CLNT.c = %s %s %s \n",
	   clntname, clientname, xdrname);
  f_print (fout, "TARGETS = %s %s %s %s %s %s\n\n",
	   hdrname, xdrname, clntname,
	   svcname, clientname, servername);

  f_print (fout, "OBJECTS_CLNT = $(SOURCES_CLNT.c:%%.c=%%.o) \
$(TARGETS_CLNT.c:%%.c=%%.o)");

  f_print (fout, "\nOBJECTS_SVC = $(SOURCES_SVC.c:%%.c=%%.o) \
$(TARGETS_SVC.c:%%.c=%%.o)");

  f_print (fout, "\n# Compiler flags \n");
  if (mtflag)
    fprintf (fout, "\nCPPFLAGS += -D_REENTRANT\nCFLAGS += -g \nLDLIBS \
+= -lnsl -lpthread \n ");
  else
    f_print (fout, "\nCFLAGS += -g \nLDLIBS += -lnsl\n");
  f_print (fout, "RPCGENFLAGS = \n");

  f_print (fout, "\n# Targets \n\n");

  f_print (fout, "all : $(CLIENT) $(SERVER)\n\n");
  f_print (fout, "$(TARGETS) : $(SOURCES.x) \n");
  f_print (fout, "\trpcgen $(RPCGENFLAGS) $(SOURCES.x)\n\n");
  f_print (fout, "$(OBJECTS_CLNT) : $(SOURCES_CLNT.c) $(SOURCES_CLNT.h) \
$(TARGETS_CLNT.c) \n\n");

  f_print (fout, "$(OBJECTS_SVC) : $(SOURCES_SVC.c) $(SOURCES_SVC.h) \
$(TARGETS_SVC.c) \n\n");
  f_print (fout, "$(CLIENT) : $(OBJECTS_CLNT) \n");
  f_print (fout, "\t$(LINK.c) -o $(CLIENT) $(OBJECTS_CLNT) \
$(LDLIBS) \n\n");
  f_print (fout, "$(SERVER) : $(OBJECTS_SVC) \n");
  f_print (fout, "\t$(LINK.c) -o $(SERVER) $(OBJECTS_SVC) $(LDLIBS)\n\n ");
  f_print (fout, "clean:\n\t $(RM) core $(TARGETS) $(OBJECTS_CLNT) \
$(OBJECTS_SVC) $(CLIENT) $(SERVER)\n\n");
  close_output (mkfilename);

  free (clntprogname);
  free (servprogname);
  if (servername != space)
    free (servername);
  if (clientname != space)
    free (clientname);
  if (mkfilename != (char *) cmd->outfile)
    free (mkfilename);
  if (svcname != space)
    free (svcname);
  if (clntname != space)
    free (clntname);
  if (xdrname != space)
    free (xdrname);
  if (hdrname != space)
    free (hdrname);
}

/*
 * Perform registrations for service output
 * Return 0 if failed; 1 otherwise.
 */
static int
do_registers (int argc, const char *argv[])
{
  int i;

  if (inetdflag || !tirpcflag)
    {
      for (i = 1; i < argc; i++)
	{
	  if (streq (argv[i], "-s"))
	    {
	      if (!check_nettype (argv[i + 1], valid_i_nettypes))
		return 0;
	      write_inetd_register (argv[i + 1]);
	      i++;
	    }
	}
    }
  else
    {
      for (i = 1; i < argc; i++)
	if (streq (argv[i], "-s"))
	  {
	    if (!check_nettype (argv[i + 1], valid_ti_nettypes))
	      return 0;
	    write_nettype_register (argv[i + 1]);
	    i++;
	  }
	else if (streq (argv[i], "-n"))
	  {
	    write_netid_register (argv[i + 1]);
	    i++;
	  }
    }
  return 1;
}

/*
 * Add another argument to the arg list
 */
static void
addarg (const char *cp)
{
  if (argcount >= ARGLISTLEN)
    {
      fprintf (stderr, _("rpcgen: too many defines\n"));
      crash ();
      /*NOTREACHED */
    }
  arglist[argcount++] = cp;
}

static void
putarg (int whereto, const char *cp)
{
  if (whereto >= ARGLISTLEN)
    {
      fprintf (stderr, _("rpcgen: arglist coding error\n"));
      crash ();
      /*NOTREACHED */
    }
  arglist[whereto] = cp;
}

/*
 * if input file is stdin and an output file is specified then complain
 * if the file already exists. Otherwise the file may get overwritten
 * If input file does not exist, exit with an error
 */

static void
checkfiles (const char *infile, const char *outfile)
{
  struct stat buf;

  if (infile)			/* infile ! = NULL */
    if (stat (infile, &buf) < 0)
      {
	perror (infile);
	crash ();
      }
  if (outfile)
    {
      if (stat (outfile, &buf) < 0)
	return;			/* file does not exist */
      else
	{
	  fprintf (stderr,
		   /* TRANS: the file will not be removed; this is an
		      TRANS: informative message.  */
		   _("file `%s' already exists and may be overwritten\n"),
		   outfile);
	  crash ();
	}
    }
}

/*
 * Parse command line arguments
 */
static int
parseargs (int argc, const char *argv[], struct commandline *cmd)
{
  int i;
  int j;
  int c;
  char flag[(1 << 8 * sizeof (char))];
  int nflags;

  cmdname = argv[0];
  cmd->infile = cmd->outfile = NULL;
  if (argc < 2)
    {
      return (0);
    }
  allfiles = 0;
  flag['c'] = 0;
  flag['h'] = 0;
  flag['l'] = 0;
  flag['m'] = 0;
  flag['o'] = 0;
  flag['s'] = 0;
  flag['n'] = 0;
  flag['t'] = 0;
  flag['S'] = 0;
  flag['C'] = 0;
  flag['M'] = 0;

  for (i = 1; i < argc; i++)
    {
      if (argv[i][0] != '-')
	{
	  if (cmd->infile)
	    {
	      fprintf (stderr,
		       _("Cannot specify more than one input file!\n"));
	      return 0;
	    }
	  cmd->infile = argv[i];
	}
      else if (strcmp (argv[i], "--help") == 0)
	usage (stdout, 0);
      else if (strcmp (argv[i], "--version") == 0)
	print_version ();
      else
	{
	  for (j = 1; argv[i][j] != 0; j++)
	    {
	      c = argv[i][j];
	      switch (c)
		{
		case 'a':
		  allfiles = 1;
		  break;
		case 'c':
		case 'h':
		case 'l':
		case 'm':
		case 't':
		  if (flag[c])
		    return 0;
		  flag[c] = 1;
		  break;
		case 'S':
		  /* sample flag: Ss or Sc.
		     Ss means set flag['S'];
		     Sc means set flag['C'];
		     Sm means set flag['M']; */
 		  c = argv[i][++j];	/* get next char */
		  if (c == 's')
		    c = 'S';
		  else if (c == 'c')
		    c = 'C';
		  else if (c == 'm')
		    c = 'M';
		  else
		    return 0;

		  if (flag[c])
		    return 0;
		  flag[c] = 1;
		  break;
		case 'C':	/* ANSI C syntax */
		  Cflag = 1;
		  break;

		case 'k':  /* K&R C syntax */
		  Cflag = 0;
		  break;

		case 'b':  /* turn TIRPC flag off for
			      generating backward compatible
			   */
		  tirpcflag = 0;
		  break;

		case '5':  /* turn TIRPC flag on for
			      generating SysVr4 compatible
			   */
		  tirpcflag = 1;
		  break;
		case 'I':
		  inetdflag = 1;
		  break;
		case 'N':
		  newstyle = 1;
		  break;
		case 'L':
		  logflag = 1;
		  break;
		case 'K':
		  if (++i == argc)
		    {
		      return (0);
		    }
		  svcclosetime = argv[i];
		  goto nextarg;
		case 'T':
		  tblflag = 1;
		  break;
		case 'M':
		  mtflag = 1;
		  break;
		case 'i':
		  if (++i == argc)
		    {
		      return (0);
		    }
		  inlineflag = atoi (argv[i]);
		  goto nextarg;
		case 'n':
		case 'o':
		case 's':
		  if (argv[i][j - 1] != '-' ||
		      argv[i][j + 1] != 0)
		    {
		      return (0);
		    }
		  flag[c] = 1;
		  if (++i == argc)
		    {
		      return (0);
		    }
		  if (c == 's')
		    {
		      if (!streq (argv[i], "udp") &&
			  !streq (argv[i], "tcp"))
			return 0;
		    }
		  else if (c == 'o')
		    {
		      if (cmd->outfile)
			return 0;
		      cmd->outfile = argv[i];
		    }
		  goto nextarg;
		case 'D':
		  if (argv[i][j - 1] != '-')
		    return 0;
		  addarg (argv[i]);
		  goto nextarg;
		case 'Y':
		  if (++i == argc)
		    return 0;
		  {
		    size_t len = strlen (argv[i]);
		    pathbuf = malloc (len + 5);
		    if (pathbuf == NULL)
		      {
			perror (cmdname);
			crash ();
		      }
		    stpcpy (stpcpy (pathbuf,
				    argv[i]),
			    "/cpp");
		    CPP = pathbuf;
		    cppDefined = 1;
		    goto nextarg;
		  }

		default:
		  return 0;
		}
	      }
	nextarg:
	  ;
	}
    }

  cmd->cflag = flag['c'];
  cmd->hflag = flag['h'];
  cmd->lflag = flag['l'];
  cmd->mflag = flag['m'];
  cmd->nflag = flag['n'];
  cmd->sflag = flag['s'];
  cmd->tflag = flag['t'];
  cmd->Ssflag = flag['S'];
  cmd->Scflag = flag['C'];
  cmd->makefileflag = flag['M'];

#ifndef _RPC_THREAD_SAFE_
  if (mtflag || newstyle)
    {
      /* glibc doesn't support these flags.  */
      f_print (stderr,
	       _("This implementation doesn't support newstyle or MT-safe code!\n"));
      return (0);
    }
#endif
  if (tirpcflag)
    {
      pmflag = inetdflag ? 0 : 1;    /* pmflag or inetdflag is always TRUE */
      if ((inetdflag && cmd->nflag))
	{			/* netid not allowed with inetdflag */
	  fprintf (stderr, _("Cannot use netid flag with inetd flag!\n"));
	  return 0;
	}
    }
  else
    {				/* 4.1 mode */
      pmflag = 0;		/* set pmflag only in tirpcmode */
      if (cmd->nflag)
	{			/* netid needs TIRPC */
	  f_print (stderr, _("Cannot use netid flag without TIRPC!\n"));
	  return (0);
	}
    }

  if (newstyle && (tblflag || cmd->tflag))
    {
      f_print (stderr, _("Cannot use table flags with newstyle!\n"));
      return (0);
    }

  /* check no conflicts with file generation flags */
  nflags = cmd->cflag + cmd->hflag + cmd->lflag + cmd->mflag +
    cmd->sflag + cmd->nflag + cmd->tflag + cmd->Ssflag + cmd->Scflag;

  if (nflags == 0)
    {
      if (cmd->outfile != NULL || cmd->infile == NULL)
	{
	  return (0);
	}
    }
  else if (cmd->infile == NULL &&
	   (cmd->Ssflag || cmd->Scflag || cmd->makefileflag))
    {
      fprintf (stderr,
	       _("\"infile\" is required for template generation flags.\n"));
      return 0;
    }
  if (nflags > 1)
    {
      fprintf (stderr, _("Cannot have more than one file generation flag!\n"));
      return 0;
    }
  return 1;
}

static void
usage (FILE *stream, int status)
{
  fprintf (stream, _("usage: %s infile\n"), cmdname);
  fprintf (stream, _("\t%s [-abkCLNTM][-Dname[=value]] [-i size] \
[-I [-K seconds]] [-Y path] infile\n"), cmdname);
  fprintf (stream, _("\t%s [-c | -h | -l | -m | -t | -Sc | -Ss | -Sm] \
[-o outfile] [infile]\n"), cmdname);
  fprintf (stream, _("\t%s [-s nettype]* [-o outfile] [infile]\n"), cmdname);
  fprintf (stream, _("\t%s [-n netid]* [-o outfile] [infile]\n"), cmdname);
  options_usage (stream, status);
  exit (status);
}

static void
options_usage (FILE *stream, int status)
{
  f_print (stream, _("options:\n"));
  f_print (stream, _("-a\t\tgenerate all files, including samples\n"));
  f_print (stream, _("-b\t\tbackward compatibility mode (generates code for SunOS 4.1)\n"));
  f_print (stream, _("-c\t\tgenerate XDR routines\n"));
  f_print (stream, _("-C\t\tANSI C mode\n"));
  f_print (stream, _("-Dname[=value]\tdefine a symbol (same as #define)\n"));
  f_print (stream, _("-h\t\tgenerate header file\n"));
  f_print (stream, _("-i size\t\tsize at which to start generating inline code\n"));
  f_print (stream, _("-I\t\tgenerate code for inetd support in server (for SunOS 4.1)\n"));
  f_print (stream, _("-K seconds\tserver exits after K seconds of inactivity\n"));
  f_print (stream, _("-l\t\tgenerate client side stubs\n"));
  f_print (stream, _("-L\t\tserver errors will be printed to syslog\n"));
  f_print (stream, _("-m\t\tgenerate server side stubs\n"));
  f_print (stream, _("-M\t\tgenerate MT-safe code\n"));
  f_print (stream, _("-n netid\tgenerate server code that supports named netid\n"));
  f_print (stream, _("-N\t\tsupports multiple arguments and call-by-value\n"));
  f_print (stream, _("-o outfile\tname of the output file\n"));
  f_print (stream, _("-s nettype\tgenerate server code that supports named nettype\n"));
  f_print (stream, _("-Sc\t\tgenerate sample client code that uses remote procedures\n"));
  f_print (stream, _("-Ss\t\tgenerate sample server code that defines remote procedures\n"));
  f_print (stream, _("-Sm \t\tgenerate makefile template \n"));
  f_print (stream, _("-t\t\tgenerate RPC dispatch table\n"));
  f_print (stream, _("-T\t\tgenerate code to support RPC dispatch tables\n"));
  f_print (stream, _("-Y path\t\tdirectory name to find C preprocessor (cpp)\n"));

  f_print (stream, _("\n\
For bug reporting instructions, please see:\n\
%s.\n"), REPORT_BUGS_TO);
  exit (status);
}

static void
print_version (void)
{
  printf ("rpcgen %s%s\n", PKGVERSION, VERSION);
  exit (0);
}
