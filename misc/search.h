/* search.h -- declarations for System V style searching functions.
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
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef _SEARCH_H
#define	_SEARCH_H 1

#include <sys/cdefs.h>

#define __need_size_t
#define __need_NULL
#include <stddef.h>

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
extern void remque __P ((struct qelem *__elem));


/* For use with hsearch(3).  */
#ifndef __COMPAR_FN_T
#define __COMPAR_FN_T
typedef int (*__compar_fn_t) __P ((__const __ptr_t, __const __ptr_t));
#endif

/* Action which shall be performed in the call the hsearch.  */
typedef enum
  {
    FIND,
    ENTER
  }
ACTION;

typedef struct entry
  {
    char *key;
    char *data;
  }
ENTRY;

/* Opaque type for internal use.  */
struct _ENTRY;

/* Data type for reentrent functions.  */
struct hsearch_data
  {
    struct _ENTRY *table;
    unsigned int size;
    unsigned int filled;
  };

/* Family of hash table handling functions.  The functions also have
   reentrent counterparts ending with _r.  */
extern ENTRY *hsearch __P ((ENTRY __item, ACTION __action));
extern int hcreate __P ((unsigned int __nel));
extern void hdestroy __P ((void));

extern int hsearch_r __P ((ENTRY __item, ACTION __action, ENTRY **__retval,
			   struct hsearch_data *__htab));
extern int hcreate_r __P ((unsigned int __nel, struct hsearch_data *htab));
extern void hdestroy_r __P ((struct hsearch_data *htab));


/* The tsearch routines are very interesting. They make many
   assumptions about the compiler.  It assumpts that the first field
   in node must be the "key" field, which points to the datum.
   Everything depends on that.  */
/* For tsearch */
typedef enum
{
  preorder,
  postorder,
  endorder,
  leaf
}
VISIT;

extern void *tsearch __P ((__const void * __key, void **__rootp,
			   __compar_fn_t compar));

extern void *tfind __P ((__const void * __key, __const void ** __rootp,
			 __compar_fn_t compar));

extern void *tdelete __P ((__const void * __key, void ** __rootp,
			   __compar_fn_t compar));

#ifndef __ACTION_FN_T
#define __ACTION_FN_T
typedef void (*__action_fn_t) __P ((__const void *__nodep,
				    __const VISIT __value,
				    __const int __level));
#endif

extern void twalk __P ((__const void * __root, __action_fn_t action));


extern void * lfind __P ((__const void * __key, __const void * __base,
			  size_t * __nmemb, size_t __size,
			  __compar_fn_t __compar));

extern void * lsearch __P ((__const void * __key, __const void * __base,
			    size_t * __nmemb, size_t __size,
			    __compar_fn_t __compar));

__END_DECLS

#endif /* search.h */
