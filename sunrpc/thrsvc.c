#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <arpa/inet.h>

#define PROGNUM 1234
#define VERSNUM 1
#define PROCNUM 1

static int exitcode;

static void
dispatch(struct svc_req *request, SVCXPRT *xprt)
{
    svc_sendreply(xprt, (xdrproc_t)xdr_void, 0);
}

static void
test_one_call (CLIENT *c)
{
  struct timeval tout = { 60, 0 };
  enum clnt_stat result;

  printf ("test_one_call: ");
  result = clnt_call (c, PROCNUM,
		      (xdrproc_t) xdr_void, 0,
		      (xdrproc_t) xdr_void, 0, tout);
  if (result == RPC_SUCCESS)
    puts ("success");
  else
    {
      clnt_perrno (result);
      putchar ('\n');
      exitcode = 1;
    }
}

static void *
thread_wrapper (void *arg)
{
  test_one_call ((CLIENT *)arg);
  return 0;
}

int
main (void)
{
  pthread_t tid;
  pid_t pid;
  int err;
  SVCXPRT *svx;
  CLIENT *clnt;
  struct sockaddr_in sin;
  struct timeval wait = { 5, 0 };
  int sock = RPC_ANYSOCK;

  svx = svcudp_create (RPC_ANYSOCK);
  svc_register (svx, PROGNUM, VERSNUM, dispatch, 0);

  pid = fork ();
  if (pid == -1)
    {
      perror ("fork");
      return 1;
    }
  if (pid == 0)
    svc_run ();

  inet_aton ("127.0.0.1", &sin.sin_addr);
  sin.sin_port = htons (svx->xp_port);
  sin.sin_family = AF_INET;

  clnt = clntudp_create (&sin, PROGNUM, VERSNUM, wait, &sock);

  /* Test in this thread */
  test_one_call (clnt);

  /* Test in a child thread */
  err = pthread_create (&tid, 0, thread_wrapper, (void *) clnt);
  if (err)
    fprintf (stderr, "pthread_create: %s\n", strerror (err));
  err = pthread_join (tid, 0);
  if (err)
    fprintf (stderr, "pthread_join: %s\n", strerror (err));

  return exitcode;
}
