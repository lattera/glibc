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
 * This is the rpc server side idle loop
 * Wait for input, call server program.
 */

#include <errno.h>
#include <unistd.h>
#include <libintl.h>
#include <sys/poll.h>
#include <rpc/rpc.h>

/* This function can be used as a signal handler to terminate the
   server loop.  */
void
svc_exit (void)
{
  free (svc_pollfd);
  svc_pollfd = NULL;
  svc_max_pollfd = 0;
}

void
svc_run (void)
{
  int i;
  struct pollfd *my_pollfd = NULL;
  int last_max_pollfd = 0;

  for (;;)
    {
      int max_pollfd = svc_max_pollfd;
      if (max_pollfd == 0 && svc_pollfd == NULL)
	break;

      if (last_max_pollfd != max_pollfd)
	{
	  struct pollfd *new_pollfd
	    = realloc (my_pollfd, sizeof (struct pollfd) * max_pollfd);

	  if (new_pollfd == NULL)
	    {
	      perror (_("svc_run: - out of memory"));
	      break;
	    }

	  my_pollfd = new_pollfd;
	  last_max_pollfd = max_pollfd;
	}

      for (i = 0; i < max_pollfd; ++i)
	{
	  my_pollfd[i].fd = svc_pollfd[i].fd;
	  my_pollfd[i].events = svc_pollfd[i].events;
	  my_pollfd[i].revents = 0;
	}

      switch (i = __poll (my_pollfd, max_pollfd, -1))
	{
	case -1:
	  if (errno == EINTR)
	    continue;
	  perror (_("svc_run: - poll failed"));
	  break;
	case 0:
	  continue;
	default:
	  INTUSE(svc_getreq_poll) (my_pollfd, i);
	  continue;
	}
      break;
    }

  free (my_pollfd);
}
