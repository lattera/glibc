/* @(#)clnt_perror.c	2.1 88/07/29 4.0 RPCSRC */
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
static char sccsid[] = "@(#)clnt_perror.c 1.15 87/10/07 Copyr 1984 Sun Micro";
#endif

/*
 * clnt_perror.c
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 */
#include <stdio.h>
#include <string.h>
#include <rpc/types.h>
#include <rpc/auth.h>
#include <rpc/clnt.h>

#ifdef USE_IN_LIBIO
# include <libio/iolibio.h>
# define fputs(s, f) _IO_fputs (s, f)
#endif

static char *auth_errmsg (enum auth_stat stat) internal_function;

static char *buf;

static char *
_buf (void)
{
  if (buf == NULL)
    buf = (char *) malloc (256);
  return buf;
}

/*
 * Print reply error info
 */
char *
clnt_sperror (CLIENT * rpch, const char *msg)
{
  char buf[1024];
  struct rpc_err e;
  char *err;
  char *str = _buf ();
  char *strstart = str;
  int len;

  if (str == NULL)
    return NULL;
  CLNT_GETERR (rpch, &e);

  len = sprintf (str, "%s: ", msg);
  str += len;

  str = stpcpy (str, clnt_sperrno (e.re_status));

  switch (e.re_status)
    {
    case RPC_SUCCESS:
    case RPC_CANTENCODEARGS:
    case RPC_CANTDECODERES:
    case RPC_TIMEDOUT:
    case RPC_PROGUNAVAIL:
    case RPC_PROCUNAVAIL:
    case RPC_CANTDECODEARGS:
    case RPC_SYSTEMERROR:
    case RPC_UNKNOWNHOST:
    case RPC_UNKNOWNPROTO:
    case RPC_PMAPFAILURE:
    case RPC_PROGNOTREGISTERED:
    case RPC_FAILED:
      break;

    case RPC_CANTSEND:
    case RPC_CANTRECV:
      len = sprintf (str, "; errno = %s", __strerror_r (e.re_errno,
							buf, sizeof buf));
      str += len;
      break;

    case RPC_VERSMISMATCH:
      len= sprintf (str, _("; low version = %lu, high version = %lu"),
		    e.re_vers.low, e.re_vers.high);
      str += len;
      break;

    case RPC_AUTHERROR:
      err = auth_errmsg (e.re_why);
      str = stpcpy (str, _ ("; why = "));
      if (err != NULL)
	{
	  str = stpcpy (str, err);
	}
      else
	{
	  len = sprintf (str, _("(unknown authentication error - %d)"),
			 (int) e.re_why);
	  str += len;
	}
      break;

    case RPC_PROGVERSMISMATCH:
      len = sprintf (str, _("; low version = %lu, high version = %lu"),
		     e.re_vers.low, e.re_vers.high);
      str += len;
      break;

    default:			/* unknown */
      len = sprintf (str, "; s1 = %lu, s2 = %lu", e.re_lb.s1, e.re_lb.s2);
      str += len;
      break;
    }
  *str = '\n';
  *++str = '\0';
  return (strstart);
}

void
clnt_perror (CLIENT * rpch, const char *msg)
{
  (void) fputs (clnt_sperror (rpch, msg), stderr);
}


struct rpc_errtab
{
  enum clnt_stat status;
  const char *message;
};

static const struct rpc_errtab rpc_errlist[] =
{
  {RPC_SUCCESS,
   N_("RPC: Success")},
  {RPC_CANTENCODEARGS,
   N_("RPC: Can't encode arguments")},
  {RPC_CANTDECODERES,
   N_("RPC: Can't decode result")},
  {RPC_CANTSEND,
   N_("RPC: Unable to send")},
  {RPC_CANTRECV,
   N_("RPC: Unable to receive")},
  {RPC_TIMEDOUT,
   N_("RPC: Timed out")},
  {RPC_VERSMISMATCH,
   N_("RPC: Incompatible versions of RPC")},
  {RPC_AUTHERROR,
   N_("RPC: Authentication error")},
  {RPC_PROGUNAVAIL,
   N_("RPC: Program unavailable")},
  {RPC_PROGVERSMISMATCH,
   N_("RPC: Program/version mismatch")},
  {RPC_PROCUNAVAIL,
   N_("RPC: Procedure unavailable")},
  {RPC_CANTDECODEARGS,
   N_("RPC: Server can't decode arguments")},
  {RPC_SYSTEMERROR,
   N_("RPC: Remote system error")},
  {RPC_UNKNOWNHOST,
   N_("RPC: Unknown host")},
  {RPC_UNKNOWNPROTO,
   N_("RPC: Unknown protocol")},
  {RPC_PMAPFAILURE,
   N_("RPC: Port mapper failure")},
  {RPC_PROGNOTREGISTERED,
   N_("RPC: Program not registered")},
  {RPC_FAILED,
   N_("RPC: Failed (unspecified error)")}
};


/*
 * This interface for use by clntrpc
 */
char *
clnt_sperrno (enum clnt_stat stat)
{
  size_t i;

  for (i = 0; i < sizeof (rpc_errlist) / sizeof (struct rpc_errtab); i++)
    {
      if (rpc_errlist[i].status == stat)
	{
	  return _(rpc_errlist[i].message);
	}
    }
  return _("RPC: (unknown error code)");
}

void
clnt_perrno (enum clnt_stat num)
{
  (void) fputs (clnt_sperrno (num), stderr);
}


char *
clnt_spcreateerror (const char *msg)
{
  char buf[1024];
  char *str = _buf ();
  char *cp;
  int len;

  if (str == NULL)
    return NULL;
  len = sprintf (str, "%s: ", msg);
  cp = str + len;
  cp = stpcpy (cp, clnt_sperrno (rpc_createerr.cf_stat));
  switch (rpc_createerr.cf_stat)
    {
    case RPC_PMAPFAILURE:
      cp = stpcpy (stpcpy (cp, " - "),
		   clnt_sperrno (rpc_createerr.cf_error.re_status));
      break;

    case RPC_SYSTEMERROR:
      cp = stpcpy (stpcpy (cp, " - "),
		   __strerror_r (rpc_createerr.cf_error.re_errno,
				 buf, sizeof buf));
      break;
    default:
      break;
    }
  *cp = '\n';
  *++cp = '\0';
  return str;
}

void
clnt_pcreateerror (const char *msg)
{
  (void) fputs (clnt_spcreateerror (msg), stderr);
}

struct auth_errtab
{
  enum auth_stat status;
  const char *message;
};

static const struct auth_errtab auth_errlist[] =
{
  {AUTH_OK,
   N_("Authentication OK")},
  {AUTH_BADCRED,
   N_("Invalid client credential")},
  {AUTH_REJECTEDCRED,
   N_("Server rejected credential")},
  {AUTH_BADVERF,
   N_("Invalid client verifier")},
  {AUTH_REJECTEDVERF,
   N_("Server rejected verifier")},
  {AUTH_TOOWEAK,
   N_("Client credential too weak")},
  {AUTH_INVALIDRESP,
   N_("Invalid server verifier")},
  {AUTH_FAILED,
   N_("Failed (unspecified error)")},
};

static char *
internal_function
auth_errmsg (enum auth_stat stat)
{
  size_t i;

  for (i = 0; i < sizeof (auth_errlist) / sizeof (struct auth_errtab); i++)
    {
      if (auth_errlist[i].status == stat)
	{
	  return _(auth_errlist[i].message);
	}
    }
  return NULL;
}
