#include <stdlib.h>
#include <string.h>

#define bcopy(s,d,n)	memcpy ((d), (s), (n)) /* No overlap handling.  */

#include "hurdmalloc.h"		/* XXX see that file */

#include <mach.h>
#define vm_allocate __vm_allocate
#define vm_page_size __vm_page_size

/* 
 * Mach Operating System
 * Copyright (c) 1991,1990,1989 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 * HISTORY
 * $Log$
 * Revision 1.10  1995/02/13 22:04:34  roland
 * (malloc_init): Add self reference to avoid not only the `defined but not
 * used' warning, but also to avoid GCC optimizing out the entire function
 * (!).
 *
 * Revision 1.9  1995/02/13  16:36:08  roland
 * Include string.h; #define bcopy using memcpy.
 *
 * Revision 1.8  1995/02/03  01:54:21  roland
 * Remove bogus bcopy decl.
 *
 * Revision 1.7  1995/01/26  04:22:02  roland
 * Don't include gnu-stabs.h.
 *
 * Revision 1.6  1994/12/07  19:41:26  roland
 * (vm_allocate, vm_page_size): #define these to __ names at top.
 *
 * Revision 1.5  1994/06/04  01:48:44  roland
 * entered into RCS
 *
 * Revision 2.7  91/05/14  17:57:34  mrt
 * 	Correcting copyright
 * 
 * Revision 2.6  91/02/14  14:20:26  mrt
 * 	Added new Mach copyright
 * 	[91/02/13  12:41:21  mrt]
 * 
 * Revision 2.5  90/11/05  14:37:33  rpd
 * 	Added malloc_fork* code.
 * 	[90/11/02            rwd]
 * 
 * 	Add spin_lock_t.
 * 	[90/10/31            rwd]
 * 
 * Revision 2.4  90/08/07  14:31:28  rpd
 * 	Removed RCS keyword nonsense.
 * 
 * Revision 2.3  90/06/02  15:14:00  rpd
 * 	Converted to new IPC.
 * 	[90/03/20  20:56:57  rpd]
 * 
 * Revision 2.2  89/12/08  19:53:59  rwd
 * 	Removed conditionals.
 * 	[89/10/23            rwd]
 * 
 * Revision 2.1  89/08/03  17:09:46  rwd
 * Created.
 * 
 *
 * 13-Sep-88  Eric Cooper (ecc) at Carnegie Mellon University
 *	Changed realloc() to copy min(old size, new size) bytes.
 *	Bug found by Mike Kupfer at Olivetti.
 */
/*
 * 	File: 	malloc.c
 *	Author: Eric Cooper, Carnegie Mellon University
 *	Date:	July, 1988
 *
 * 	Memory allocator for use with multiple threads.
 */


#include <cthreads.h>
#include "cthread_internals.h"


/*
 * Structure of memory block header.
 * When free, next points to next block on free list.
 * When allocated, fl points to free list.
 * Size of header is 4 bytes, so minimum usable block size is 8 bytes.
 */
typedef union header {
	union header *next;
	struct free_list *fl;
} *header_t;

#define MIN_SIZE	8	/* minimum block size */

typedef struct free_list {
	spin_lock_t lock;	/* spin lock for mutual exclusion */
	header_t head;		/* head of free list for this size */
#ifdef	DEBUG
	int in_use;		/* # mallocs - # frees */
#endif	DEBUG
} *free_list_t;

/*
 * Free list with index i contains blocks of size 2^(i+3) including header.
 * Smallest block size is 8, with 4 bytes available to user.
 * Size argument to malloc is a signed integer for sanity checking,
 * so largest block size is 2^31.
 */
#define NBUCKETS	29

static struct free_list malloc_free_list[NBUCKETS];

/* Initialization just sets everything to zero, but might be necessary on a
   machine where spin_lock_init does otherwise, and is necessary when
   running an executable that was written by something like Emacs's unexec.
   It preserves the values of data variables like malloc_free_list, but
   does not save the vm_allocate'd space allocated by this malloc.  */

static void
malloc_init (void)
{
  int i;
  for (i = 0; i < NBUCKETS; ++i)
    {
      spin_lock_init (&malloc_free_list[i].lock);
      malloc_free_list[i].head = NULL;
#ifdef DEBUG
      malloc_free_list[i].in_use = 0;
#endif
    }

  /* This not only suppresses a `defined but not used' warning,
     but it is ABSOLUTELY NECESSARY to avoid the hyperclever
     compiler from "optimizing out" the entire function!  */
  (void) &malloc_init;
}

static void
more_memory(size, fl)
	int size;
	register free_list_t fl;
{
	register int amount;
	register int n;
	vm_address_t where;
	register header_t h;
	kern_return_t r;

	if (size <= vm_page_size) {
		amount = vm_page_size;
		n = vm_page_size / size;
		/*
		 * We lose vm_page_size - n*size bytes here.  */
	} else {
		amount = size;
		n = 1;
	}
	MACH_CALL(vm_allocate(mach_task_self(), &where, (vm_size_t) amount, TRUE), r);
	h = (header_t) where;
	do {
	  h->next = fl->head;
	  fl->head = h;
	  h = (header_t) ((char *) h + size);
	} while (--n != 0);
}

/* Declaration changed to standard one for GNU.  */
void *
malloc(size)
	register size_t size;
{
	register int i, n;
	register free_list_t fl;
	register header_t h;

	if ((int) size < 0)		/* sanity check */
		return 0;
	size += sizeof(union header);
	/*
	 * Find smallest power-of-two block size
	 * big enough to hold requested size plus header.
	 */
	i = 0;
	n = MIN_SIZE;
	while (n < size) {
		i += 1;
		n <<= 1;
	}
	ASSERT(i < NBUCKETS);
	fl = &malloc_free_list[i];
	spin_lock(&fl->lock);
	h = fl->head;
	if (h == 0) {
		/*
		 * Free list is empty;
		 * allocate more blocks.
		 */
		more_memory(n, fl);
		h = fl->head;
		if (h == 0) {
			/*
			 * Allocation failed.
			 */
			spin_unlock(&fl->lock);
			return 0;
		}
	}
	/*
	 * Pop block from free list.
	 */
	fl->head = h->next;
#ifdef	DEBUG
	fl->in_use += 1;
#endif	DEBUG
	spin_unlock(&fl->lock);
	/*
	 * Store free list pointer in block header
	 * so we can figure out where it goes
	 * at free() time.
	 */
	h->fl = fl;
	/*
	 * Return pointer past the block header.
	 */
	return ((char *) h) + sizeof(union header);
}

/* Declaration changed to standard one for GNU.  */
void
free(base)
	void *base;
{
	register header_t h;
	register free_list_t fl;
	register int i;

	if (base == 0)
		return;
	/*
	 * Find free list for block.
	 */
	h = (header_t) (base - sizeof(union header));
	fl = h->fl;
	i = fl - malloc_free_list;
	/*
	 * Sanity checks.
	 */
	if (i < 0 || i >= NBUCKETS) {
		ASSERT(0 <= i && i < NBUCKETS);
		return;
	}
	if (fl != &malloc_free_list[i]) {
		ASSERT(fl == &malloc_free_list[i]);
		return;
	}
	/*
	 * Push block on free list.
	 */
	spin_lock(&fl->lock);
	h->next = fl->head;
	fl->head = h;
#ifdef	DEBUG
	fl->in_use -= 1;
#endif	DEBUG
	spin_unlock(&fl->lock);
	return;
}

/* Declaration changed to standard one for GNU.  */
void *
realloc(old_base, new_size)
        void *old_base;
        size_t new_size;
{
	register header_t h;
	register free_list_t fl;
	register int i;
	unsigned int old_size;
	char *new_base;

	if (old_base == 0)
	  return malloc (new_size);

	/*
	 * Find size of old block.
	 */
	h = (header_t) (old_base - sizeof(union header));
	fl = h->fl;
	i = fl - malloc_free_list;
	/*
	 * Sanity checks.
	 */
	if (i < 0 || i >= NBUCKETS) {
		ASSERT(0 <= i && i < NBUCKETS);
		return 0;
	}
	if (fl != &malloc_free_list[i]) {
		ASSERT(fl == &malloc_free_list[i]);
		return 0;
	}
	/*
	 * Free list with index i contains blocks of size 2^(i+3) including header.
	 */
	old_size = (1 << (i+3)) - sizeof(union header);
	/*
	 * Allocate new block, copy old bytes, and free old block.
	 */
	new_base = malloc(new_size);
	if (new_base != 0)
		bcopy(old_base, new_base, (int) (old_size < new_size ? old_size : new_size));
	free(old_base);
	return new_base;
}

#ifdef	DEBUG
void
print_malloc_free_list()
{
  	register int i, size;
	register free_list_t fl;
	register int n;
  	register header_t h;
	int total_used = 0;
	int total_free = 0;

	fprintf(stderr, "      Size     In Use       Free      Total\n");
  	for (i = 0, size = MIN_SIZE, fl = malloc_free_list;
	     i < NBUCKETS;
	     i += 1, size <<= 1, fl += 1) {
		spin_lock(&fl->lock);
		if (fl->in_use != 0 || fl->head != 0) {
			total_used += fl->in_use * size;
			for (n = 0, h = fl->head; h != 0; h = h->next, n += 1)
				;
			total_free += n * size;
			fprintf(stderr, "%10d %10d %10d %10d\n",
				size, fl->in_use, n, fl->in_use + n);
		}
		spin_unlock(&fl->lock);
  	}
  	fprintf(stderr, " all sizes %10d %10d %10d\n",
		total_used, total_free, total_used + total_free);
}
#endif	DEBUG

static void malloc_fork_prepare()
/*
 * Prepare the malloc module for a fork by insuring that no thread is in a
 * malloc critical section.
 */
{
    register int i;
    
    for (i = 0; i < NBUCKETS; i++) {
	spin_lock(&malloc_free_list[i].lock);
    }
}

static void malloc_fork_parent()
/*
 * Called in the parent process after a fork() to resume normal operation.
 */
{
    register int i;

    for (i = NBUCKETS-1; i >= 0; i--) {
	spin_unlock(&malloc_free_list[i].lock);
    }
}

static void malloc_fork_child()
/*
 * Called in the child process after a fork() to resume normal operation.
 */
{
    register int i;

    for (i = NBUCKETS-1; i >= 0; i--) {
	spin_unlock(&malloc_free_list[i].lock);
    }
}


text_set_element (_hurd_fork_prepare_hook, malloc_fork_prepare);
text_set_element (_hurd_fork_parent_hook, malloc_fork_parent);
text_set_element (_hurd_fork_child_hook, malloc_fork_child);
text_set_element (_hurd_preinit_hook, malloc_init);
