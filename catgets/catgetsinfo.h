/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper, <drepper@gnu.ai.mit.edu>.

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

#include <sys/types.h>
#include <bits/libc-lock.h>


struct catalog_obj
{
  u_int32_t magic;
  u_int32_t plane_size;
  u_int32_t plane_depth;
  /* This is in fact two arrays in one: always a pair of name and
     pointer into the data area.  */
  u_int32_t name_ptr[0];
};


/* This structure will be filled after loading the catalog.  */
typedef struct catalog_info
{
  enum { closed, nonexisting, mmapped, malloced } status;

  const char *cat_name;
  const char *env_var;
  const char *nlspath;

  size_t plane_size;
  size_t plane_depth;
  u_int32_t *name_ptr;
  const char *strings;

  struct catalog_obj *file_ptr;
  size_t file_size;

  __libc_lock_define (,lock);
} *__nl_catd;



/* The magic number to signal we really have a catalog file.  */
#define CATGETS_MAGIC 0x960408de


/* Prototypes for helper functions.  */
void __open_catalog (__nl_catd __catalog);
