#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>


int
main (void)
{
#define N 10
  struct gaicb reqmem[N];
  struct gaicb *req[N];
  int n;

  for (n = 0; n < N; ++n)
    {
      asprintf (&reqmem[n].ar_name, "test%d.test.redhat.com", 140 + n);
      reqmem[n].ar_service = NULL;
      reqmem[n].ar_request = NULL;
      reqmem[n].ar_result = NULL;
      req[n] = &reqmem[n];
    }

  if (getaddrinfo_a (GAI_NOWAIT, req, N, NULL) != 0)
    {
      puts ("queue call failed");
      exit (1);
    }
  else
    puts ("queue call successful");

  while (1)
    {
      int any = 0;

      for (n = 0; n < N; ++n)
	if (req[n] != NULL && gai_error (req[n]) != EAI_INPROGRESS)
	  {
	    if (gai_error (req[n]) == 0)
	      {
		struct addrinfo *runp = req[n]->ar_result;

		while (runp != NULL)
		  {
		    switch (runp->ai_family)
		      {
		      case PF_INET:
			{
			  struct sockaddr_in *sinp;

			  sinp = (struct sockaddr_in *) runp->ai_addr;
			  printf ("%2d: %s = %s\n", n,
				  req[n]->ar_name, inet_ntoa (sinp->sin_addr));
			}
			break;
		      default:
			printf ("%2d: family %d\n", n, runp->ai_family);
			break;
		      }
		    runp = runp->ai_next;
		  }
	      }
	    else
	      printf ("error for %d: %s\n", n,
		      gai_strerror (gai_error (req[n])));
	    req[n] = NULL;
	    break;
	  }
	else if (req[n] != NULL)
	  any = 1;

      if (n == N)
	{
	  if (any)
	    gai_suspend (req, N, NULL);
	  else
	    break;
	}
    }

  __libc_write(1,"got all\n", 8);

  for (n = 0; n < N; ++n)
    if (gai_error (&reqmem[n]) == 0)
      {
	struct addrinfo *runp = reqmem[n].ar_result;

	while (runp != NULL)
	  {
	    struct addrinfo *oldp = runp;
	    runp = runp->ai_next;
	    freeaddrinfo (oldp);
	  }
      }

  return 0;
}
