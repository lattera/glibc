#ifndef _SEARCH_H
#include <misc/search.h>

/* Now define the internal interfaces.  */
extern void __hdestroy __P ((void));
extern void *__tsearch __PMT ((__const void *__key, void **__rootp,
			       __compar_fn_t compar));
extern void *__tfind __PMT ((__const void *__key, void *__const *__rootp,
			     __compar_fn_t compar));
extern void *__tdelete __PMT ((__const void *__key, void **__rootp,
			       __compar_fn_t compar));
extern void __twalk __PMT ((__const void *__root, __action_fn_t action));
extern void __tdestroy __PMT ((void *__root, __free_fn_t freefct));
#endif
