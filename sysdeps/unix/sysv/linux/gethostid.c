/* Copyright (C) 1995 Free Software Foundation, Inc.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define HOSTIDFILE "/var/adm/hostid"

#ifdef SET_PROCEDURE
int
sethostid (id)
     long int id;
{
  int fd;
  ssize_t written;

  /* Test for appropriate rights to set host ID.  */
  if (geteuid () || getuid ())
    {
      errno = EPERM;
      return -1;
    }

  /* Open file for writing.  Everybody is allowed to read this file.  */
  fd = __open (HOSTIDFILE, O_CREAT|O_WRONLY, 0644);
  if (fd < 0)
    return -1;

  written = __write (fd, &id, sizeof (id));

  __close (fd);

  return written != sizeof (id) ? -1 : 0;
}

#else
# include <sys/param.h>
# include <resolv/netdb.h>
# include <netinet/in.h>

long int
gethostid ()
{
  char hostname[MAXHOSTNAMELEN + 1];
  struct hostent *hp;
  unsigned long id;
  struct in_addr in;
  int fd;

  /* First try to get the ID from a former invocation of sethostid.  */
  fd = __open (HOSTIDFILE, O_RDONLY);
  if (fd >= 0)
    {
      ssize_t n = __read (fd, &id, sizeof (id));

      __close (fd);

      if (n == sizeof (id))
	return id;
    }

  /* Getting from the file was not succesful.  An intelligent guess for
     a unique number of a host is its IP address.  Return this.  */
  if (gethostname (hostname, MAXHOSTNAMELEN) < 0 || hostname[0] == '\0')
    /* This also fails.  Return and arbitrary value.  */
    return 0;

  /* To get the IP address we need to knoe the host name.  */
  hp = gethostbyname (hostname);
  if (hp == NULL)
    return 0;

  in.s_addr = 0;
  memcpy (&in, hp->h_addr,
	  sizeof (in) < hp->h_length ? sizeof (in) : hp->h_length);

  /* For the return value to be not exactly the IP address we do some
     bit fiddling.  */
  return in.s_addr << 16 | in.s_addr >> 16;
}
#endif
