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

  /* Pointer to the `__pthread_threads_events' variable in the target.  */
  psaddr_t pthread_threads_eventsp;

  /* Pointer to the `__pthread_last_event' variable in the target.  */
  psaddr_t pthread_last_event;

  /* Pointer to the `__pthread_handles_num' variable.  */
  psaddr_t pthread_handles_num;
};


/* Type used internally to keep track of thread agent descriptors.  */
struct agent_list
{
  td_thragent_t *ta;
  struct agent_list *next;
};

/* List of all known descriptors.  */
extern struct agent_list *__td_agent_list;

/* Function used to test for correct thread agent pointer.  */
static inline int
ta_ok (const td_thragent_t *ta)
{
  struct agent_list *runp = __td_agent_list;

  if (ta == NULL)
    return 0;

  while (runp != NULL && runp->ta != ta)
    runp = runp->next;

  return runp != NULL;
}

#endif /* thread_dbP.h */
