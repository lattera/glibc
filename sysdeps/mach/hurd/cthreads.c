#include <bits/libc-lock.h>
#include <errno.h>
#include <stdlib.h>

/* Placeholder for key creation routine from Hurd cthreads library.  */
int
weak_function
cthread_keycreate (key)
     cthread_key_t *key;
{
  __set_errno (ENOSYS);
 *key = -1;
  return -1;
}

/* Placeholder for key retrieval routine from Hurd cthreads library.  */
int
weak_function
cthread_getspecific (key, pval)
     cthread_key_t key;
     void **pval;
{
  *pval = NULL;
  __set_errno (ENOSYS);
  return -1;
}

/* Placeholder for key setting routine from Hurd cthreads library.  */
int
weak_function
cthread_setspecific (key, val)
     cthread_key_t key;
     void *val;
{
  __set_errno (ENOSYS);
  return -1;
}

/* Call cthread_getspecific which gets a pointer to the return value instead
   of just returning it.  */
void *
__libc_getspecific (key)
     cthread_key_t key;
{
  void *val;
  cthread_getspecific (key, &val);
  return val;
}
