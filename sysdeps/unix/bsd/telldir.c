/* Copyright (C) 1994 Free Software Foundation, Inc.
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

#include <ansidecl.h>
#include <errno.h>
#include <stddef.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

/* Internal data structure for telldir and seekdir.  */
struct record
  {
    struct record *next; /* Link in chain.  */
    off_t cookie;		/* Value returned by `telldir'.  */
    off_t pos;
    size_t offset;
  };
#define NBUCKETS 32
static struct record *records[32];
static off_t lastpos;


/* Return the current position of DIRP.  */
off_t
DEFUN(telldir, (dirp), DIR *dirp)
{
  struct record *new;

  new = malloc (sizeof *new);
  if (new == NULL)
    return (off_t) -1;

  new->pos = dirp->__pos;
  new->offset = dirp->__offset;
  new->cookie = ++lastpos;
  new->next = records[new->cookie % NBUCKETS];
  records[new->cookie % NBUCKETS] = new;

  return new->cookie;
}



/* Seek to position POS in DIRP.  */
void
DEFUN(seekdir, (dirp, pos), DIR *dirp AND __off_t pos)
{
  struct record *r, **prevr;

  for (prevr = &records[pos % NBUCKETS], r = *prevr;
       r != NULL;
       prevr = &r->next, r = r->next)
    if (r->cookie == pos)
      {
	if (dirp->__pos != r->pos || dirp->__offset != r->offset)
	  {
	    dirp->__size = 0;	/* Must read a fresh buffer.  */
	    /* Move to the saved position.  */
	    __lseek (dirp->__fd, r->pos, SEEK_SET);
	    dirp->__pos = r->pos;
	    dirp->__offset = 0;
	    /* Read entries until we reach the saved offset.  */
	    while (dirp->__offset < r->offset)
	      if (readdir (dirp) == NULL)
		break;
	  }

	/* To prevent leaking memory, cookies returned from telldir
	   can only be used once.  So free this one's record now.  */
	*prevr = r->next;
	free (r);
	return;
      }

  /* We lost, but have no way to indicate it.  Oh well.  */
}
