/* search.h -- declarations for `insque' and `remque'
Copyright (C) 1995 Free Software Foundation, Inc.
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

/* These functions are provided for compatibility with BSD.  */

#ifndef _SEARCH_H
#define	_SEARCH_H 1

#include <sys/cdefs.h>

__BEGIN_DECLS

/* Prototype structure for a linked-list data structure.
   This is the type used by the `insque' and `remque' functions.  */

struct qelem
  {
    struct qelem *q_forw;
    struct qelem *q_back;
    char q_data[1];
  };


/* Insert ELEM into a doubly-linked list, after PREV.  */
extern void insque __P ((struct qelem *__elem, struct qelem *__prev));

/* Unlink ELEM from the doubly-linked list that it is in.  */
extern void remque __P ((struct qelem *elem));


__END_DECLS

#endif	/* search.h */
