#ifndef _MALLOC_H
#include <malloc/malloc.h>


/* In the GNU libc we rename the global variable
   `__malloc_initialized' to `__libc_malloc_initialized'.  */
#define __malloc_initialized __libc_malloc_initialized
/* Nonzero if the malloc is already initialized.  */
extern int __malloc_initialized attribute_hidden;

/* Internal routines, operating on "arenas".  */
struct malloc_state;
typedef struct malloc_state *mstate;

extern mstate         _int_new_arena (size_t __ini_size) attribute_hidden;
extern __malloc_ptr_t _int_malloc (mstate __m, size_t __size) attribute_hidden;
extern void           _int_free (mstate __m, __malloc_ptr_t __ptr)
     attribute_hidden;
extern __malloc_ptr_t _int_realloc (mstate __m,
				    __malloc_ptr_t __ptr,
				    size_t __size) attribute_hidden;
extern __malloc_ptr_t _int_memalign (mstate __m, size_t __alignment,
				     size_t __size)
     attribute_hidden;
extern __malloc_ptr_t _int_valloc (mstate __m, size_t __size)
     attribute_hidden;

#endif
