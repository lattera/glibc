#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>

/*
 * open a socket, get the process group information of the socket, and use the
 * socket to get the network interface configuration list
 */
main(int argc, char *argv[])
{
  int sock;
  int ioctl_result;

  /* get a socket */
  sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (sock < 0)
    {
      perror("Cannot create socket");
      exit(1);
    }

  /* use ioctl() to get the process group information */
  {
    int get_process_group;

    ioctl_result = ioctl(sock, SIOCGPGRP, (char *) &get_process_group);

    if (ioctl_result < 0)
      {
        int my_errno = errno;

        fprintf(stderr, "errno %d ", my_errno);
        perror("ioctl(get process group)");
      }
  }

  /* use ioctl() to get the interface configuration list */
  {
    static struct ifconf ifc;	/* init to 0 */

    ioctl_result = ioctl(sock, SIOCGIFCONF, (char *) &ifc);

    if (ioctl_result < 0)
      {
        int my_errno = errno;

        fprintf(stderr, "errno %d ", my_errno);
        perror("ioctl(get interface configuration list)");
      }
  }
}
