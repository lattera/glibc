/* Common code for message queue passing tests.
   Copyright (C) 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2004.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <mqueue.h>
#include <search.h>
#include <stdlib.h>
#include <string.h>

/* List of temporary files.  */
struct temp_mq_list
{
  struct qelem q;
  char name[1];
} *temp_mq_list;

/* Add temporary files in list.  */
static void
__attribute__ ((unused))
add_temp_mq (const char *name)
{
  size_t len = strlen (name);
  struct temp_mq_list *newp
    = (struct temp_mq_list *) calloc (sizeof (*newp) + len, 1);
  if (newp != NULL)
    {
      memcpy (newp->name, name, len + 1);
      if (temp_mq_list == NULL)
        temp_mq_list = (struct temp_mq_list *) &newp->q;
      else
        insque (newp, temp_mq_list);
    }
}

/* Delete all temporary files.  */
static void
delete_temp_mqs (void)
{
  while (temp_mq_list != NULL)
    {
      mq_unlink (temp_mq_list->name);
      temp_mq_list = (struct temp_mq_list *) temp_mq_list->q.q_forw;
    }
}

#define CLEANUP_HANDLER	delete_temp_mqs ()
