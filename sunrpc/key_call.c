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
/*
 * Copyright (c) 1988 by Sun Microsystems, Inc.
 */
/*
 * The original source is from the RPCSRC 4.0 package from Sun Microsystems.
 * The Interface to keyserver protocoll 2, RPC over AF_UNIX and Linux/doors
 * was added by Thorsten Kukuk <kukuk@suse.de>
 * Since the Linux/doors project was stopped, I doubt that this code will
 * ever be useful <kukuk@suse.de>.
 */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <rpc/rpc.h>
#include <rpc/auth.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <rpc/key_prot.h>
#include <bits/libc-lock.h>

#ifdef HAVE_DOORS
# include "door/door.h"
#endif

#define KEY_TIMEOUT	5	/* per-try timeout in seconds */
#define KEY_NRETRY	12	/* number of retries */

#define debug(msg)		/* turn off debugging */

#ifndef SO_PASSCRED
extern int _openchild (const char *command, FILE **fto, FILE **ffrom);
#endif

static int key_call (u_long, xdrproc_t xdr_arg, char *,
		     xdrproc_t xdr_rslt, char *) internal_function;

static const struct timeval trytimeout = {KEY_TIMEOUT, 0};
static const struct timeval tottimeout = {KEY_TIMEOUT *KEY_NRETRY, 0};

int
key_setsecret (char *secretkey)
{
  keystatus status;

  if (!key_call ((u_long) KEY_SET, (xdrproc_t) INTUSE(xdr_keybuf), secretkey,
		 (xdrproc_t) INTUSE(xdr_keystatus), (char *) &status))
    return -1;
  if (status != KEY_SUCCESS)
    {
      debug ("set status is nonzero");
      return -1;
    }
  return 0;
}

/* key_secretkey_is_set() returns 1 if the keyserver has a secret key
 * stored for the caller's effective uid; it returns 0 otherwise
 *
 * N.B.:  The KEY_NET_GET key call is undocumented.  Applications shouldn't
 * be using it, because it allows them to get the user's secret key.
 */
int
key_secretkey_is_set (void)
{
  struct key_netstres kres;

  memset (&kres, 0, sizeof (kres));
  if (key_call ((u_long) KEY_NET_GET, (xdrproc_t) INTUSE(xdr_void),
		(char *) NULL, (xdrproc_t) INTUSE(xdr_key_netstres),
		(char *) &kres) &&
      (kres.status == KEY_SUCCESS) &&
      (kres.key_netstres_u.knet.st_priv_key[0] != 0))
    {
      /* avoid leaving secret key in memory */
      memset (kres.key_netstres_u.knet.st_priv_key, 0, HEXKEYBYTES);
      return 1;
    }
  return 0;
}

int
key_encryptsession (char *remotename, des_block *deskey)
{
  cryptkeyarg arg;
  cryptkeyres res;

  arg.remotename = remotename;
  arg.deskey = *deskey;
  if (!key_call ((u_long) KEY_ENCRYPT, (xdrproc_t) INTUSE(xdr_cryptkeyarg),
		 (char *) &arg, (xdrproc_t) INTUSE(xdr_cryptkeyres),
		 (char *) &res))
    return -1;

  if (res.status != KEY_SUCCESS)
    {
      debug ("encrypt status is nonzero");
      return -1;
    }
  *deskey = res.cryptkeyres_u.deskey;
  return 0;
}

int
key_decryptsession (char *remotename, des_block *deskey)
{
  cryptkeyarg arg;
  cryptkeyres res;

  arg.remotename = remotename;
  arg.deskey = *deskey;
  if (!key_call ((u_long) KEY_DECRYPT, (xdrproc_t) INTUSE(xdr_cryptkeyarg),
		 (char *) &arg, (xdrproc_t) INTUSE(xdr_cryptkeyres),
		 (char *) &res))
    return -1;
  if (res.status != KEY_SUCCESS)
    {
      debug ("decrypt status is nonzero");
      return -1;
    }
  *deskey = res.cryptkeyres_u.deskey;
  return 0;
}

int
key_encryptsession_pk (char *remotename, netobj *remotekey,
		       des_block *deskey)
{
  cryptkeyarg2 arg;
  cryptkeyres res;

  arg.remotename = remotename;
  arg.remotekey = *remotekey;
  arg.deskey = *deskey;
  if (!key_call ((u_long) KEY_ENCRYPT_PK, (xdrproc_t) INTUSE(xdr_cryptkeyarg2),
		 (char *) &arg, (xdrproc_t) INTUSE(xdr_cryptkeyres),
		 (char *) &res))
    return -1;

  if (res.status != KEY_SUCCESS)
    {
      debug ("encrypt status is nonzero");
      return -1;
    }
  *deskey = res.cryptkeyres_u.deskey;
  return 0;
}
libc_hidden_def (key_encryptsession_pk)

int
key_decryptsession_pk (char *remotename, netobj *remotekey,
		       des_block *deskey)
{
  cryptkeyarg2 arg;
  cryptkeyres res;

  arg.remotename = remotename;
  arg.remotekey = *remotekey;
  arg.deskey = *deskey;
  if (!key_call ((u_long) KEY_DECRYPT_PK, (xdrproc_t) INTUSE(xdr_cryptkeyarg2),
		 (char *) &arg, (xdrproc_t) INTUSE(xdr_cryptkeyres),
		 (char *) &res))
    return -1;

  if (res.status != KEY_SUCCESS)
    {
      debug ("decrypt status is nonzero");
      return -1;
    }
  *deskey = res.cryptkeyres_u.deskey;
  return 0;
}
libc_hidden_def (key_decryptsession_pk)

int
key_gendes (des_block *key)
{
  struct sockaddr_in sin;
  CLIENT *client;
  int socket;
  enum clnt_stat stat;

  sin.sin_family = AF_INET;
  sin.sin_port = 0;
  sin.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
  __bzero (sin.sin_zero, sizeof (sin.sin_zero));
  socket = RPC_ANYSOCK;
  client = INTUSE(clntudp_bufcreate) (&sin, (u_long) KEY_PROG,
				      (u_long) KEY_VERS, trytimeout, &socket,
				      RPCSMALLMSGSIZE, RPCSMALLMSGSIZE);
  if (client == NULL)
    return -1;

  stat = clnt_call (client, KEY_GEN, (xdrproc_t) INTUSE(xdr_void), NULL,
		    (xdrproc_t) INTUSE(xdr_des_block), (caddr_t) key,
		    tottimeout);
  clnt_destroy (client);
  __close (socket);
  if (stat != RPC_SUCCESS)
    return -1;

  return 0;
}
libc_hidden_def (key_gendes)

int
key_setnet (struct key_netstarg *arg)
{
  keystatus status;

  if (!key_call ((u_long) KEY_NET_PUT, (xdrproc_t) INTUSE(xdr_key_netstarg),
		 (char *) arg,(xdrproc_t) INTUSE(xdr_keystatus),
		 (char *) &status))
    return -1;

  if (status != KEY_SUCCESS)
    {
      debug ("key_setnet status is nonzero");
      return -1;
    }
  return 1;
}

int
key_get_conv (char *pkey, des_block *deskey)
{
  cryptkeyres res;

  if (!key_call ((u_long) KEY_GET_CONV, (xdrproc_t) INTUSE(xdr_keybuf), pkey,
		 (xdrproc_t) INTUSE(xdr_cryptkeyres), (char *) &res))
    return -1;

  if (res.status != KEY_SUCCESS)
    {
      debug ("get_conv status is nonzero");
      return -1;
    }
  *deskey = res.cryptkeyres_u.deskey;
  return 0;
}

/*
 * Hack to allow the keyserver to use AUTH_DES (for authenticated
 * NIS+ calls, for example).  The only functions that get called
 * are key_encryptsession_pk, key_decryptsession_pk, and key_gendes.
 *
 * The approach is to have the keyserver fill in pointers to local
 * implementations of these functions, and to call those in key_call().
 */

cryptkeyres *(*__key_encryptsession_pk_LOCAL) (uid_t, char *);
cryptkeyres *(*__key_decryptsession_pk_LOCAL) (uid_t, char *);
des_block *(*__key_gendes_LOCAL) (uid_t, char *);

#ifndef SO_PASSCRED
static int
internal_function
key_call_keyenvoy (u_long proc, xdrproc_t xdr_arg, char *arg,
		   xdrproc_t xdr_rslt, char *rslt)
{
  XDR xdrargs;
  XDR xdrrslt;
  FILE *fargs;
  FILE *frslt;
  sigset_t oldmask, mask;
  union wait status;
  int pid;
  int success;
  uid_t ruid;
  uid_t euid;
  static const char MESSENGER[] = "/usr/etc/keyenvoy";

  success = 1;
  sigemptyset (&mask);
  sigaddset (&mask, SIGCHLD);
  __sigprocmask (SIG_BLOCK, &mask, &oldmask);

  /*
   * We are going to exec a set-uid program which makes our effective uid
   * zero, and authenticates us with our real uid. We need to make the
   * effective uid be the real uid for the setuid program, and
   * the real uid be the effective uid so that we can change things back.
   */
  euid = __geteuid ();
  ruid = __getuid ();
  __setreuid (euid, ruid);
  pid = _openchild (MESSENGER, &fargs, &frslt);
  __setreuid (ruid, euid);
  if (pid < 0)
    {
      debug ("open_streams");
      __sigprocmask (SIG_SETMASK, &oldmask, NULL);
      return (0);
    }
  xdrstdio_create (&xdrargs, fargs, XDR_ENCODE);
  xdrstdio_create (&xdrrslt, frslt, XDR_DECODE);

  if (!INTUSE(xdr_u_long) (&xdrargs, &proc) || !(*xdr_arg) (&xdrargs, arg))
    {
      debug ("xdr args");
      success = 0;
    }
  fclose (fargs);

  if (success && !(*xdr_rslt) (&xdrrslt, rslt))
    {
      debug ("xdr rslt");
      success = 0;
    }
  fclose(frslt);

 wait_again:
  if (__wait4 (pid, &status, 0, NULL) < 0)
    {
      if (errno == EINTR)
	goto wait_again;
      debug ("wait4");
      if (errno == ECHILD || errno == ESRCH)
	perror ("wait");
      else
	success = 0;
    }
  else
    if (status.w_retcode)
      {
	debug ("wait4 1");
	success = 0;
      }
  __sigprocmask (SIG_SETMASK, &oldmask, NULL);

  return success;
}
#endif

struct  key_call_private {
  CLIENT  *client;        /* Client handle */
  pid_t   pid;            /* process-id at moment of creation */
  uid_t   uid;            /* user-id at last authorization */
};
#ifdef _RPC_THREAD_SAFE_
#define key_call_private_main RPC_THREAD_VARIABLE(key_call_private_s)
#else
static struct key_call_private *key_call_private_main;
#endif
__libc_lock_define_initialized (static, keycall_lock)

/*
 * Keep the handle cached.  This call may be made quite often.
 */
static CLIENT *
getkeyserv_handle (int vers)
{
  struct key_call_private *kcp = key_call_private_main;
  struct timeval wait_time;
  int fd;
  struct sockaddr_un name;
  int namelen = sizeof(struct sockaddr_un);

#define TOTAL_TIMEOUT   30      /* total timeout talking to keyserver */
#define TOTAL_TRIES     5       /* Number of tries */

  if (kcp == (struct key_call_private *)NULL)
    {
      kcp = (struct key_call_private *)malloc (sizeof (*kcp));
      if (kcp == (struct key_call_private *)NULL)
	return (CLIENT *) NULL;

      key_call_private_main = kcp;
      kcp->client = NULL;
    }

  /* if pid has changed, destroy client and rebuild */
  if (kcp->client != NULL && kcp->pid != __getpid ())
    {
      clnt_destroy (kcp->client);
      kcp->client = NULL;
    }

  if (kcp->client != NULL)
    {
      /* if other side closed socket, build handle again */
      clnt_control (kcp->client, CLGET_FD, (char *)&fd);
      if (__getpeername (fd,(struct sockaddr *)&name,&namelen) == -1)
	{
	  auth_destroy (kcp->client->cl_auth);
	  clnt_destroy (kcp->client);
	  kcp->client = NULL;
	}
    }

  if (kcp->client != NULL)
    {
      /* if uid has changed, build client handle again */
      if (kcp->uid != __geteuid ())
	{
        kcp->uid = __geteuid ();
        auth_destroy (kcp->client->cl_auth);
        kcp->client->cl_auth =
          INTUSE(authunix_create) ((char *)"", kcp->uid, 0, 0, NULL);
        if (kcp->client->cl_auth == NULL)
          {
            clnt_destroy (kcp->client);
            kcp->client = NULL;
            return ((CLIENT *) NULL);
          }
	}
      /* Change the version number to the new one */
      clnt_control (kcp->client, CLSET_VERS, (void *)&vers);
      return kcp->client;
    }

  if ((kcp->client == (CLIENT *) NULL))
    /* Use the AF_UNIX transport */
    kcp->client = INTUSE(clnt_create) ("/var/run/keyservsock", KEY_PROG, vers,
				       "unix");

  if (kcp->client == (CLIENT *) NULL)
    return (CLIENT *) NULL;

  kcp->uid = __geteuid ();
  kcp->pid = __getpid ();
  kcp->client->cl_auth = INTUSE(authunix_create) ((char *)"", kcp->uid, 0, 0,
						  NULL);
  if (kcp->client->cl_auth == NULL)
    {
      clnt_destroy (kcp->client);
      kcp->client = NULL;
      return (CLIENT *) NULL;
    }

  wait_time.tv_sec = TOTAL_TIMEOUT/TOTAL_TRIES;
  wait_time.tv_usec = 0;
  clnt_control (kcp->client, CLSET_RETRY_TIMEOUT,
		(char *)&wait_time);
  if (clnt_control (kcp->client, CLGET_FD, (char *)&fd))
    __fcntl (fd, F_SETFD, 1);  /* make it "close on exec" */

  return kcp->client;
}

/* returns  0 on failure, 1 on success */
static int
internal_function
key_call_socket (u_long proc, xdrproc_t xdr_arg, char *arg,
               xdrproc_t xdr_rslt, char *rslt)
{
  CLIENT *clnt;
  struct timeval wait_time;
  int result = 0;

  __libc_lock_lock (keycall_lock);
  if ((proc == KEY_ENCRYPT_PK) || (proc == KEY_DECRYPT_PK) ||
      (proc == KEY_NET_GET) || (proc == KEY_NET_PUT) ||
      (proc == KEY_GET_CONV))
    clnt = getkeyserv_handle(2); /* talk to version 2 */
  else
    clnt = getkeyserv_handle(1); /* talk to version 1 */

  if (clnt != NULL)
    {
      wait_time.tv_sec = TOTAL_TIMEOUT;
      wait_time.tv_usec = 0;

      if (clnt_call (clnt, proc, xdr_arg, arg, xdr_rslt, rslt,
		     wait_time) == RPC_SUCCESS)
	result = 1;
    }

  __libc_lock_unlock (keycall_lock);

  return result;
}

#ifdef HAVE_DOORS
/* returns 0 on failure, 1 on success */
static int
internal_function
key_call_door (u_long proc, xdrproc_t xdr_arg, char *arg,
	       xdrproc_t xdr_rslt, char *rslt)
{
  XDR xdrs;
  int fd, ret;
  door_arg_t args;
  char *data_ptr;
  u_long data_len = 0;
  char res[255];

  if ((fd = open("/var/run/keyservdoor", O_RDONLY)) < 0)
    return 0;
  res[0] = 0;

  data_len = xdr_sizeof (xdr_arg, arg);
  data_ptr = calloc (1, data_len + 2 * sizeof (u_long));
  if (data_ptr == NULL)
    return 0;

  INTUSE(xdrmem_create) (&xdrs, &data_ptr[2 * sizeof (u_long)], data_len,
			 XDR_ENCODE);
  if (!xdr_arg (&xdrs, arg))
    {
      xdr_destroy (&xdrs);
      free (data_ptr);
      return 0;
    }
  xdr_destroy (&xdrs);

  memcpy (data_ptr, &proc, sizeof (u_long));
  memcpy (&data_ptr[sizeof (proc)], &data_len, sizeof (u_long));

  args.data_ptr = data_ptr;
  args.data_size = data_len + 2 * sizeof (u_long);
  args.desc_ptr = NULL;
  args.desc_num = 0;
  args.rbuf = res;
  args.rsize = sizeof (res);

  ret = __door_call (fd, &args);
  free (data_ptr);
  close (fd);

  if (ret < 0)
    return 0;

  memcpy (&data_len, args.data_ptr, sizeof (u_long));
  if (data_len != 0)
    return 0;

  memcpy (&data_len, &args.data_ptr[sizeof (u_long)], sizeof (u_long));
  INTUSE(xdrmem_create) (&xdrs, &args.data_ptr[2 * sizeof (u_long)],
			 data_len, XDR_DECODE);
  if (!xdr_rslt (&xdrs, rslt))
    {
      xdr_destroy (&xdrs);
      return 0;
    }
  xdr_destroy (&xdrs);

  return 1;
}
#endif

/* returns 0 on failure, 1 on success */
static int
internal_function
key_call (u_long proc, xdrproc_t xdr_arg, char *arg,
	  xdrproc_t xdr_rslt, char *rslt)
{
#ifndef SO_PASSCRED
  static int use_keyenvoy;
#endif
#ifdef HAVE_DOORS
  static int not_use_doors;
#endif

  if (proc == KEY_ENCRYPT_PK && __key_encryptsession_pk_LOCAL)
    {
      cryptkeyres *res;
      res = (*__key_encryptsession_pk_LOCAL) (__geteuid (), arg);
      *(cryptkeyres *) rslt = *res;
      return 1;
    }
  else if (proc == KEY_DECRYPT_PK && __key_decryptsession_pk_LOCAL)
    {
      cryptkeyres *res;
      res = (*__key_decryptsession_pk_LOCAL) (__geteuid (), arg);
      *(cryptkeyres *) rslt = *res;
      return 1;
    }
  else if (proc == KEY_GEN && __key_gendes_LOCAL)
    {
      des_block *res;
      res = (*__key_gendes_LOCAL) (__geteuid (), 0);
      *(des_block *) rslt = *res;
      return 1;
    }

#ifdef HAVE_DOORS
  if (!not_use_doors)
    {
      if (key_call_door (proc, xdr_arg, arg, xdr_rslt, rslt))
	return 1;
      not_use_doors = 1;
    }
#endif

#ifdef SO_PASSCRED
  return key_call_socket (proc, xdr_arg, arg, xdr_rslt, rslt);
#else
  if (!use_keyenvoy)
    {
      if (key_call_socket (proc, xdr_arg, arg, xdr_rslt, rslt))
	return 1;
      use_keyenvoy = 1;
    }
  return key_call_keyenvoy (proc, xdr_arg, arg, xdr_rslt, rslt);
#endif
}

#ifdef _RPC_THREAD_SAFE_
void
__rpc_thread_key_cleanup (void)
{
	struct key_call_private *kcp = RPC_THREAD_VARIABLE(key_call_private_s);

	if (kcp) {
		if (kcp->client)
			clnt_destroy(kcp->client);
		free (kcp);
	}
}
#endif /* _RPC_THREAD_SAFE_ */
