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
#include <libintl.h>
#include <rpc/rpc.h>

#ifdef USE_IN_LIBIO
# include <wchar.h>
# include <libio/iolibio.h>
#endif

static char *auth_errmsg (enum auth_stat stat) internal_function;

#ifdef _RPC_THREAD_SAFE_
/*
 * Making buf a preprocessor macro requires renaming the local
 * buf variable in a few functions.  Overriding a global variable
 * with a local variable of the same name is a bad idea, anyway.
 */
#define buf RPC_THREAD_VARIABLE(clnt_perr_buf_s)
#else
static char *buf;
#endif

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
  char chrbuf[1024];
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
							chrbuf, sizeof chrbuf));
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
libc_hidden_def (clnt_sperror)

void
clnt_perror (CLIENT * rpch, const char *msg)
{
#ifdef USE_IN_LIBIO
  if (_IO_fwide (stderr, 0) > 0)
    (void) __fwprintf (stderr, L"%s", clnt_sperror (rpch, msg));
  else
#endif
    (void) fputs (clnt_sperror (rpch, msg), stderr);
}
libc_hidden_def (clnt_perror)


struct rpc_errtab
{
  enum clnt_stat status;
  unsigned int message_off;
};

static const char rpc_errstr[] =
{
#define RPC_SUCCESS_IDX		0
  N_("RPC: Success")
  "\0"
#define RPC_CANTENCODEARGS_IDX	(RPC_SUCCESS_IDX + sizeof "RPC: Success")
  N_("RPC: Can't encode arguments")
  "\0"
#define RPC_CANTDECODERES_IDX	(RPC_CANTENCODEARGS_IDX \
				 + sizeof "RPC: Can't encode arguments")
  N_("RPC: Can't decode result")
  "\0"
#define RPC_CANTSEND_IDX	(RPC_CANTDECODERES_IDX \
				 + sizeof "RPC: Can't decode result")
  N_("RPC: Unable to send")
  "\0"
#define RPC_CANTRECV_IDX	(RPC_CANTSEND_IDX \
				 + sizeof "RPC: Unable to send")
  N_("RPC: Unable to receive")
  "\0"
#define RPC_TIMEDOUT_IDX	(RPC_CANTRECV_IDX \
				 + sizeof "RPC: Unable to receive")
  N_("RPC: Timed out")
  "\0"
#define RPC_VERSMISMATCH_IDX	(RPC_TIMEDOUT_IDX \
				 + sizeof "RPC: Timed out")
  N_("RPC: Incompatible versions of RPC")
  "\0"
#define RPC_AUTHERROR_IDX	(RPC_VERSMISMATCH_IDX \
				 + sizeof "RPC: Incompatible versions of RPC")
  N_("RPC: Authentication error")
  "\0"
#define RPC_PROGUNAVAIL_IDX		(RPC_AUTHERROR_IDX \
				 + sizeof "RPC: Authentication error")
  N_("RPC: Program unavailable")
  "\0"
#define RPC_PROGVERSMISMATCH_IDX (RPC_PROGUNAVAIL_IDX \
				  + sizeof "RPC: Program unavailable")
  N_("RPC: Program/version mismatch")
  "\0"
#define RPC_PROCUNAVAIL_IDX	(RPC_PROGVERSMISMATCH_IDX \
				 + sizeof "RPC: Program/version mismatch")
  N_("RPC: Procedure unavailable")
  "\0"
#define RPC_CANTDECODEARGS_IDX	(RPC_PROCUNAVAIL_IDX \
				 + sizeof "RPC: Procedure unavailable")
  N_("RPC: Server can't decode arguments")
  "\0"
#define RPC_SYSTEMERROR_IDX	(RPC_CANTDECODEARGS_IDX \
				 + sizeof "RPC: Server can't decode arguments")
  N_("RPC: Remote system error")
  "\0"
#define RPC_UNKNOWNHOST_IDX	(RPC_SYSTEMERROR_IDX \
				 + sizeof "RPC: Remote system error")
  N_("RPC: Unknown host")
  "\0"
#define RPC_UNKNOWNPROTO_IDX	(RPC_UNKNOWNHOST_IDX \
				 + sizeof "RPC: Unknown host")
  N_("RPC: Unknown protocol")
  "\0"
#define RPC_PMAPFAILURE_IDX	(RPC_UNKNOWNPROTO_IDX \
				 + sizeof "RPC: Unknown protocol")
  N_("RPC: Port mapper failure")
  "\0"
#define RPC_PROGNOTREGISTERED_IDX (RPC_PMAPFAILURE_IDX \
				   + sizeof "RPC: Port mapper failure")
  N_("RPC: Program not registered")
  "\0"
#define RPC_FAILED_IDX		(RPC_PROGNOTREGISTERED_IDX \
				 + sizeof "RPC: Program not registered")
  N_("RPC: Failed (unspecified error)")
};

static const struct rpc_errtab rpc_errlist[] =
{
  { RPC_SUCCESS, RPC_SUCCESS_IDX },
  { RPC_CANTENCODEARGS, RPC_CANTENCODEARGS_IDX },
  { RPC_CANTDECODERES, RPC_CANTDECODERES_IDX },
  { RPC_CANTSEND, RPC_CANTSEND_IDX },
  { RPC_CANTRECV, RPC_CANTRECV_IDX },
  { RPC_TIMEDOUT, RPC_TIMEDOUT_IDX },
  { RPC_VERSMISMATCH, RPC_VERSMISMATCH_IDX },
  { RPC_AUTHERROR, RPC_AUTHERROR_IDX },
  { RPC_PROGUNAVAIL, RPC_PROGUNAVAIL_IDX },
  { RPC_PROGVERSMISMATCH, RPC_PROGVERSMISMATCH_IDX },
  { RPC_PROCUNAVAIL, RPC_PROCUNAVAIL_IDX },
  { RPC_CANTDECODEARGS, RPC_CANTDECODEARGS_IDX },
  { RPC_SYSTEMERROR, RPC_SYSTEMERROR_IDX },
  { RPC_UNKNOWNHOST, RPC_UNKNOWNHOST_IDX },
  { RPC_UNKNOWNPROTO, RPC_UNKNOWNPROTO_IDX },
  { RPC_PMAPFAILURE, RPC_PMAPFAILURE_IDX },
  { RPC_PROGNOTREGISTERED, RPC_PROGNOTREGISTERED_IDX },
  { RPC_FAILED, RPC_FAILED_IDX }
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
	  return _(rpc_errstr + rpc_errlist[i].message_off);
	}
    }
  return _("RPC: (unknown error code)");
}
libc_hidden_def (clnt_sperrno)

void
clnt_perrno (enum clnt_stat num)
{
#ifdef USE_IN_LIBIO
  if (_IO_fwide (stderr, 0) > 0)
    (void) __fwprintf (stderr, L"%s", clnt_sperrno (num));
  else
#endif
    (void) fputs (clnt_sperrno (num), stderr);
}


char *
clnt_spcreateerror (const char *msg)
{
  char chrbuf[1024];
  char *str = _buf ();
  char *cp;
  int len;
  struct rpc_createerr *ce;

  if (str == NULL)
    return NULL;
  ce = &get_rpc_createerr ();
  len = sprintf (str, "%s: ", msg);
  cp = str + len;
  cp = stpcpy (cp, clnt_sperrno (ce->cf_stat));
  switch (ce->cf_stat)
    {
    case RPC_PMAPFAILURE:
      cp = stpcpy (stpcpy (cp, " - "),
		   clnt_sperrno (ce->cf_error.re_status));
      break;

    case RPC_SYSTEMERROR:
      cp = stpcpy (stpcpy (cp, " - "),
		   __strerror_r (ce->cf_error.re_errno,
				 chrbuf, sizeof chrbuf));
      break;
    default:
      break;
    }
  *cp = '\n';
  *++cp = '\0';
  return str;
}
libc_hidden_def (clnt_spcreateerror)

void
clnt_pcreateerror (const char *msg)
{
#ifdef USE_IN_LIBIO
  if (_IO_fwide (stderr, 0) > 0)
    (void) __fwprintf (stderr, L"%s", clnt_spcreateerror (msg));
  else
#endif
    (void) fputs (clnt_spcreateerror (msg), stderr);
}

struct auth_errtab
{
  enum auth_stat status;
  unsigned int message_off;
};

static const char auth_errstr[] =
{
#define AUTH_OK_IDX		0
   N_("Authentication OK")
   "\0"
#define AUTH_BADCRED_IDX	(AUTH_OK_IDX + sizeof "Authentication OK")
   N_("Invalid client credential")
   "\0"
#define AUTH_REJECTEDCRED_IDX	(AUTH_BADCRED_IDX \
				 + sizeof "Invalid client credential")
   N_("Server rejected credential")
   "\0"
#define AUTH_BADVERF_IDX	(AUTH_REJECTEDCRED_IDX \
				 + sizeof "Server rejected credential")
   N_("Invalid client verifier")
   "\0"
#define AUTH_REJECTEDVERF_IDX	(AUTH_BADVERF_IDX \
				 + sizeof "Invalid client verifier")
   N_("Server rejected verifier")
   "\0"
#define AUTH_TOOWEAK_IDX	(AUTH_REJECTEDVERF_IDX \
				 + sizeof "Server rejected verifier")
   N_("Client credential too weak")
   "\0"
#define AUTH_INVALIDRESP_IDX	(AUTH_TOOWEAK_IDX \
				 + sizeof "Client credential too weak")
   N_("Invalid server verifier")
   "\0"
#define AUTH_FAILED_IDX		(AUTH_INVALIDRESP_IDX \
				 + sizeof "Invalid server verifier")
   N_("Failed (unspecified error)")
};

static const struct auth_errtab auth_errlist[] =
{
  { AUTH_OK, AUTH_OK_IDX },
  { AUTH_BADCRED, AUTH_BADCRED_IDX },
  { AUTH_REJECTEDCRED, AUTH_REJECTEDCRED_IDX },
  { AUTH_BADVERF, AUTH_BADVERF_IDX },
  { AUTH_REJECTEDVERF, AUTH_REJECTEDVERF_IDX },
  { AUTH_TOOWEAK, AUTH_TOOWEAK_IDX },
  { AUTH_INVALIDRESP, AUTH_INVALIDRESP_IDX },
  { AUTH_FAILED, AUTH_FAILED_IDX }
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
	  return _(auth_errstr + auth_errlist[i].message_off);
	}
    }
  return NULL;
}


libc_freeres_fn (free_mem)
{
  /* Not libc_freeres_ptr, since buf is a macro.  */
  free (buf);
}
