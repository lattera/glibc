/* Private header for thread debug library.  */
#ifndef _THREAD_DBP_H
#define _THREAD_DBP_H	1

#include <string.h>
#include "proc_service.h"
#include "thread_db.h"
#include "../linuxthreads/internals.h"


/* Comment out the following for less verbose output.  */
#define LOG(c) __libc_write (2, c "\n", strlen (c "\n"))


/* Handle for a process.  This type is opaque.  */
struct td_thragent
{
  /* Delivered by the debugger and we have to pass it back in the
     proc callbacks.  */
  struct ps_prochandle *ph;

  /* Some cached information.  */

  /* Address of the `__pthread_handles' array.  */
  struct pthread_handle_struct *handles;

  /* Address of the `pthread_kyes' array.  */
  struct pthread_key_struct *keys;

  /* Maximum number of threads.  */
  int pthread_threads_max;

  /* Maximum number of thread-local data keys.  */
  int pthread_keys_max;

  /* Size of 2nd level array for thread-local data keys.  */
  int pthread_key_2ndlevel_size;

  /* Sizeof struct _pthread_descr_struct.  */
  int sizeof_descr;
};


#endif /* thread_dbP.h */
