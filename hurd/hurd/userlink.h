/* Support for chains recording users of a resource; `struct hurd_userlink'.
Copyright (C) 1994 Free Software Foundation, Inc.
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

#ifndef	_HURD_USERLINK_H

#define	_HURD_USERLINK_H	1
#include <features.h>

#define __need_NULL
#include <stddef.h>


/* This structure is simply a doubly-linked list.  Users of a given
   resource are recorded by their presence in a list associated with that
   resource.  A user attaches his own link (in local storage) to a shared
   chain at the time he begins using some resource.  When finished with
   that resource, the user removes his link from the chain.  If his link is
   the last (there are no other users of the resource), and his chain has
   been detached from the shared cell (the resource in the cell has been
   replaced), then the user deallocates the resource that he used.  */

struct hurd_userlink
  {
    struct hurd_userlink *next, **prevp;
  };


#ifndef _EXTERN_INLINE
#define _EXTERN_INLINE extern __inline
#endif


/* Attach LINK to the chain of users at *CHAINP.  */

_EXTERN_INLINE void
_hurd_userlink_link (struct hurd_userlink **chainp,
		     struct hurd_userlink *link)
{
  link->next = *chainp;
  if (link->next)
    link->next->prevp = &link->next;
  link->prevp = chainp;
  *chainp = link;
}


/* Detach LINK from its chain.  If the return value is nonzero, the caller
   should deallocate the resource he started using after attaching LINK to
   the chain it's on.  If the return value is zero, then someone else is
   still using the resource.  */

_EXTERN_INLINE int
_hurd_userlink_unlink (struct hurd_userlink *link)
{
  /* The caller should deallocate the resource he used if his chain has
     been detached from the cell (and thus has a nil `prevp'), and there is
     no next link representing another user reference to the same resource. */
  int dealloc = ! link->next && ! link->prevp;

  /* Remove our link from the chain of current users.  */
  if (link->prevp)
    *link->prevp = link->next;
  if (link->next)
    link->next->prevp = link->prevp;

  return dealloc;
}


/* Clear all users from *CHAINP.  Call this when the resource *CHAINP
   protects is changing.  If the return value is nonzero, no users are on
   the chain and the caller should deallocate the resource.  If the return
   value is zero, someone is still using the resource and they will
   deallocate it when they are finished.  */

_EXTERN_INLINE int
_hurd_userlink_clear (struct hurd_userlink **chainp)
{
  if (*chainp == NULL)
    return 1;

  /* Detach the chain of current users from the cell.  The last user to
     remove his link from that chain will deallocate the old resource.  */
  (*chainp)->prevp = NULL;
  *chainp = NULL;
  return 0;
}

#endif	/* hurd/userlink.h */
