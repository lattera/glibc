/* Copyright (C) 1992 Free Software Foundation, Inc.
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
#include <stddef.h>

struct qelem
  {
    struct qelem *q_forw;
    struct qelem *q_back;
    char q_data[1];
  };

/* Insert ELEM into a doubly-linked list, after PREV.  */

void
DEFUN(insque, (elem, prev) ,
      struct qelem *elem AND struct qelem *prev)
{
  struct qelem *next = prev->q_forw;
  prev->q_forw = elem;
  if (next != NULL)
    next->q_back = elem;
  elem->q_forw = next;
  elem->q_back = prev;
}

/* Unlink ELEM from the doubly-linked list that it is in.  */

void
DEFUN(remque, (elem),
      struct qelem *elem)
{
  struct qelem *next = elem->q_forw;
  struct qelem *prev = elem->q_back;
  if (next != NULL)
    next->q_back = prev;
  if (prev != NULL)
    prev->q_forw = next;
}
