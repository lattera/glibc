#ifndef _SEARCH_H
#include <misc/search.h>

libc_hidden_proto (hcreate_r)
libc_hidden_proto (hdestroy_r)
libc_hidden_proto (hsearch_r)
libc_hidden_proto (lfind)

/* Now define the internal interfaces.  */
extern void __hdestroy (void);
extern void *__tsearch (const void *__key, void **__rootp,
			__compar_fn_t compar);
extern void *__tfind (const void *__key, void *const *__rootp,
		      __compar_fn_t compar);
extern void *__tdelete (const void *__key, void **__rootp,
			__compar_fn_t compar);
extern void __twalk (const void *__root, __action_fn_t action);
extern void __tdestroy (void *__root, __free_fn_t freefct);
#endif
