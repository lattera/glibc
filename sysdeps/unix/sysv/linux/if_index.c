/* Copyright (C) 1997 Free Software Foundation, Inc.
This file is part of the GNU C Library.

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
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>

#define IF_INET6_FILENAME   "/proc/net/if_inet6"

/* /proc/net/if_inet6 contains lines that look like this:
 *
 * fe8000000000000000000000836fc168 0b 00 20 80     sit7 
 *                        
 *                               |   |  |  |  |      |
 *                     address --'   |  |  |  |      |
 *                     index --------'  |  |  |      |
 *                     prefix length ---'  |  |      |
 *                     scope --------------'  |      |
 *                     flags -----------------'      |
 *                     name -------------------------'
 *
 */

static int get_one_interface(FILE *fd, char *interface, int iflen, unsigned int *index)
{
  char buffer[80];
  static char seps[] = " \012";
  char *c = buffer;
  char *sp;
  if (!fgets(buffer, 80, fd))
    return 1;
  if (strtok_r(buffer, seps, &sp) == NULL) return 1;
  if (c = strtok_r(NULL, seps, &sp), c == NULL) return 1;
  *index = strtoul(c, NULL, 16);
  if (strtok_r(NULL, seps, &sp) == NULL) return 1;
  if (strtok_r(NULL, seps, &sp) == NULL) return 1;
  if (strtok_r(NULL, seps, &sp) == NULL) return 1;
  if (c = strtok_r(NULL, seps, &sp), c == NULL) return 1;
  strncpy(interface, c, iflen);
  return 0;
}

unsigned int if_nametoindex(const char *ifname)
{
  FILE *fd = fopen(IF_INET6_FILENAME, "r");
  char this_ifname[IFNAMSIZ];
  unsigned int this_index;
  if (!fd) return 0;
  while (get_one_interface(fd, this_ifname, IFNAMSIZ, &this_index) == 0) {
    if (!strcmp(this_ifname, ifname)) {
      fclose(fd);
      return this_index;
    }
  } 
  fclose(fd);
  return 0;
}

char *if_indextoname(unsigned int ifindex, char *ifname)
{
  FILE *fd = fopen(IF_INET6_FILENAME, "r");
  unsigned int this_index;
  if (!fd) return NULL;
  while (get_one_interface(fd, ifname, IFNAMSIZ, &this_index) == 0) {
    if (this_index == ifindex) {
      fclose(fd);
      return ifname;
    }
  } 
  fclose(fd);
  return NULL;
}

void if_freenameindex(struct if_nameindex *ifn)
{
  struct if_nameindex *ptr = ifn;
  while (ptr->if_name || ptr->if_index) 
    {
      if (ptr->if_name)
	free(ptr->if_name);
      ptr++;
    }
  free(ifn);
}

struct if_nameindex *if_nameindex(void)
{
  FILE *fd = fopen(IF_INET6_FILENAME, "r");
  struct if_nameindex *ifn = NULL;
  int nifs = 0;
  if (!fd) return NULL;
  do 
    {
      struct if_nameindex *newifn;
      nifs++;
      newifn = realloc(ifn, nifs*sizeof(struct if_nameindex));
      if (!newifn) 
	{
	  /* We ran out of memory. */
	  if (--nifs) 
	    {
	      free(ifn[nifs-1].if_name);
	      ifn[nifs-1].if_name = 0;
	      ifn[nifs-1].if_index = 0;
	      if_freenameindex(ifn);
	    }
	  return NULL;
	}
      ifn = newifn;
      ifn[nifs-1].if_index = 0;
      ifn[nifs-1].if_name = malloc(IFNAMSIZ);
      if (ifn[nifs-1].if_name == NULL) 
	{
	  if_freenameindex(ifn);
	  return NULL;
	}
    }
  while (get_one_interface(fd, ifn[nifs-1].if_name, IFNAMSIZ, 
			   &ifn[nifs-1].if_index) == 0);
  free(ifn[nifs-1].if_name);
  ifn[nifs-1].if_name = NULL;
  fclose(fd);
  return ifn;
}
