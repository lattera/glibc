/* Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <list.h>
#include <lowlevellock.h>

/* The fork generation counter, defined in libpthread.  */
extern unsigned long int __fork_generation attribute_hidden;

/* Pointer to the fork generation counter in the thread library.  */
extern unsigned long int *__fork_generation_pointer attribute_hidden;

/* Lock to protect handling of fork handlers.  */
extern lll_lock_t __fork_lock attribute_hidden;

/* Lists of registered fork handlers.  */
extern list_t __fork_prepare_list attribute_hidden;
extern list_t __fork_parent_list attribute_hidden;
extern list_t __fork_child_list attribute_hidden;


/* Elements of the fork handler lists.  */
struct fork_handler
{
  list_t list;
  void (*handler) (void);
  void *dso_handle;
};


/* Function to call to unregister fork handlers.  */
extern void __unregister_atfork (void *dso_handle) attribute_hidden;
#define UNREGISTER_ATFORK(dso_handle) __unregister_atfork (dso_handle)


/* C library side function to register new fork handlers.  */
extern int __register_atfork (void (*__prepare) (void),
			      void (*__parent) (void),
			      void (*__child) (void),
			      void *dso_handle);

/* Register the generation counter in the libpthread with the libc.  */
extern void __libc_pthread_int (unsigned long int *__ptr,
				void (*reclaim) (void));
