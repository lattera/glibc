/* @(#)svc_simple.c	2.2 88/08/01 4.0 RPCSRC */
/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
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
#if !defined(lint) && defined(SCCSIDS)
static char sccsid[] = "@(#)svc_simple.c 1.18 87/08/11 Copyr 1984 Sun Micro";
#endif

/*
 * svc_simple.c
 * Simplified front end to rpc.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <rpc/pmap_clnt.h>
#include <sys/socket.h>
#include <netdb.h>

#ifdef USE_IN_LIBIO
# include <wchar.h>
# include <libio/iolibio.h>
#endif

struct proglst_
  {
    char *(*p_progname) (char *);
    int p_prognum;
    int p_procnum;
    xdrproc_t p_inproc, p_outproc;
    struct proglst_ *p_nxt;
  };
#ifdef _RPC_THREAD_SAFE_
#define proglst RPC_THREAD_VARIABLE(svcsimple_proglst_s)
#else
static struct proglst_ *proglst;
#endif


static void universal (struct svc_req *rqstp, SVCXPRT *transp_s);
#ifdef _RPC_THREAD_SAFE_
#define transp RPC_THREAD_VARIABLE(svcsimple_transp_s)
#else
static SVCXPRT *transp;
#endif

int
registerrpc (u_long prognum, u_long versnum, u_long procnum,
	     char *(*progname) (char *), xdrproc_t inproc, xdrproc_t outproc)
{
  struct proglst_ *pl;
  char *buf;

  if (procnum == NULLPROC)
    {

      if (__asprintf (&buf, _("can't reassign procedure number %ld\n"),
		      NULLPROC) < 0)
	buf = NULL;
      goto err_out;
    }
  if (transp == 0)
    {
      transp = INTUSE(svcudp_create) (RPC_ANYSOCK);
      if (transp == NULL)
	{
	  buf = strdup (_("couldn't create an rpc server\n"));
	  goto err_out;
	}
    }
  (void) pmap_unset ((u_long) prognum, (u_long) versnum);
  if (!svc_register (transp, (u_long) prognum, (u_long) versnum,
		     universal, IPPROTO_UDP))
    {
      if (__asprintf (&buf, _("couldn't register prog %ld vers %ld\n"),
		      prognum, versnum) < 0)
	buf = NULL;
      goto err_out;
    }
  pl = (struct proglst_ *) malloc (sizeof (struct proglst_));
  if (pl == NULL)
    {
      buf = strdup (_("registerrpc: out of memory\n"));
      goto err_out;
    }
  pl->p_progname = progname;
  pl->p_prognum = prognum;
  pl->p_procnum = procnum;
  pl->p_inproc = inproc;
  pl->p_outproc = outproc;
  pl->p_nxt = proglst;
  proglst = pl;
  return 0;

 err_out:
  if (buf == NULL)
    return -1;
#ifdef USE_IN_LIBIO
  if (_IO_fwide (stderr, 0) > 0)
    (void) __fwprintf (stderr, L"%s", buf);
  else
#endif
    (void) fputs (buf, stderr);
  free (buf);
  return -1;
}

static void
universal (struct svc_req *rqstp, SVCXPRT *transp_l)
{
  int prog, proc;
  char *outdata;
  char xdrbuf[UDPMSGSIZE];
  struct proglst_ *pl;
  char *buf = NULL;

  /*
   * enforce "procnum 0 is echo" convention
   */
  if (rqstp->rq_proc == NULLPROC)
    {
      if (INTUSE(svc_sendreply) (transp_l, (xdrproc_t)INTUSE(xdr_void),
				 (char *) NULL) == FALSE)
	{
	  __write (STDERR_FILENO, "xxx\n", 4);
	  exit (1);
	}
      return;
    }
  prog = rqstp->rq_prog;
  proc = rqstp->rq_proc;
  for (pl = proglst; pl != NULL; pl = pl->p_nxt)
    if (pl->p_prognum == prog && pl->p_procnum == proc)
      {
	/* decode arguments into a CLEAN buffer */
	__bzero (xdrbuf, sizeof (xdrbuf));	/* required ! */
	if (!svc_getargs (transp_l, pl->p_inproc, xdrbuf))
	  {
	    INTUSE(svcerr_decode) (transp_l);
	    return;
	  }
	outdata = (*(pl->p_progname)) (xdrbuf);
	if (outdata == NULL && pl->p_outproc != (xdrproc_t)INTUSE(xdr_void))
	  /* there was an error */
	  return;
	if (!INTUSE(svc_sendreply) (transp_l, pl->p_outproc, outdata))
	  {
	    if (__asprintf (&buf, _("trouble replying to prog %d\n"),
			    pl->p_prognum) < 0)
	      buf = NULL;
	    goto err_out2;
	  }
	/* free the decoded arguments */
	(void) svc_freeargs (transp_l, pl->p_inproc, xdrbuf);
	return;
      }
  if (__asprintf (&buf, _("never registered prog %d\n"), prog) < 0)
    buf = NULL;
 err_out2:
  if (buf == NULL)
    exit (1);
#ifdef USE_IN_LIBIO
  if (_IO_fwide (stderr, 0) > 0)
    __fwprintf (stderr, L"%s", buf);
  else
#endif
    fputs (buf, stderr);
  free (buf);
  exit (1);
}
